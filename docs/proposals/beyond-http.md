# Beyond HTTP — nginx/apache-equivalent capabilities

**Status:** roadmap inventory — not yet started. Captures
features the Amalgame web stack needs to match nginx / apache /
HAProxy / Postfix territory.  No code yet; this is here so the
package boundary + priority is decided before someone starts.

## Why this doc

By v0.5.0 the Mosaic stack covers **the application's HTTP
listener and its TLS/cert lifecycle**.  That's the "Apache as
a web server" half.  The other half — Apache/nginx as a *front
door* — is missing:

- Reverse proxy in front of N upstream servers
- Load balancing across those upstreams
- Generic TCP/UDP proxying (databases, websockets, custom)
- FTP/SFTP/SCP file transfer
- SMTP / IMAP / POP3 mail relay

This roadmap maps each to a candidate package, sketches the
minimum viable surface, and orders them by impact.

## Inventory

### 1. Reverse proxy (HTTP/HTTPS)

The "proxy in front of N app servers" pattern.  nginx + Apache
both ship this as their #1 use case in 2026 — pure-web-server
deployments are rarer than reverse-proxy-fronting-something.

**Package:** `amalgame-net-proxy` (new) — or extend `amalgame-web`
with a `WebApp.MountUpstream(prefix, backend_url)` helper.

**Minimum viable surface:**

```amalgame
import Amalgame.Net.Proxy

let p = new ReverseProxy()
p.Forward("/api", "http://localhost:8081")    // path-prefix route
p.Forward("/", "http://localhost:8080")
p.Header("X-Forwarded-For")                   // injected by default
p.Timeout(30)                                 // upstream connect timeout
p.Serve(80)
```

Implementation sketch:
- HTTP/1.1: parse request, rewrite Host, open upstream conn,
  stream response back.  ~400 LoC.
- HTTP/2: per-stream proxying via nghttp2.  Reuses
  `H2Conn`/`H2Server` types.  ~600 LoC.
- WebSocket transparent forwarding: hijack the connection after
  the Upgrade handshake, splice bytes both ways.  ~100 LoC.
- Sticky sessions via cookie hash → upstream choice.  Part of
  v0.2.x.

**Dependencies:** `amalgame-net-http v0.8.0+` (graceful shutdown
+ SO_REUSEPORT), `amalgame-tls` (for HTTPS termination on the
front side).

**Priority:** HIGH — biggest user-facing gap vs nginx today.

### 2. Load balancing

Often a feature OF the reverse proxy (above), but worth calling
out because the policy choices are non-trivial:

- Round-robin
- Least-connections
- IP-hash (sticky)
- Health checks (`/healthz` probe before sending traffic)
- Outlier detection (auto-remove backends on N consecutive 5xx)

**Package:** part of `amalgame-net-proxy` v0.2.x.

```amalgame
let pool = new UpstreamPool()
pool.Add("http://node1:8080").Weight(2)
pool.Add("http://node2:8080").Weight(1)
pool.Strategy(LoadBalanceStrategy.LeastConnections)
pool.HealthCheck("/healthz", 5)               // every 5s
p.Forward("/api", pool)
```

**Priority:** HIGH — pairs naturally with reverse proxy.

### 3. TCP/UDP raw proxy

The "stream4" / `stream {}` block in nginx, or HAProxy's TCP
mode.  Forwards arbitrary TCP / UDP without parsing the payload.

Use cases:
- Postgres in front of pgbouncer
- Redis Sentinel failover
- MQTT brokers
- Game servers (UDP)

**Package:** `amalgame-net-stream` (new).

```amalgame
import Amalgame.Net.Stream

let s = new StreamProxy()
s.Tcp(5432, "pg-primary.internal:5432")
s.Tcp(6379, "redis-primary.internal:6379")
s.Udp(53, ["dns1.internal:53", "dns2.internal:53"])
s.Serve()
```

Implementation: `accept()` on the listen socket, `connect()` to
the upstream, splice bytes via two threads (one each direction)
until either side closes.  UDP variant uses recvfrom/sendto in
a per-port loop.  ~250 LoC total.

**Dependencies:** none (pure socket I/O).

**Priority:** MEDIUM — useful for production deployments but
not as universally required as HTTP reverse proxy.

### 4. SMTP relay + IMAP/POP3 server

The mail proxy / server slice.  Three sub-protocols, each
non-trivial.

**Packages:**
- `amalgame-net-smtp` — outbound relay + receive
- `amalgame-net-imap` — inbound mail access
- `amalgame-net-pop3` — legacy inbound access (less critical)

```amalgame
import Amalgame.Net.Smtp

let relay = new SmtpRelay()
relay.Listen(25)
relay.OnMessage(msg => {
    // Filter / queue / forward
    Console.WriteLine(msg.From + " → " + String_Join(msg.To, ","))
    return SmtpResult.Accept
})
relay.Forward("smtp.gmail.com:587", "user@gmail.com", "app-password")
relay.Serve()
```

Implementation: SMTP is text-line-based — straightforward to
parse, but the *production* surface (DKIM signing, SPF check,
bounce handling, queue management) is large.  v0.1 ships
inbound receive + outbound forward, no anti-spam.  ~800 LoC.

**Dependencies:** `amalgame-tls` (STARTTLS), `amalgame-crypto`
(DKIM signing later).

**Priority:** LOW — every team needing mail uses Sendgrid /
Postmark / SES / a hosted forwarder.  Self-hosted SMTP is a
hostile environment (deliverability, blocklists, anti-spam),
not a first-class onboarding target.

### 5. FTP / SFTP / SCP

Three different protocols, all about moving files.

- **FTP**: legacy, plaintext + control channel + data channel.
  Pretty much dead outside intranets; modern alternative is
  HTTP-based (S3 / WebDAV / direct uploads).
- **SFTP**: SSH-based, the only one still actively recommended.
  Universally supported by clients.
- **SCP**: deprecated by OpenSSH itself (in favour of SFTP).
  Same wire protocol family — implementing SFTP gets SCP
  semantics ~for free.

**Package:** `amalgame-net-ssh` (new, would also subsume SCP
file transfer + raw SSH session if anyone needs it).

```amalgame
import Amalgame.Net.Ssh

let srv = new SftpServer()
srv.AuthorizedKeys("./authorized_keys")
srv.RootDir("./uploads")
srv.ReadOnly(false)
srv.Serve(22)
```

Implementation: SSH protocol is a *lot* of crypto + framing.
The right approach is binding to libssh2 (smaller surface than
OpenSSH itself) rather than reimplementing.  ~500 LoC of glue
on top of libssh2.

**Dependencies:** `amalgame-crypto` (probably overlaps with
existing JwsKey EVP_PKEY infrastructure), `amalgame-tls` (key
loading / cert chain helpers).

**Priority:** LOW — most uploads in 2026 are HTTP-based
(presigned URLs, multipart POST).  SFTP is still standard
for "drop files in this dir" automation between systems.

### 6. Static file serving

Apache and nginx both have this baked in — return files from
disk with correct MIME type, range requests, ETag, Last-Modified,
optional gzip.

**Package:** part of `amalgame-web` (the framework, since the
typical use is "serve `static/` from my Mosaic app"), via a
`WebApp.Static(prefix, dir)` middleware.

```amalgame
let app = new WebApp()
app.Static("/assets", "./static")             // GET /assets/foo.css
app.Static("/", "./public")                   // SPA fallback
```

Implementation: O_RDONLY + sendfile() syscall on Linux for
zero-copy.  Range parsing (`Range: bytes=0-1023`).  ETag from
`stat()` mtime + size.  ~300 LoC.

**Priority:** HIGH — almost every Mosaic app needs `/assets`.
Currently users have to write a route per file or copy nginx's
config style by hand.

### 7. WebDAV server

The Apache mod_dav equivalent.  Used for shared filesystems
exposed over HTTP (calendars via CalDAV, contacts via CardDAV,
generic file shares).

**Package:** `amalgame-net-webdav` (new) or middleware in
`amalgame-web`.

**Priority:** LOWEST — niche.  Skip for v0.x.

## Ordering proposal

Year-1 priority for the web stack:

1. **Static file serving** in `amalgame-web` (impact: every
   Mosaic app benefits).  ~1 day.
2. **Reverse proxy** in `amalgame-net-proxy` v0.1 (impact:
   unblocks "front of N upstreams" deploys).  ~2-3 days.
3. **Load balancing** in `amalgame-net-proxy` v0.2 (extends 2).
   ~1-2 days.
4. **TCP/UDP raw proxy** in `amalgame-net-stream` (impact: DB +
   broker fronts).  ~1 day.

Year-2 (or punt to "if someone wants it"):

5. **SFTP** via `amalgame-net-ssh` (libssh2 binding).
6. **SMTP relay** via `amalgame-net-smtp`.
7. **WebDAV** (lowest).

## What's NOT in scope

- **Modules / dynamic plugins** (mod_*.so).  nginx's plugin
  system is its own complexity tax; Amalgame's compile-time
  package add gets us 90% of the equivalent without runtime
  fragility.
- **Configuration as a DSL.** nginx's config syntax is a deep
  source of foot-guns.  Mosaic uses TOML + AM code, full stop.
- **`mod_php` / FastCGI / WSGI.**  Amalgame programs run in-
  process via Mosaic, not behind a runtime separation.  If you
  need to front a non-Amalgame app, use the reverse proxy
  (#1) — that's exactly what it's for.

## Acceptance criteria for any of the above

Same bar as the existing web-stack packages:

- `amc package add <name>` works.
- `[dependencies]` table declares transitive needs.
- CI tests against the latest stable amc release.
- README ships a "10-line example".
- Indexed in `amalgame-lang/packages-index`.
- Documented in `docs/mosaic-configuration.md` when it
  intersects Mosaic's TOML schema (`[proxy]`, `[static]`, etc.).
