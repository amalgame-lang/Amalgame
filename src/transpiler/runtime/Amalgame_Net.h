/*
 * Amalgame Standard Library - Amalgame.Net
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/BastienMOUGET/Amalgame
 *
 * Provides: Http, TcpClient, TcpServer, UdpSocket
 *
 * Http requires libcurl:
 *   Debian/Ubuntu : sudo apt install libcurl4-openssl-dev
 *   macOS         : brew install curl
 *   Fedora/RHEL   : sudo dnf install libcurl-devel
 *   Windows/MSYS2 : pacman -S mingw-w64-x86_64-curl
 */

#ifndef AMALGAME_NET_H
#define AMALGAME_NET_H

#include "_runtime.h"
#include "Amalgame_Collections.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* ================================================================
   HttpResponse
   ================================================================ */

typedef struct {
    i64         Status;
    code_string Body;
    code_string Error;
    code_bool   Ok;
} AmalgameHttpResponse;

static inline AmalgameHttpResponse* _amnet_resp_new(
        long status, code_string body, code_string err) {
    AmalgameHttpResponse* r =
        (AmalgameHttpResponse*) GC_MALLOC(sizeof(AmalgameHttpResponse));
    r->Status = (i64) status;
    r->Body   = body ? body : "";
    r->Error  = err  ? err  : NULL;
    r->Ok     = (status >= 200 && status < 300);
    return r;
}

/* ================================================================
   Http — libcurl implementation (conditional)
   ================================================================ */

#ifdef __has_include
#  if __has_include(<curl/curl.h>)
#    define AMALGAME_HAS_CURL 1
#    include <curl/curl.h>
#  endif
#endif

#ifdef AMALGAME_HAS_CURL

typedef struct {
    char*  data;
    size_t size;
} _AmNetBuffer;

static size_t _amnet_write_cb(void* ptr, size_t size,
                               size_t nmemb, void* userdata) {
    _AmNetBuffer* buf = (_AmNetBuffer*) userdata;
    size_t total = size * nmemb;
    char* nd = (char*) GC_MALLOC(buf->size + total + 1);
    if (buf->size > 0) memcpy(nd, buf->data, buf->size);
    memcpy(nd + buf->size, ptr, total);
    nd[buf->size + total] = '\0';
    buf->data = nd;
    buf->size += total;
    return total;
}

static AmalgameHttpResponse* _amnet_curl(
        const char*  method,
        code_string  url,
        code_string  body,
        AmalgameMap* headers,
        i64          timeoutMs) {

    CURL* curl = curl_easy_init();
    if (!curl) return _amnet_resp_new(0, NULL, "curl_easy_init failed");

    _AmNetBuffer buf = { NULL, 0 };
    long statusCode  = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _amnet_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amalgame/0.6.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS,
                     timeoutMs > 0 ? (long)timeoutMs : 30000L);

    if (getenv("AMALGAME_SSL_NOVERIFY") != NULL) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    struct curl_slist* curlHeaders = NULL;
    if (headers) {
        AmalgameList* keys = AmalgameMap_keys(headers);
        for (int i = 0; i < keys->size; i++) {
            code_string k = (code_string) keys->data[i];
            code_string v = (code_string) AmalgameMap_get(headers, k);
            if (v) {
                size_t hlen = strlen(k) + strlen(v) + 3;
                char*  h    = (char*) GC_MALLOC(hlen);
                snprintf(h, hlen, "%s: %s", k, v);
                curlHeaders = curl_slist_append(curlHeaders, h);
            }
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    }

    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                         body ? body : "");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                         body ? (long)strlen(body) : 0L);
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (strcmp(method, "PATCH") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    }

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    if (curlHeaders) curl_slist_free_all(curlHeaders);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        return _amnet_resp_new(0, NULL, curl_easy_strerror(res));
    return _amnet_resp_new(statusCode, buf.data, NULL);
}

static inline AmalgameHttpResponse* Http_Get(code_string url) {
    return _amnet_curl("GET", url, NULL, NULL, 0);
}
static inline AmalgameHttpResponse* Http_GetWithHeaders(
        code_string url, AmalgameMap* headers) {
    return _amnet_curl("GET", url, NULL, headers, 0);
}
static inline AmalgameHttpResponse* Http_GetTimeout(
        code_string url, i64 ms) {
    return _amnet_curl("GET", url, NULL, NULL, ms);
}
static inline AmalgameHttpResponse* Http_Post(
        code_string url, code_string body) {
    return _amnet_curl("POST", url, body, NULL, 0);
}
static inline AmalgameHttpResponse* Http_PostJson(
        code_string url, code_string json) {
    AmalgameMap* h = AmalgameMap_new();
    AmalgameMap_set(h, "Content-Type", (void*)"application/json");
    return _amnet_curl("POST", url, json, h, 0);
}
static inline AmalgameHttpResponse* Http_PostWithHeaders(
        code_string url, code_string body, AmalgameMap* headers) {
    return _amnet_curl("POST", url, body, headers, 0);
}
static inline AmalgameHttpResponse* Http_Put(
        code_string url, code_string body) {
    return _amnet_curl("PUT", url, body, NULL, 0);
}
static inline AmalgameHttpResponse* Http_Delete(code_string url) {
    return _amnet_curl("DELETE", url, NULL, NULL, 0);
}
static inline AmalgameHttpResponse* Http_Patch(
        code_string url, code_string body) {
    return _amnet_curl("PATCH", url, body, NULL, 0);
}

#else /* no libcurl */

static inline AmalgameHttpResponse* _amnet_no_curl(code_string url) {
    (void)url;
    return _amnet_resp_new(0, NULL,
        "Http requires libcurl. Install: sudo apt install libcurl4-openssl-dev");
}
static inline AmalgameHttpResponse* Http_Get(code_string u)
    { return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_GetWithHeaders(
        code_string u, AmalgameMap* h)
    { (void)h; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_GetTimeout(code_string u, i64 t)
    { (void)t; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_Post(code_string u, code_string b)
    { (void)b; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_PostJson(code_string u, code_string b)
    { (void)b; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_PostWithHeaders(
        code_string u, code_string b, AmalgameMap* h)
    { (void)b; (void)h; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_Put(code_string u, code_string b)
    { (void)b; return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_Delete(code_string u)
    { return _amnet_no_curl(u); }
static inline AmalgameHttpResponse* Http_Patch(code_string u, code_string b)
    { (void)b; return _amnet_no_curl(u); }

#endif /* AMALGAME_HAS_CURL */

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

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd >= 0 && connect(fd, res->ai_addr, res->ai_addrlen) == 0) {
        c->_fd       = fd;
        c->Connected = true;
    } else if (fd >= 0) {
        close(fd);
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
    close(c->_fd);
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
    AmalgameTcpServer* s =
        (AmalgameTcpServer*) GC_MALLOC(sizeof(AmalgameTcpServer));
    s->Port = port; s->Listening = false; s->_fd = -1;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return s;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t) port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ||
        listen(fd, (int)(backlog > 0 ? backlog : 10)) < 0) {
        close(fd); return s;
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
    int cfd = accept(s->_fd, (struct sockaddr*)&addr, &len);
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
    close(s->_fd); s->_fd = -1; s->Listening = false;
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
    close(c->_fd); c->_fd = -1; c->Connected = false;
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
    AmalgameUdpSocket* s =
        (AmalgameUdpSocket*) GC_MALLOC(sizeof(AmalgameUdpSocket));
    s->_fd = socket(AF_INET, SOCK_DGRAM, 0);
    s->Bound = false; s->BoundPort = 0;
    return s;
}
static inline code_bool UdpSocket_Bind(AmalgameUdpSocket* s, i64 port) {
    if (!s || s->_fd < 0) return false;
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(s->_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        s->Bound = true; s->BoundPort = port; return true;
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
    close(s->_fd); s->_fd = -1;
}

#endif /* AMALGAME_NET_H */
