# Beyond HTTP — nginx/apache-equivalent capabilities

**Status:** in-progress. Verified against the repos **2026-06-04**.
**Shipped:** #6 Static file serving (web v0.13.0, + Range / Last-Modified
/ `.gz` follow-ups), #11 SSE (net-http v0.16.0 + web v0.28.0), #1 Reverse
proxy + #2 Load balancing (`amalgame-net-proxy` v0.2.1: path routing +
round-robin / IP-hash / least-connections). #4's **outbound** SMTP client
shipped separately as `amalgame-net-smtp` v0.2.4 (the inbound relay /
IMAP / POP3 server described here is still roadmap).
**Still roadmap:** #3 TCP/UDP raw proxy, #4 SMTP *server*/IMAP/POP3, #5
SFTP, #7 VPN/WireGuard, #8 CDN, #9 gRPC, #10 DNS/DoH, #12 WebDAV; plus
proxy follow-ups (health checks, circuit breaker) and static `sendfile(2)`.
The inventory below captures what the stack still needs to match nginx /
apache / HAProxy / Postfix territory.

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

### 1. Reverse proxy (HTTP/HTTPS) — ✅ SHIPPED (net-proxy v0.2.x)

> ✅ Shipped as `amalgame-net-proxy` v0.2.1: longest-prefix path
> routing, `X-Forwarded-For` injection, hop-by-hop header stripping,
> wired into Mosaic via `mosaic_server.am` (`AddHandler()`). TODO:
> active health checks, circuit breaker, WS transparent forwarding,
> caching proxy.

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

### 2. Load balancing — 🟢 mostly SHIPPED (net-proxy v0.2.1)

> 🟢 Shipped in `amalgame-net-proxy` v0.2.1: round-robin, IP-hash
> (sticky / weighted), least-connections. TODO: active health checks
> (`/healthz` probe) and outlier detection (auto-eject on N×5xx).

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

### 3. TCP/UDP raw proxy — 🟢 TCP SHIPPED (net-stream v0.1.0)

> 🟢 **TCP shipped** as `amalgame-net-stream` v0.1.0 (`TcpProxy`): binary-safe
> byte splice to a fixed upstream, with security wired in by default —
> SIGPIPE-safe, idle + connect timeouts, global + per-source-IP connection
> caps (over-cap dropped immediately), graceful SIGINT/SIGTERM shutdown,
> bounded splice buffer. The byte pump is C (explicit recv/send lengths)
> because the bundled `Amalgame.Net` TcpConn primitives `strlen()` the
> buffer and truncate at the first NUL — corrupting any binary stream.
> Audit (all green): binary-safety round-trip (all 256 byte values +
> embedded NULs, byte-exact), per-IP cap enforcement, graceful shutdown.
> **TODO v0.2:** UDP forwarding, load-balancing across N upstreams, IPv6
> listener; **v0.3:** TLS edge.

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

### 4. SMTP relay + IMAP/POP3 server — 🟡 outbound client only

> 🟡 The **outbound** half shipped as `amalgame-net-smtp` v0.2.4 (a
> TLS SMTP *client* + `Mail` builder for transactional mail, RFC 2047
> subjects — used by Mosaic contact forms). The **inbound** relay /
> IMAP / POP3 *server* described below is still roadmap.

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

### 6. Static file serving — SHIPPED v0.13.0 (2026-05-24)

Lives in `amalgame-web` v0.13.0 as the `Static` class, wired
into the WebApp pipeline between CSRF validation and the
router.

```amalgame
let app = WebApp.New()
    .WithStatic(Static.New("/assets", "./public").WithCacheMaxAge(3600))
    .Get("/", ctx => HttpResponse.New().Html("<h1>Hi</h1>"))
app.Serve(8080)
```

**Behavior matrix:**

| Status | When |
|---|---|
| 200 | file served, `Content-Type` + ETag (+ `Cache-Control`?) |
| 304 | `If-None-Match` matches the strong ETag (`"size-mtime"`) |
| 403 | exact-prefix request (no dir listing); sub-directory; `../` traversal escape |
| 404 | file missing |
| 405 | method other than GET / HEAD |

**Security:** containment check — normalized full path must
start with normalized root prefix. Blocks `../` even after
`Path_Normalize` collapses the dots, because escaping `..`
removes the root segment itself. No auto-indexing
(intentional — `Options +Indexes` is a recurring source of
accidental exposure).

**Binary safety:** response uses `HttpResponse.File(path)` →
`H1Conn_RespondFile` (net-http v0.9.6), so PNG / JPEG / PDF /
WASM survive NUL bytes intact. This was the big runtime
diff vs the original proposal — `H1Conn_Respond` `strlen()`s
the body, which truncates anything with a NUL.

**MIME coverage:** ~35 extensions baked in (HTML/CSS/JS/WASM
+ image formats + font formats + PDF + archives + YAML/TOML).
Unknown → `application/octet-stream`. Case-insensitive.

**Multi-PR dependency chain:**

1. [Amalgame#543](https://github.com/amalgame-lang/Amalgame/pull/543) — runtime helpers `File_Mtime` / `File_IsFile` / `File_IsDir` (amc v0.8.48)
2. [amalgame-net-http#12](https://github.com/amalgame-lang/amalgame-net-http/pull/12) — `H1Conn_RespondBytes` / `H1Conn_RespondFile` + `HttpResponse.File()` (v0.9.6)
3. [amalgame-web#20](https://github.com/amalgame-lang/amalgame-web/pull/20) — `Static` class + `WithStatic()` builder (v0.13.0)

**Deferred to follow-up PRs** (scope kept tight to ship the
v0.1 quickly — each item below is independent and can be
picked up à la carte):

- **`Range: bytes=N-M` requests.** Needs a runtime
  `H1Conn_RespondFileRange(c, status, ct, path, offset,
  length)` that `fseek`s + sends a slice. Today the whole
  file is sent. Blocks resumable downloads + media seeking.
- **`Last-Modified` / `If-Modified-Since`.** Needs an
  HTTP-date helper in `amalgame-datetime` (today's
  `FormatIso` emits ISO 8601 which `If-Modified-Since`
  consumers don't grok). ETag covers the common
  cache-revalidation path on its own — every browser honors
  `If-None-Match` — so this is cosmetic, not blocking.
- **`sendfile(2)` zero-copy.** `H1Conn_RespondFile`
  currently `GC_MALLOC`s the full body before sending. Fine
  for typical assets (<10 MB), wasteful for large
  downloads. Wire-up: `sendfile(out_fd, in_fd, &offset,
  len)` after sending the headers. Linux only initially;
  `mmap+write` fallback elsewhere.
- **Pre-compressed variant selection.** When `foo.css.gz` /
  `foo.css.br` exists next to `foo.css`, pick it based on
  `Accept-Encoding`. ~30 LoC in `Static.Serve` once the file
  resolution is factored out.
- **HEAD requests skip the body.** Today HEAD is accepted
  (same code path as GET) but still ships the body via
  `RespondFile`. Need a runtime variant that emits the
  headers + Content-Length but skips the bytes.

### 7. VPN — WireGuard binding

The thing nginx/apache don't even pretend to do, but every
infrastructure stack ends up needing. WireGuard is the modern
answer (kernel-fast, simple protocol, batteries-included
clients on every OS).

Three implementation tiers, in increasing ambition:

- **Tier A — `wg-quick` wrapper.** Shell out to the system
  `wg` / `wg-quick` CLI. Trivial (~150 LoC), zero new C deps,
  delegates all crypto + kernel-module work to the existing
  wireguard-tools package. Caller manages config files.
- **Tier B — libwireguard binding.** Dynamic link to the C
  library when available, so config can live in AM-side
  builders (`Wg.NewInterface().AddPeer(...).Up()`) without
  shelling out. Falls back to Tier A when libwireguard isn't
  present.
- **Tier C — pure-AM Noise IKpsk2 implementation.** Re-implement
  the WireGuard handshake + transport in Amalgame on top of
  `amalgame-crypto` (Curve25519, ChaCha20-Poly1305, Blake2s,
  HKDF). Ambitious — the handshake is subtle and audit-prone.
  Probably not worth it; userspace WireGuard exists already
  (boringtun) and binding it is cheaper than rewriting.

**Package:** `amalgame-net-vpn` (new).

**Minimum viable surface (Tier A):**

```amalgame
import Amalgame.Net.Vpn

let iface = Wg.Interface("wg0")
    .PrivateKey(File.ReadAll("/etc/wireguard/wg0.key"))
    .ListenPort(51820)
    .AddPeer(
        Wg.Peer()
            .PublicKey("abc…=")
            .AllowedIPs(["10.0.0.2/32"])
            .Endpoint("vpn.example.com:51820"))

iface.Up()
defer iface.Down()
```

Implementation sketch (Tier A): each builder method appends to
an in-memory config buffer, `Up()` writes it to a temp file +
`wg-quick up <tmp>`. `Down()` likewise. Status helpers
(`iface.PeerStatus()`, `iface.Stats()`) shell `wg show` and
parse the output.

**Dependencies:** `wireguard-tools` system package (Tier A);
`libwireguard.so` (Tier B); none for Tier C beyond
`amalgame-crypto`.

**Crypto note:** WireGuard relies on Curve25519 + ChaCha20-Poly1305
+ Blake2s. `amalgame-crypto` ships HMAC-SHA-256 + JWS-strict
ES256/RS256 today — adding the WG primitive set is a precondition
for Tier B/C. ~300 LoC additional in crypto.

**Priority:** MEDIUM. Two real demand drivers:
- Site-to-site between deploys (replaces tinc / OpenVPN clumsy setups).
- Per-user mesh (Tailscale-equivalent — Amalgame app exposing
  a relay/coordinator endpoint).

### 8. CDN — edge cache + origin pull

Edge caching with origin pull, `Vary`-aware revalidate, geo-routing
hooks. Cloudflare/Fastly territory. Pairs directly with the v0.13.0
`Static` middleware on the origin side: edge nodes pull, cache,
respect `Cache-Control` + ETag + `Last-Modified`, serve to clients.

**Package:** `amalgame-net-cdn` (new). Runs as a standalone binary
that fronts an origin (your `WebApp.Serve(8080)` instance) and
listens on the public port. Or runs as a Mosaic middleware for the
"all-in-one origin + edge" small-scale case.

**Minimum viable surface:**

```amalgame
import Amalgame.Net.Cdn

let edge = new Cdn()
edge.Origin("http://origin.internal:8080")
edge.CacheDir("/var/cache/amalgame-cdn")
edge.MaxCacheSize(50 * 1024 * 1024 * 1024)    // 50 GB
edge.Listen(":80", ":443")                     // termination via amalgame-tls

// Optional: cache-key tweaks
edge.VaryOn(["Accept-Encoding", "Accept-Language"])
edge.PurgeRoute("/_purge")                     // POST /_purge {path:"/x"}

edge.Serve()
```

**Implementation sketch:**

- HTTP/1.1 + HTTP/2 listener (reuse `amalgame-net-http` server).
- LRU cache keyed by `(method, host, path, vary-hash)`. Eviction
  via combination of LRU + age (`Cache-Control: max-age`).
- Storage: filesystem (atomic rename for write, hardlink for
  dedup). RAM index (size, mtime, vary-hash, etag).
- Revalidation: when a cached entry's `max-age` expires, fire a
  conditional GET to origin (`If-None-Match: <etag>` /
  `If-Modified-Since: <date>`). 304 from origin → bump mtime;
  200 → replace.
- Pass-through for `no-store` / `private` / `Authorization` /
  `Set-Cookie` (cache-invalidating semantics, by RFC 7234).
- Purge endpoint: POST `/_purge` with a JSON body of paths;
  authenticated via shared secret in `Authorization`.

**What's deferred to v0.2:**

- **Geo-routing** (multiple edges, DNS-based or Anycast). v0.1
  is single-node — for true geo, you run N edges with the same
  origin, geo-DNS does the rest. The package doesn't need
  awareness of "which edge am I" beyond a config knob.
- **Image-resize at edge** (Cloudflare Polish-style). Out of
  scope — that's a media-processing pipeline, not a CDN.
- **HTTP/3 (QUIC).** Needs `amalgame-net-quic` (not yet
  designed). Defer to v0.3.

**Dependencies:** `amalgame-net-http` (server + client both —
client used for origin pull), `amalgame-tls` (HTTPS termination
at the edge), `amalgame-datetime` (Cache-Control max-age
arithmetic, HTTP-date parsing).

**Priority:** HIGH if you ship at scale; LOW otherwise. The
80/20 path for most Mosaic deployments today is to put Cloudflare
or Fastly in front and skip writing this. But for sovereign /
self-hosted / air-gapped scenarios, having a first-class CDN
package matters. ~6-8 days for v0.1.

### 9. gRPC server — 🟢 servable end-to-end (unary); codegen next

> 🟡 **Two layers shipped:**
> - `amalgame-formats-protobuf` v0.1.0 — proto3 **wire-format codec**
>   (varint, zigzag, length-delimited, fixed32/64, bool, tag, skip),
>   binary-safe on `List<int>`, 7/7 tests (NUL+0xFF byte-exact).
> - `amalgame-net-grpc` v0.1.0 — **gRPC server core** (transport-agnostic):
>   length-prefixed message framing (`GrpcFrame`), status codes, method-path
>   routing, unary dispatch (`GrpcServer.Register/Dispatch/ResponseFrame`,
>   unknown→UNIMPLEMENTED), 7/7 tests.
>
> **Servable end-to-end (unary).** `amalgame-net-http` v0.22.0 added the
> H2 gRPC primitives (`RespondGrpc` → `grpc-status`/`grpc-message`
> trailers; `BodyByteAt` → binary body), and `amalgame-net-grpc` v0.2.0
> added `GrpcServer.ServeH2c(port)` — accept → read `:path` + framed body
> → dispatch → `RespondGrpc`. **Proven end-to-end:** a compiled Amalgame
> gRPC server answers a real `nghttp2` client over TCP — `:status 200` +
> `content-type application/grpc` + `grpc-status 0` + a framed echo body
> byte-exact (`examples/grpc_h2c_server.am`). **Message codegen shipped**
> in `amalgame-formats-protobuf` v0.2.0 (`tools/proto-gen.js`: proto3
> `.proto` → AM message classes with `Encode`/`Decode` — scalars,
> `repeated`, nested messages, binary `bytes`, round-trip tested).
> **Remaining:** gRPC *service*-stub codegen (the `.proto` `service {}`
> blocks → typed `GrpcServer` registration), client stubs, streaming,
> compression, a `grpcurl` interop pass, TLS-fronted (h2) endpoints.

The other half of "modern microservice 2026". HTTP/2 + protobuf,
strongly typed, streaming-capable. amc + nghttp2 already ship the
transport (since `amalgame-net-http` v0.2.0); the missing layer is
the protobuf parser/emitter + the codegen that turns `.proto` IDLs
into AM service classes.

**Package:** `amalgame-net-grpc` (new).

**Minimum viable surface:**

```amalgame
import Amalgame.Net.Grpc
import App.Generated   // emitted by `amc grpc-gen <foo.proto>`

public class GreeterImpl extends App.Generated.GreeterService {
    public override Future<HelloReply> SayHello(HelloRequest req) {
        let r = HelloReply.New()
        r.Message = "Hello, " + req.Name
        return Future.Resolved(r)
    }
}

let srv = GrpcServer.New()
srv.Register(new GreeterImpl())
srv.Serve(50051)
```

**Implementation sketch:**

- **Protobuf v3 parser** in `amalgame-formats-protobuf` (new
  sibling of `amalgame-formats-json`). Pure-AM, ~600 LoC.
  Streams + recursive messages + the wire-format tag/wiretype
  encoding.
- **`.proto` IDL compiler** as an `amc grpc-gen` subcommand
  (or stand-alone `mosaic grpc`). Parses `.proto` files,
  emits AM classes with field accessors + a service trait the
  user extends.
- **gRPC framing** — HTTP/2 trailers (`grpc-status`,
  `grpc-message`), length-prefixed messages, server-streaming
  + client-streaming + bidi via H2 stream IDs.
- **Reflection service** (v1alpha) optional — enables
  `grpcurl` exploration without the `.proto` files.

**Deferred to v0.2:**

- gRPC-Web (browser variant — base64-encoded length-prefixed
  over HTTP/1.1).
- TLS + ALPN h2 termination (just plug `amalgame-tls` on the
  port).
- Interceptors (middleware: auth, retry, deadline propagation).

**Dependencies:** `amalgame-net-http` (H2Server + nghttp2),
`amalgame-formats-protobuf` (new), `amalgame-async` (streaming
endpoints).

**Priority:** HIGH — biggest "missing modern" piece of the stack.
Pair this with `amalgame-net-cdn` and a sovereign deployment is
basically complete. ~3-4 days for v0.1 (server + unary RPC),
+2 days for streaming, +1 day for the codegen polish.

### 10. DNS server (incl. DNS-over-HTTPS)

Authoritative DNS responder, with DNS-over-HTTPS (RFC 8484) as
the headline modern variant. Privacy-aware stacks publish DoH
endpoints alongside their HTTPS API; ad-blocker / split-horizon
self-hosters want full authoritative DNS.

**Package:** `amalgame-net-dns` (new).

**Minimum viable surface:**

```amalgame
import Amalgame.Net.Dns

let zone = Dns.Zone("example.com")
zone.A    ("@",    "192.0.2.10",     300)
zone.A    ("www",  "192.0.2.10",     300)
zone.AAAA ("@",    "2001:db8::10",   300)
zone.MX   ("@",    10, "mx.example.com.", 300)
zone.TXT  ("@",    "v=spf1 mx -all", 300)

let srv = DnsServer.New()
srv.AddZone(zone)
srv.ListenUdp(":53")    // classic UDP
srv.ListenTcp(":53")    // large response fallback
srv.ListenDoh(":443", "cert.pem", "key.pem")  // RFC 8484
srv.Serve()
```

**Implementation sketch:**

- **Wire-format parser/emitter** (RFC 1035 + DNS extensions):
  header, question, answer/authority/additional sections, label
  compression. ~400 LoC pure-AM.
- **Zone file ingestion** (BIND format) optional — config-from-
  TOML is the primary path; BIND-zone is for migrations.
- **DoH layer:** GET/POST `/dns-query` accepting
  `application/dns-message`. Reuses the existing HTTPS server
  (`amalgame-tls` + `amalgame-net-http`).
- **AXFR / IXFR zone transfer**, **DNSSEC signing** — explicit
  v0.2 features. v0.1 = unsigned authoritative-only.

**Recursive resolver:** NOT in scope. The package is
authoritative-only. If you want a recursive caching resolver,
proxy to a public DoH endpoint (Cloudflare / Quad9) via the
reverse-proxy (#1).

**Dependencies:** `amalgame-net-http` (for DoH listener),
`amalgame-tls`. UDP socket support is in the bundled runtime
already (`UdpSocket` since v0.6.x).

**Priority:** MEDIUM. Real demand from:
- DoH endpoints for privacy-aware apps (publish over your
  existing TLS termination).
- Self-hosted home/SMB DNS (replaces BIND + Pi-hole stacks).
- Split-horizon DNS for VPN deployments (pairs with #7 VPN).

~3 days for v0.1 (authoritative + DoH); +2 days for DNSSEC
signing if needed.

### 11. Server-Sent Events (SSE) — ✅ SHIPPED

> ✅ Shipped: `SseConn` in `amalgame-net-http` v0.16.0 + `WebApp.Sse(path,
> handler)` in `amalgame-web` v0.28.0. The sketch below predates the
> final API (`WebApp.Sse` rather than `Sse.NewStream`).

One-way push channel from server to browser over a long-lived
HTTP/1.1 or HTTP/2 connection. Way simpler than WebSocket — no
upgrade dance, no framing, just text lines with a
`text/event-stream` content-type. Native `EventSource` API in
every browser.

**Package:** middleware inside `amalgame-net-http` (small enough
to not warrant a separate package). Lives next to the existing
WebSocket server.

**Minimum viable surface:**

```amalgame
import Amalgame.Net.Http

let app = WebApp.New()
app.Get("/events", ctx => {
    let stream = Sse.NewStream(ctx)
    // Push periodic updates
    stream.Send("counter", "1")
    stream.Send("counter", "2")
    stream.Comment("keep-alive")
    return stream.Response()    // 200 with Content-Type: text/event-stream
})
```

**Implementation sketch:**

- `Sse.NewStream(ctx)` builder that wraps the underlying
  `H1Conn` / `H2Conn`. Each `.Send(event, data)` writes one
  framed event (`event: <name>\ndata: <payload>\n\n`).
- Auto-injects `Cache-Control: no-cache`,
  `Connection: keep-alive` (HTTP/1.1) headers.
- `Sse.Reconnect(id, retryMs)` helpers for the standard
  reconnect protocol.
- Per-stream cleanup on client disconnect (TCP RST handler) —
  reuses the H1Conn closed-callback already in
  `amalgame-net-http`.

**Why prefer over WebSocket:**
- Native browser auto-reconnect.
- Works through HTTP/2 multiplexing (no separate connection
  pool).
- Trivially proxyable through any HTTP/1.1 stack (CDN, reverse
  proxy, load balancer).
- Text-based, easy to debug with `curl -N`.

**Deferred:** binary frames (use WebSocket for those — SSE is
text-only by design).

**Dependencies:** none beyond what `amalgame-net-http` already
links. No new package.

**Priority:** **LOW-HANGING FRUIT** — ~150 LoC, ~half a day, no
new deps. Should ship with the next `amalgame-net-http` patch
release rather than a separate proposal slot.

### 12. WebDAV server

The Apache mod_dav equivalent.  Used for shared filesystems
exposed over HTTP (calendars via CalDAV, contacts via CardDAV,
generic file shares).

**Package:** `amalgame-net-webdav` (new) or middleware in
`amalgame-web`.

**Priority:** LOWEST — niche.  Skip for v0.x.

## Ordering proposal

Year-1 priority for the web stack (✅ = done as of 2026-06-04):

1. ~~**Static file serving** in `amalgame-web`~~ ✅ **shipped
   v0.13.0 (2026-05-24)** (+ Range / Last-Modified / `.gz`
   follow-ups since; `sendfile(2)` still TODO).
2. ~~**Reverse proxy** in `amalgame-net-proxy`~~ ✅ **shipped v0.2.x**.
3. ~~**Load balancing** in `amalgame-net-proxy`~~ ✅ **shipped v0.2.1**
   (RR / IP-hash / least-conn; health checks + outlier detection TODO).
4. **gRPC server** in `amalgame-net-grpc` (#9). 🟢 **servable
   end-to-end (unary)**: protobuf codec v0.1.0 + net-grpc v0.2.0
   (`ServeH2c`) + net-http v0.22.0 (H2 trailers/binary body), proven
   against a real nghttp2 client. Remaining: `.proto` codegen, client
   stubs, streaming, grpcurl interop.
5. ~~**Server-Sent Events (SSE)**~~ ✅ **shipped** (net-http v0.16.0 +
   web v0.28.0).
6. **TCP/UDP raw proxy** in `amalgame-net-stream` — 🟢 **TCP shipped
   v0.1.0** (binary-safe splice + caps/timeouts/graceful shutdown).
   UDP + load-balancing across N upstreams = v0.2.

Year-2 (or punt to "if someone wants it"):

7.  **VPN** via `amalgame-net-vpn` (#7; WireGuard binding,
    Tier A `wg` CLI wrapper first).  ~2-3 days.
8.  **DNS server / DoH** via `amalgame-net-dns` (#10; pairs
    with VPN for split-horizon, with HTTPS termination for DoH).
    ~3 days.
9.  **CDN** via `amalgame-net-cdn` (#8; only if shipping
    sovereign / air-gapped — most deployments will Cloudflare).
    ~6-8 days.
10. **SFTP** via `amalgame-net-ssh` (libssh2 binding).
11. **SMTP relay** via `amalgame-net-smtp`.
12. **WebDAV** (lowest).

### Static follow-ups (à la carte, not blocking the next item above)

- **`Range:` requests** — 1 day (runtime `H1Conn_RespondFileRange` + Static.Serve parsing).
- **HEAD body skip** — 0.5 day (runtime variant of `RespondFile`).
- **Pre-compressed variant selection** — 0.5 day (in Static.Serve only).
- **`Last-Modified` / `If-Modified-Since`** — 1 day (HTTP-date helper in `amalgame-datetime` first).
- **`sendfile(2)`** — 1-1.5 day (Linux-first, mmap fallback later).

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
