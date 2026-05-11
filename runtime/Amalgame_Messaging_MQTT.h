/*
 * Amalgame Standard Library — Amalgame.Messaging.MQTT
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * MQTT 3.1.1 client speaking the wire protocol directly over TCP.
 * No external client lib — the protocol fits in ~450 lines of C and
 * MQTT 3.1.1 is the most widely-deployed version (every broker
 * understands it: Mosquitto, HiveMQ, EMQX, AWS IoT, Azure IoT Hub).
 *
 * Surface (v1):
 *   Open / Close / IsOpen / LastError                — lifecycle + diag
 *   Ping                                              — PINGREQ → PINGRESP
 *   Publish(topic, payload)                           — QoS 0
 *   Subscribe(topic)                                  — QoS 0
 *   WaitMessage(timeout_ms) / LastTopic / LastPayload — recv loop
 *
 * Deferred to v2: QoS 1 (PUBACK), QoS 2 (PUBREC/PUBREL/PUBCOMP),
 * retain, last-will, automatic keepalive timer, multi-topic
 * Subscribe in one packet, MQTT 5 properties, TLS, username/password
 * auth, wildcards exercised in tests (`+` single-level, `#`
 * multi-level — brokers handle them; we just don't test).
 *
 * Reuses the cross-platform socket layer from Amalgame_Net.h.
 *
 * Threading: single-owner handle. Don't call WaitMessage from one
 * thread while Publish runs on another against the same handle —
 * the wire is half-duplex per packet. Concurrent handles are safe.
 */

#ifndef AMALGAME_MESSAGING_MQTT_H
#define AMALGAME_MESSAGING_MQTT_H

#include "_runtime.h"
#include "Amalgame_Collections.h"
#include "Amalgame_Net.h"   /* cross-platform sockets + _amnet_init_once */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#  include <sys/time.h>     /* struct timeval for SO_RCVTIMEO */
#endif

typedef struct AmalgameMQTT {
    int         fd;                /* socket fd; -1 = not connected */
    char*       last_error;        /* GC-strdup'd, or NULL */
    char*       last_topic;        /* most recent PUBLISH topic */
    char*       last_payload;      /* most recent PUBLISH payload */
    i64         next_packet_id;    /* monotonic; 1..65535 */
} AmalgameMQTT;

/* ── Small helpers ──────────────────────────────────── */

static inline code_string _ammqtt_err_dup(const char* msg) {
    if (!msg) return NULL;
    size_t n = strlen(msg);
    char* p = (char*) code_alloc(n + 1);
    memcpy(p, msg, n + 1);
    return p;
}

/* send() can short-write; loop until every byte's on the wire. */
static inline int _ammqtt_send_all(int fd, const unsigned char* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t k = send(fd, (const char*) buf + off, n - off, 0);
        if (k <= 0) return -1;
        off += (size_t) k;
    }
    return 0;
}

/* recv() can short-read; loop until exactly N bytes arrive. */
static inline int _ammqtt_recv_all(int fd, unsigned char* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t k = recv(fd, (char*) buf + off, n - off, 0);
        if (k <= 0) return -1;
        off += (size_t) k;
    }
    return 0;
}

/* MQTT "remaining length" — variable-length encoding, 1..4 bytes.
 * Each byte's MSB indicates continuation; the LSBs are base-128
 * data. Encodes 0..268_435_455. */
static inline size_t _ammqtt_encode_remlen(size_t n, unsigned char* out) {
    size_t i = 0;
    do {
        unsigned char byte = (unsigned char) (n & 0x7F);
        n >>= 7;
        if (n > 0) byte |= 0x80;
        out[i++] = byte;
    } while (n > 0 && i < 4);
    return i;
}

/* Decode remaining length from socket. Returns the length (>= 0)
 * or -1 on socket error / malformed input. */
static inline long long _ammqtt_decode_remlen(int fd) {
    long long mult = 1;
    long long val  = 0;
    int       i    = 0;
    while (i < 4) {
        unsigned char b;
        if (_ammqtt_recv_all(fd, &b, 1) < 0) return -1;
        val += (long long) (b & 0x7F) * mult;
        if ((b & 0x80) == 0) return val;
        mult *= 128;
        i++;
    }
    return -1; /* malformed: 5th byte with continuation bit */
}

/* Set a recv timeout on the socket. Pass 0 to clear (block
 * indefinitely). Helps WaitMessage return false on idle brokers. */
static inline void _ammqtt_set_rcvtimeo(int fd, i64 ms) {
#ifdef _WIN32
    DWORD t = (DWORD) (ms > 0 ? ms : 0);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &t, sizeof(t));
#else
    struct timeval tv;
    if (ms <= 0) { tv.tv_sec = 0; tv.tv_usec = 0; }
    else         { tv.tv_sec = (time_t) (ms / 1000);
                   tv.tv_usec = (suseconds_t) ((ms % 1000) * 1000); }
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

/* ── Lifecycle ──────────────────────────────────────── */

/* Connect to mqtt://host:port and complete the CONNECT/CONNACK
 * handshake. clientId may be empty — the broker assigns one (but
 * with the clean-session bit set, persistent subscriptions are
 * scoped to this connection only). Returns a non-NULL handle even
 * on failure — call MQTT.IsOpen() to check. */
static inline AmalgameMQTT* MQTT_Open(code_string host, i64 port, code_string clientId) {
    _amnet_init_once();
    AmalgameMQTT* m = (AmalgameMQTT*) code_alloc(sizeof(AmalgameMQTT));
    m->fd             = -1;
    m->last_error     = NULL;
    m->last_topic     = NULL;
    m->last_payload   = NULL;
    m->next_packet_id = 1;

    if (!host || !*host) {
        m->last_error = _ammqtt_err_dup("host is empty");
        return m;
    }

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%lld", (long long) port);

    struct addrinfo hints = {0};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    if (getaddrinfo(host, portStr, &hints, &res) != 0) {
        m->last_error = _ammqtt_err_dup("getaddrinfo failed");
        return m;
    }
    int fd = (int) socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        m->last_error = _ammqtt_err_dup("socket() failed");
        freeaddrinfo(res);
        return m;
    }
    if (connect(fd, res->ai_addr, (int) res->ai_addrlen) != 0) {
        m->last_error = _ammqtt_err_dup("connect() failed");
        _amnet_close_socket(fd);
        freeaddrinfo(res);
        return m;
    }
    freeaddrinfo(res);

    /* Build CONNECT packet:
     *   Fixed header:   0x10 + RemLen
     *   Variable header:
     *     Protocol Name "MQTT" (length-prefixed)
     *     Protocol Level 0x04 (MQTT 3.1.1)
     *     Connect Flags  0x02 (clean session)
     *     Keep Alive     0x00 0x3C (60s — no auto-ping in v1, but
     *                              we declare it so brokers won't
     *                              hang up immediately on idle)
     *   Payload:
     *     Client ID (length-prefixed, may be empty) */
    const char*  cid    = clientId ? clientId : "";
    size_t       cidLen = strlen(cid);
    size_t       varhdr_payload_len = 2 + 4 /* "MQTT" */
                                    + 1     /* protocol level */
                                    + 1     /* connect flags */
                                    + 2     /* keep alive */
                                    + 2 + cidLen; /* client id */
    unsigned char remlen_buf[4];
    size_t remlen_n = _ammqtt_encode_remlen(varhdr_payload_len, remlen_buf);

    size_t pkt_total = 1 + remlen_n + varhdr_payload_len;
    unsigned char* pkt = (unsigned char*) code_alloc(pkt_total);
    size_t pos = 0;
    pkt[pos++] = 0x10;
    memcpy(pkt + pos, remlen_buf, remlen_n);
    pos += remlen_n;
    pkt[pos++] = 0x00; pkt[pos++] = 0x04;
    memcpy(pkt + pos, "MQTT", 4); pos += 4;
    pkt[pos++] = 0x04;          /* MQTT 3.1.1 */
    pkt[pos++] = 0x02;          /* Clean session */
    pkt[pos++] = 0x00; pkt[pos++] = 0x3C;  /* Keep alive 60s */
    pkt[pos++] = (unsigned char) ((cidLen >> 8) & 0xFF);
    pkt[pos++] = (unsigned char) (cidLen & 0xFF);
    if (cidLen > 0) memcpy(pkt + pos, cid, cidLen);
    pos += cidLen;

    if (_ammqtt_send_all(fd, pkt, pkt_total) < 0) {
        m->last_error = _ammqtt_err_dup("CONNECT send failed");
        _amnet_close_socket(fd);
        return m;
    }

    /* Read CONNACK: 0x20 0x02 SP RC */
    unsigned char hdr[4];
    if (_ammqtt_recv_all(fd, hdr, 4) < 0) {
        m->last_error = _ammqtt_err_dup("CONNACK recv failed");
        _amnet_close_socket(fd);
        return m;
    }
    if (hdr[0] != 0x20 || hdr[1] != 0x02) {
        m->last_error = _ammqtt_err_dup("unexpected CONNACK header");
        _amnet_close_socket(fd);
        return m;
    }
    if (hdr[3] != 0x00) {
        /* Return codes 1..5 are documented refusals (bad version,
         * id rejected, server unavailable, bad credentials, not
         * authorized). Surface as the LastError code. */
        char buf[64];
        snprintf(buf, sizeof(buf), "CONNECT refused (code %u)", (unsigned) hdr[3]);
        m->last_error = _ammqtt_err_dup(buf);
        _amnet_close_socket(fd);
        return m;
    }
    m->fd = fd;
    return m;
}

/* Send DISCONNECT (0xE0 0x00) then close the socket. Idempotent. */
static inline void MQTT_Close(AmalgameMQTT* m) {
    if (m && m->fd >= 0) {
        unsigned char dc[2] = { 0xE0, 0x00 };
        send(m->fd, (const char*) dc, 2, 0);  /* best-effort */
        _amnet_close_socket(m->fd);
        m->fd = -1;
    }
}

static inline code_bool MQTT_IsOpen(AmalgameMQTT* m) {
    return (m && m->fd >= 0) ? 1 : 0;
}

static inline code_string MQTT_LastError(AmalgameMQTT* m) {
    if (!m) return "";
    return m->last_error ? m->last_error : "";
}

/* ── PINGREQ / PINGRESP ─────────────────────────────── */

static inline code_bool MQTT_Ping(AmalgameMQTT* m) {
    if (!m || m->fd < 0) return 0;
    unsigned char req[2]  = { 0xC0, 0x00 };
    if (_ammqtt_send_all(m->fd, req, 2) < 0) {
        m->last_error = _ammqtt_err_dup("PINGREQ send failed");
        return 0;
    }
    unsigned char resp[2];
    if (_ammqtt_recv_all(m->fd, resp, 2) < 0) {
        m->last_error = _ammqtt_err_dup("PINGRESP recv failed");
        return 0;
    }
    if (resp[0] != 0xD0 || resp[1] != 0x00) {
        m->last_error = _ammqtt_err_dup("unexpected PINGRESP");
        return 0;
    }
    return 1;
}

/* ── Publish ────────────────────────────────────────── */

/* PUBLISH packet, QoS 0:
 *   0x30 + RemLen
 *   Topic Length (2 bytes)
 *   Topic bytes
 *   Payload bytes (rest of packet — no length prefix) */
static inline code_bool MQTT_Publish(AmalgameMQTT* m, code_string topic, code_string payload) {
    if (!m || m->fd < 0) return 0;
    const char* t = topic   ? topic   : "";
    const char* p = payload ? payload : "";
    size_t      tlen = strlen(t);
    size_t      plen = strlen(p);
    size_t      varhdr_payload_len = 2 + tlen + plen;

    unsigned char remlen_buf[4];
    size_t remlen_n = _ammqtt_encode_remlen(varhdr_payload_len, remlen_buf);
    size_t pkt_total = 1 + remlen_n + varhdr_payload_len;
    unsigned char* pkt = (unsigned char*) code_alloc(pkt_total);
    size_t pos = 0;
    pkt[pos++] = 0x30;  /* PUBLISH, DUP=0, QoS=0, RETAIN=0 */
    memcpy(pkt + pos, remlen_buf, remlen_n);
    pos += remlen_n;
    pkt[pos++] = (unsigned char) ((tlen >> 8) & 0xFF);
    pkt[pos++] = (unsigned char) (tlen & 0xFF);
    if (tlen > 0) memcpy(pkt + pos, t, tlen);
    pos += tlen;
    if (plen > 0) memcpy(pkt + pos, p, plen);
    pos += plen;

    if (_ammqtt_send_all(m->fd, pkt, pkt_total) < 0) {
        m->last_error = _ammqtt_err_dup("PUBLISH send failed");
        return 0;
    }
    /* QoS 0: no PUBACK round-trip. Brokers may close the
     * connection on protocol error but won't reply on success. */
    return 1;
}

/* ── Subscribe ──────────────────────────────────────── */

/* SUBSCRIBE packet:
 *   0x82 + RemLen        (0x82 = SUBSCRIBE with reserved bit set)
 *   Packet ID (2 bytes)
 *   Topic Length (2 bytes) + topic bytes
 *   Requested QoS (1 byte) */
static inline code_bool MQTT_Subscribe(AmalgameMQTT* m, code_string topic) {
    if (!m || m->fd < 0) return 0;
    const char* t    = topic ? topic : "";
    size_t      tlen = strlen(t);
    if (tlen == 0) {
        m->last_error = _ammqtt_err_dup("subscribe topic is empty");
        return 0;
    }
    i64 pid = m->next_packet_id;
    m->next_packet_id = (pid >= 65535) ? 1 : pid + 1;

    size_t varhdr_payload_len = 2 + 2 + tlen + 1;
    unsigned char remlen_buf[4];
    size_t remlen_n = _ammqtt_encode_remlen(varhdr_payload_len, remlen_buf);
    size_t pkt_total = 1 + remlen_n + varhdr_payload_len;
    unsigned char* pkt = (unsigned char*) code_alloc(pkt_total);
    size_t pos = 0;
    pkt[pos++] = 0x82;
    memcpy(pkt + pos, remlen_buf, remlen_n);
    pos += remlen_n;
    pkt[pos++] = (unsigned char) ((pid >> 8) & 0xFF);
    pkt[pos++] = (unsigned char) (pid & 0xFF);
    pkt[pos++] = (unsigned char) ((tlen >> 8) & 0xFF);
    pkt[pos++] = (unsigned char) (tlen & 0xFF);
    memcpy(pkt + pos, t, tlen);
    pos += tlen;
    pkt[pos++] = 0x00;  /* QoS 0 */

    if (_ammqtt_send_all(m->fd, pkt, pkt_total) < 0) {
        m->last_error = _ammqtt_err_dup("SUBSCRIBE send failed");
        return 0;
    }

    /* Read SUBACK: 0x90 + RemLen + PacketID(2) + GrantedQoS(1).
     * RemLen is at least 3 for a single-topic SUBSCRIBE. */
    unsigned char ack_hdr;
    if (_ammqtt_recv_all(m->fd, &ack_hdr, 1) < 0) {
        m->last_error = _ammqtt_err_dup("SUBACK recv failed");
        return 0;
    }
    if (ack_hdr != 0x90) {
        m->last_error = _ammqtt_err_dup("unexpected SUBACK header");
        return 0;
    }
    long long rem = _ammqtt_decode_remlen(m->fd);
    if (rem < 3) {
        m->last_error = _ammqtt_err_dup("SUBACK too short");
        return 0;
    }
    unsigned char body[256];
    size_t want = (size_t) (rem < (long long) sizeof(body) ? rem : (long long) sizeof(body));
    if (_ammqtt_recv_all(m->fd, body, want) < 0) {
        m->last_error = _ammqtt_err_dup("SUBACK body recv failed");
        return 0;
    }
    /* body[0..1] = packet id; body[2] = granted QoS (0x80 = failure) */
    if (body[2] == 0x80) {
        m->last_error = _ammqtt_err_dup("SUBACK: subscription failure");
        return 0;
    }
    return 1;
}

/* ── Receive loop ───────────────────────────────────── */

/* Block for up to timeout_ms waiting for a single PUBLISH packet.
 * On success: caches the topic + payload (accessible via
 * MQTT.LastTopic / MQTT.LastPayload) and returns true. On
 * timeout / EOF / error: returns false (LastError set on error).
 *
 * Non-PUBLISH packets (PINGRESP from a concurrent Ping, etc.) are
 * consumed silently so they don't desync the stream; we keep
 * reading until we hit a PUBLISH or hit the timeout window once.
 *
 * timeout_ms <= 0 blocks indefinitely. */
static inline code_bool MQTT_WaitMessage(AmalgameMQTT* m, i64 timeout_ms) {
    if (!m || m->fd < 0) return 0;
    _ammqtt_set_rcvtimeo(m->fd, timeout_ms);

    /* Loop once per non-PUBLISH packet; bail on timeout or error. */
    while (1) {
        unsigned char hdr;
        ssize_t k = recv(m->fd, (char*) &hdr, 1, 0);
        if (k <= 0) {
            _ammqtt_set_rcvtimeo(m->fd, 0);
            return 0;
        }
        long long rem = _ammqtt_decode_remlen(m->fd);
        if (rem < 0) {
            _ammqtt_set_rcvtimeo(m->fd, 0);
            m->last_error = _ammqtt_err_dup("malformed remaining length");
            return 0;
        }
        unsigned char* body = NULL;
        if (rem > 0) {
            body = (unsigned char*) code_alloc((size_t) rem);
            if (_ammqtt_recv_all(m->fd, body, (size_t) rem) < 0) {
                _ammqtt_set_rcvtimeo(m->fd, 0);
                return 0;
            }
        }
        unsigned char type = (unsigned char) (hdr >> 4);
        if (type == 3) {  /* PUBLISH */
            /* Parse variable header:
             *   Topic Length (2)
             *   Topic bytes
             *   [Packet ID (2) — only if QoS > 0; we accept QoS 0 here]
             *   Payload bytes (rest) */
            unsigned char qos = (unsigned char) ((hdr >> 1) & 0x03);
            if (rem < 2) {
                _ammqtt_set_rcvtimeo(m->fd, 0);
                m->last_error = _ammqtt_err_dup("PUBLISH too short");
                return 0;
            }
            size_t tlen = ((size_t) body[0] << 8) | (size_t) body[1];
            if (2 + tlen > (size_t) rem) {
                _ammqtt_set_rcvtimeo(m->fd, 0);
                m->last_error = _ammqtt_err_dup("PUBLISH topic overruns body");
                return 0;
            }
            char* topic = (char*) code_alloc(tlen + 1);
            memcpy(topic, body + 2, tlen);
            topic[tlen] = '\0';
            size_t payload_start = 2 + tlen;
            if (qos > 0) payload_start += 2;  /* skip packet id */
            if (payload_start > (size_t) rem) payload_start = (size_t) rem;
            size_t plen = (size_t) rem - payload_start;
            char*  payload = (char*) code_alloc(plen + 1);
            if (plen > 0) memcpy(payload, body + payload_start, plen);
            payload[plen] = '\0';
            m->last_topic   = topic;
            m->last_payload = payload;
            _ammqtt_set_rcvtimeo(m->fd, 0);
            return 1;
        }
        /* Non-PUBLISH (PINGRESP, SUBACK from a race, etc.) —
         * discard and read the next packet. The timeout is shared
         * across the whole call though, so we'll bail eventually. */
    }
}

static inline code_string MQTT_LastTopic(AmalgameMQTT* m) {
    if (!m || !m->last_topic) return "";
    return m->last_topic;
}

static inline code_string MQTT_LastPayload(AmalgameMQTT* m) {
    if (!m || !m->last_payload) return "";
    return m->last_payload;
}

#endif /* AMALGAME_MESSAGING_MQTT_H */
