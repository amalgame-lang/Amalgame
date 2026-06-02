/*
 * Amalgame Standard Library - Amalgame.Net
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: TcpClient, TcpServer, UdpSocket.
 *
 * The libcurl-backed HTTP client + AmalgameHttpResponse lived here
 * through v0.8.30 but was extracted in v0.8.31 — pure-AM HTTP/1.1
 * (and a future libcurl wrapper) now live in their own external
 * packages so that amc itself has zero dependency on libcurl:
 *
 *   amalgame-net-http   pure-AM HTTP/1.1 client + server
 *   amalgame-net-curl   thin libcurl binding (planned)
 */

#ifndef AMALGAME_NET_H
#define AMALGAME_NET_H

#include "_runtime.h"
#include "Amalgame_Collections.h"

/* Cross-platform socket layer. POSIX hosts use the BSD sockets API
 * directly; Windows pulls in Winsock2 and aliases the few calls that
 * differ (close → closesocket) plus a one-shot WSAStartup. Link with
 * -lws2_32 on Windows; -lc is enough on POSIX. */
#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
typedef SSIZE_T ssize_t;
static void _amnet_init_once(void) {
    static int done = 0;
    if (!done) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        done = 1;
    }
}
static int _amnet_close_socket(int fd) { return closesocket(fd); }
#else
#  include <sys/socket.h>
#  include <sys/select.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
static inline void _amnet_init_once(void) {}
static inline int _amnet_close_socket(int fd) { return close(fd); }
#endif

#include <errno.h>
#include <string.h>


/* ================================================================
   TcpClient
   ================================================================ */

typedef struct {
    int         _fd;
    code_bool   Connected;
    code_string RemoteHost;
    i64         RemotePort;
} AmalgameTcpClient;

static inline AmalgameTcpClient* TcpClient_Connect(
        code_string host, i64 port) {
    _amnet_init_once();
    AmalgameTcpClient* c =
        (AmalgameTcpClient*) GC_MALLOC(sizeof(AmalgameTcpClient));
    c->Connected  = false;
    c->RemoteHost = host;
    c->RemotePort = port;
    c->_fd        = -1;

    struct addrinfo hints = {0};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%lld", (long long)port);

    struct addrinfo* res = NULL;
    if (getaddrinfo(host, portStr, &hints, &res) != 0) return c;

    /* Try every resolved address, not just the first. getaddrinfo often
     * returns IPv6 first (e.g. www.google.com); on a host with broken or
     * partial IPv6, connect() to that address hangs until the TCP
     * timeout. Iterating to the IPv4 fallback (like curl's happy
     * eyeballs, minus the parallelism) avoids the hang. */
    for (struct addrinfo* rp = res; rp != NULL; rp = rp->ai_next) {
        int fd = (int) socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, rp->ai_addr, (int) rp->ai_addrlen) == 0) {
            c->_fd       = fd;
            c->Connected = true;
            break;
        }
        _amnet_close_socket(fd);
    }
    freeaddrinfo(res);
    return c;
}

static inline code_bool TcpClient_Send(AmalgameTcpClient* c,
                                        code_string data) {
    if (!c || !c->Connected || c->_fd < 0) return false;
    return send(c->_fd, data, strlen(data), 0) >= 0;
}

static inline code_string TcpClient_Receive(AmalgameTcpClient* c,
                                              i64 maxBytes) {
    if (!c || !c->Connected || c->_fd < 0) return NULL;
    if (maxBytes <= 0) maxBytes = 4096;
    char* buf = (char*) GC_MALLOC(maxBytes + 1);
    ssize_t n = recv(c->_fd, buf, (size_t)maxBytes, 0);
    if (n <= 0) { c->Connected = false; return NULL; }
    buf[n] = '\0';
    return buf;
}

static inline void TcpClient_Close(AmalgameTcpClient* c) {
    if (!c || c->_fd < 0) return;
    _amnet_close_socket(c->_fd);
    c->_fd = -1;
    c->Connected = false;
}

static inline code_bool TcpClient_IsConnected(AmalgameTcpClient* c) {
    return c && c->Connected;
}

/* ================================================================
   TcpServer
   ================================================================ */

typedef struct {
    int       _fd;
    i64       Port;
    code_bool Listening;
} AmalgameTcpServer;

typedef struct {
    int         _fd;
    code_bool   Connected;
    code_string RemoteIp;
    i64         RemotePort;
} AmalgameTcpConn;

static inline AmalgameTcpServer* TcpServer_Listen(i64 port, i64 backlog) {
    _amnet_init_once();
    AmalgameTcpServer* s =
        (AmalgameTcpServer*) GC_MALLOC(sizeof(AmalgameTcpServer));
    s->Port = port; s->Listening = false; s->_fd = -1;

    int fd = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return s;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t) port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ||
        listen(fd, (int)(backlog > 0 ? backlog : 10)) < 0) {
        _amnet_close_socket(fd); return s;
    }
    s->_fd = fd; s->Listening = true;
    return s;
}

static inline AmalgameTcpConn* TcpServer_Accept(AmalgameTcpServer* s) {
    AmalgameTcpConn* c =
        (AmalgameTcpConn*) GC_MALLOC(sizeof(AmalgameTcpConn));
    c->Connected = false; c->_fd = -1;
    c->RemoteIp = ""; c->RemotePort = 0;

    if (!s || !s->Listening || s->_fd < 0) return c;

    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);
    int cfd = (int) accept(s->_fd, (struct sockaddr*)&addr, &len);
    if (cfd < 0) return c;

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    char* ipcopy = (char*) GC_MALLOC(INET_ADDRSTRLEN);
    memcpy(ipcopy, ip, INET_ADDRSTRLEN);

    c->_fd = cfd; c->Connected = true;
    c->RemoteIp = ipcopy;
    c->RemotePort = (i64) ntohs(addr.sin_port);
    return c;
}

static inline void TcpServer_Close(AmalgameTcpServer* s) {
    if (!s || s->_fd < 0) return;
    _amnet_close_socket(s->_fd); s->_fd = -1; s->Listening = false;
}
static inline code_bool TcpServer_IsListening(AmalgameTcpServer* s) {
    return s && s->Listening;
}
static inline code_bool TcpConn_Send(AmalgameTcpConn* c, code_string data) {
    if (!c || !c->Connected || c->_fd < 0) return false;
    return send(c->_fd, data, strlen(data), 0) >= 0;
}
static inline code_string TcpConn_Receive(AmalgameTcpConn* c, i64 maxBytes) {
    if (!c || !c->Connected || c->_fd < 0) return NULL;
    if (maxBytes <= 0) maxBytes = 4096;
    char* buf = (char*) GC_MALLOC(maxBytes + 1);
    ssize_t n = recv(c->_fd, buf, (size_t)maxBytes, 0);
    if (n <= 0) { c->Connected = false; return NULL; }
    buf[n] = '\0'; return buf;
}
static inline void TcpConn_Close(AmalgameTcpConn* c) {
    if (!c || c->_fd < 0) return;
    _amnet_close_socket(c->_fd); c->_fd = -1; c->Connected = false;
}
static inline code_bool TcpConn_IsConnected(AmalgameTcpConn* c) {
    return c && c->Connected;
}

/* ================================================================
   UdpSocket
   ================================================================ */

typedef struct {
    int   _fd;
    i64   BoundPort;
    code_bool Bound;
} AmalgameUdpSocket;

static inline AmalgameUdpSocket* UdpSocket_New() {
    _amnet_init_once();
    AmalgameUdpSocket* s =
        (AmalgameUdpSocket*) GC_MALLOC(sizeof(AmalgameUdpSocket));
    s->_fd = (int) socket(AF_INET, SOCK_DGRAM, 0);
    s->Bound = false; s->BoundPort = 0;
    /* v0.8.53 — default SO_RCVBUF to 8 MiB. Linux's stock 208 KiB
     * was good enough for client-style UDP but the moment any peer-
     * to-peer protocol (Pollen, future SMTP-over-UDP, etc.) bursts
     * datagrams faster than the receiver thread can drain them, the
     * buffer overflows and the kernel silently drops. 8 MiB holds
     * ~50 k typical datagrams — enough headroom for a single-thread
     * receiver to catch up. Callers can still override with
     * UdpSocket_SetRecvBuf if they need more (or less). */
    if (s->_fd >= 0) {
        int rcvbuf = 8 * 1024 * 1024;
        setsockopt(s->_fd, SOL_SOCKET, SO_RCVBUF,
                   (const char*)&rcvbuf, sizeof(rcvbuf));
        /* Kernel may clamp to net.core.rmem_max — we don't error
         * out on partial accept since the smaller window still
         * works, just with less headroom. Tune sysctl if needed. */
    }
    return s;
}

/* Explicit tuning hook. Returns the size the kernel actually granted
 * (clamped to net.core.rmem_max on Linux), or -1 on error. Caller
 * passes the desired total buffer in bytes. */
static inline i64 UdpSocket_SetRecvBuf(AmalgameUdpSocket* s, i64 bytes) {
    if (!s || s->_fd < 0) return -1;
    int v = (int)(bytes > 0 ? bytes : 0);
    if (setsockopt(s->_fd, SOL_SOCKET, SO_RCVBUF,
                   (const char*)&v, sizeof(v)) < 0) return -1;
    int actual = 0;
    socklen_t alen = sizeof(actual);
    if (getsockopt(s->_fd, SOL_SOCKET, SO_RCVBUF,
                   (char*)&actual, &alen) < 0) return -1;
    /* Linux returns 2× the requested value (one half for data, one
     * for bookkeeping) — we report the kernel-reported value as-is
     * so callers can correlate with sysctl values. */
    return (i64) actual;
}
static inline code_bool UdpSocket_Bind(AmalgameUdpSocket* s, i64 port) {
    if (!s || s->_fd < 0) return false;
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(s->_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        s->Bound = true;
        if (port == 0) {
            /* Caller asked for an ephemeral port — read back the
             * actual port the kernel assigned. Before v0.8.52 we
             * stored 0, which left every ephemeral binder unable
             * to advertise itself. */
            struct sockaddr_in actual = {0};
            socklen_t alen = sizeof(actual);
            if (getsockname(s->_fd, (struct sockaddr*)&actual, &alen) == 0) {
                s->BoundPort = (i64) ntohs(actual.sin_port);
            } else {
                s->BoundPort = 0;
            }
        } else {
            s->BoundPort = port;
        }
        return true;
    }
    return false;
}
static inline code_bool UdpSocket_Send(AmalgameUdpSocket* s,
        code_string host, i64 port, code_string data) {
    if (!s || s->_fd < 0) return false;
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    return sendto(s->_fd, data, strlen(data), 0,
                  (struct sockaddr*)&addr, sizeof(addr)) >= 0;
}
static inline code_string UdpSocket_Receive(AmalgameUdpSocket* s,
        i64 maxBytes) {
    if (!s || s->_fd < 0) return NULL;
    if (maxBytes <= 0) maxBytes = 4096;
    char* buf = (char*) GC_MALLOC(maxBytes + 1);
    ssize_t n = recv(s->_fd, buf, (size_t)maxBytes, 0);
    if (n < 0) return NULL;
    buf[n] = '\0'; return buf;
}
static inline void UdpSocket_Close(AmalgameUdpSocket* s) {
    if (!s || s->_fd < 0) return;
    _amnet_close_socket(s->_fd); s->_fd = -1;
}

/* ────────────────────────────────────────────────────────
 * UdpDatagram (v0.8.52) — single UDP packet with sender info.
 *
 * Returned by UdpSocket.ReceiveFrom. The plain UdpSocket.Receive
 * above uses recv() and so cannot tell callers who sent the
 * packet — fine for client-style UDP but useless for any peer-to-
 * peer protocol (ACK routing, request/response, etc.). ReceiveFrom
 * uses recvfrom() and exposes the source ip:port. It also accepts
 * a millisecond timeout (select-driven) so the receive loop can
 * be bounded — required for any retry/ack pattern.
 * ──────────────────────────────────────────────────────── */

typedef struct {
    code_string Data;     /* NUL-terminated for AM string ergonomics */
    i64         DataLen;  /* real byte count — Data may contain NULs */
    code_string FromIp;   /* "192.168.1.42" */
    i64         FromPort;
} AmalgameUdpDatagram;

static inline AmalgameUdpDatagram* UdpSocket_ReceiveFrom(
        AmalgameUdpSocket* s, i64 maxBytes, i64 timeoutMs) {
    if (!s || s->_fd < 0) return NULL;
    if (maxBytes <= 0) maxBytes = 4096;

    /* Bounded wait via select(). timeoutMs<0 = block forever. */
    if (timeoutMs >= 0) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(s->_fd, &rfds);
        struct timeval tv;
        tv.tv_sec  = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        int rv = select(s->_fd + 1, &rfds, NULL, NULL, &tv);
        if (rv <= 0) return NULL;   /* timeout / interrupt / error → null */
    }

    char* buf = (char*) GC_MALLOC((size_t) maxBytes + 1);
    struct sockaddr_in from = {0};
    socklen_t flen = sizeof(from);
    ssize_t n = recvfrom(s->_fd, buf, (size_t) maxBytes, 0,
                         (struct sockaddr*)&from, &flen);
    if (n < 0) return NULL;
    buf[n] = '\0';

    char ipbuf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &from.sin_addr, ipbuf, sizeof(ipbuf))) {
        ipbuf[0] = '\0';
    }
    char* ipcopy = (char*) GC_MALLOC(strlen(ipbuf) + 1);
    strcpy(ipcopy, ipbuf);

    AmalgameUdpDatagram* dg =
        (AmalgameUdpDatagram*) GC_MALLOC(sizeof(AmalgameUdpDatagram));
    dg->Data     = buf;
    dg->DataLen  = (i64) n;
    dg->FromIp   = ipcopy;
    dg->FromPort = (i64) ntohs(from.sin_port);
    return dg;
}

#endif /* AMALGAME_NET_H */
