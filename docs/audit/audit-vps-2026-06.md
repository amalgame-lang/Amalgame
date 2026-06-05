# Amalgame end-to-end VPS audit — June 2026

**Status:** planning → execution
**Infra:** IONOS VPS (Ubuntu 24.04 LTS, public IPv4+IPv6, multi-region)
**Scope:** AMC (compiler/distribution), Mosaic (web framework), Pollen (distributed P2P bus)
**Scale:** 8+ VPS (large) — enables real HA failover + a multi-region Pollen demo video
**Priorities (all four):** Mosaic ACME migration · Pollen multi-region video · deep security audit · clean AMC distribution

> This document is the single source of truth for the campaign. Result tables start
> empty; fill `Status` (PASS/FAIL/N-A) and `Notes` as tests run. Every FAIL feeds the
> correction loop (§6) and gets a row in §8.

---

## 1. Objective

Prove **end-to-end on the public Internet**, outside the dev environment, that the three
pillars actually work for a real user on a clean machine:

| Pillar | What we concretely prove |
|---|---|
| **AMC** | The install/distribution story holds on a pristine host (release binary, `install.sh`, build-from-source bootstrap, test suite, compiling a real program with an external package). |
| **Mosaic** | A real site served over public HTTPS: real ACME/Let's Encrypt, SNI multi-domain, static files, forms (SMTP+reCAPTCHA), sessions, systemd service, observability, load behaviour. Also completes the **ACME Phase 3 migration** (node sites → Mosaic). |
| **Pollen** | A genuinely **distributed** P2P bus across N geographically separate hosts: transport, workflow execution tree, capability discovery, power-of-two load balancing, HA failover. |

Each test has an explicit **success criterion**. FAIL → issue → fix → redeploy → re-test (§6).

---

## 2. Resource plan (VPS allocation)

Sizing notes:
- AMC bootstrap + gcc + Boehm GC is RAM-hungry → **min 2 GB** on any node that *compiles*.
  Pollen exec-only nodes can stay on VPS S (1 GB).
- **arm64**: out of IONOS scope — ARM64 stays validated on the Raspberry Pi (already in CI/release).
- **DNS**: use `amalgame.me` subdomains for real ACME (HTTP-01). Provision before issuing certs.
- **Security baseline (day 0, every host)**: SSH key-only (disable password), UFW/nftables
  firewall (allow 22/80/443 + Pollen ports only), unattended-upgrades, non-root deploy user.
  ⚠️ Purge any secrets from configs before exposing anything publicly.

### Proposed allocation (~9 VPS)

| Host | Role | Region | Size | Open ports |
|---|---|---|---|---|
| `vps-amc-01` | AMC clean install + build + test | DE | M (2 vCPU / 2-4 GB) | 22 |
| `vps-mosaic-01` | Public Mosaic site, ACME, SNI, forms | DE | M | 22, 80, 443 |
| `vps-mosaic-02` | 2nd site / migration target / 2nd SNI domain | ES | M | 22, 80, 443 |
| `vps-pollen-mgr` | Pollen Manager UI + controller | DE | M | 22, 443, Pollen |
| `vps-pollen-01` | Pollen exec node | DE | S (1 vCPU / 1 GB) | 22, Pollen |
| `vps-pollen-02` | Pollen exec node | ES | S | 22, Pollen |
| `vps-pollen-03` | Pollen exec node | UK | S | 22, Pollen |
| `vps-pollen-04` | Pollen exec node | US | S | 22, Pollen |
| `vps-pollen-05` | Pollen exec node (HA spare for failover demo) | US | S | 22, Pollen |

Multi-region (DE/ES/UK/US) is deliberate: it makes the Pollen video credible and exercises
real inter-DC latency. Scale exec nodes up/down without touching the rest of the plan.

---

## 3. AMC test plan (`vps-amc-01`, pristine host)

| # | Test | Success criterion | Status | Notes |
|---|---|---|---|---|
| A1 | `install.sh` from release on bare Ubuntu 24.04 | `amc --version` OK, no missing deps | | |
| A2 | Build from source (bootstrap snapshot) | `./build_amc.sh` green (~10s), no migrated-header error | | |
| A3 | AM test bundles (`./amc test ./tests/<bundle>/`) | 100% PASS | | |
| A4 | Compile + link a real program (hello + indexed external package) | Binary produced, runs, clean link | | |
| A5 | `ldd ./amc` | No residual `-lcurl` | | |
| A6 | Package resolution via packages-index (download + build) | An `amalgame-*` package installs and compiles | | |

---

## 4. Mosaic test plan — exhaustive (`vps-mosaic-01`, `vps-mosaic-02`, public + DNS)

Goal: exercise **every** Mosaic feature that ships today, and **probe the behaviour of the
planned/unfinished ones** so the report states exactly where the framework stands. Status
legend per row:
- **SHIPPED** — feature exists and is expected to PASS.
- **PARTIAL** — exists with a documented limitation; test verifies the limit, doesn't fail the audit.
- **PLANNED** — not built yet; the "test" probes current behaviour and records the gap (expected: graceful absence, not a crash).

Versions under test (fill at run time): amalgame-web `v____`, net-http `v____`, tls `v____`,
net-smtp `v____`, net-proxy `v____`.

### 4.A — Serving & routing

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MA1 | `WebApp.Serve` / `ServeMt` / `ServeWith` (HTTP) | Single + multi-thread serve OK, config knobs honoured | SHIPPED | | |
| MA2 | `WebApp.ServeAsync` / `ServeAsyncWith` (Linux epoll) | Async fiber serve OK under concurrency | SHIPPED | | |
| MA3 | `WebApp.ServeHttps` / `ServeHttpsMt` (in-process TLS) | TLS terminated in-process, no external proxy | SHIPPED | | |
| MA4 | `MosaicServer` multi-site Host dispatch | N sites by Host header, correct app per host | SHIPPED | | |
| MA5 | `MosaicServer` `.AddHandler` raw handler (proxy hook) | Raw closure handler dispatched per host | SHIPPED | | |
| MA6 | `:80 → :443` redirect + www canonicalization | 301/308 to https, www→apex (or apex→www) per config | SHIPPED | | |
| MA7 | Programmatic routing (`:param`, `*splat`, methods) | Params + splat captured, first-match-wins | SHIPPED | | |
| MA8 | Filesystem routing (`app/` `[id]` / `[...slug]`) | `mosaic build` wires routes from `app/` tree | SHIPPED | | |
| MA9 | HTTP/1.1 keep-alive (persistent connections) | Multiple requests on one connection | PARTIAL | | Phase-1 hardening item — verify |
| MA10 | IPv6 dual-stack (`AF_INET6`, `IPV6_V6ONLY=0`) | Serves over IPv6 + IPv4 | SHIPPED | ✅ PASS | net-http v0.21.0: H1 + HTTPS-H1 (incl. async) bind dual-stack, AF_INET fallback. Live audit 2026-06-05 (`ipv6_smoke.c` vs real header): `::1` IPv6 client accepted + `127.0.0.1` IPv4 client normalized to plain `127.0.0.1` (not `::ffff:`) so RemoteAddr/rate-limit/Host-guard semantics preserved. WS/H2 listeners still IPv4 (tracked). Re-run over public IPv6 on VPS |
| MA11 | Graceful shutdown (SIGTERM drains in-flight) | In-flight requests complete, then exit | PARTIAL | | Verify vs. hard kill |

### 4.B — TLS / certificates / ACME

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MB1 | **Real ACME** Let's Encrypt **prod** (http-01, native RFC 8555) | Valid cert, chain OK (`openssl s_client`), not staging | SHIPPED | | |
| MB2 | ACME **staging** dry-run first | Staging cert issued before touching prod (rate-limit safe) | SHIPPED | | |
| MB3 | Multi-SAN cert (`Domains`/`DomainsCsv`) | One cert covering ≥2 SANs | PARTIAL | | net-http v0.12 multi-SAN — verify support |
| MB4 | **SNI multi-domain** (one listener, N certs) | Correct cert served per SNI host (`AddSni`) | SHIPPED | | net-http v0.12.0 |
| MB5 | Auto-renewal (`NeedsRenewal` + embedded thread) | Force near-expiry → re-issue | SHIPPED | | |
| MB6 | Renewal **without restart** (live cert swap) | New cert served without bouncing the process | PLANNED | | Known limit: swap needs restart — record |
| MB7 | TLS min-version floor (`WithMinVersion 12/13`) | TLS 1.2 floor default; 1.3-only when set; no TLS ≤1.1 | SHIPPED | | |
| MB8 | ALPN negotiation (`h2,http/1.1`) | Advertised protos; http/1.1 fallback path | PARTIAL | | select callback may hardcode h2 — verify |
| MB9 | mTLS client-cert auth (`WithClientAuth`) | Client cert required + `PeerCertSubject` extracted | PARTIAL | | verify on net-http path |
| MB10 | OCSP stapling | Stapled OCSP response in handshake | PLANNED | | record absence |
| MB11 | dns-01 challenge (wildcards) | Wildcard cert via DNS provider | PLANNED | | record absence |
| MB12 | tls-alpn-01 challenge | In-process challenge, no port-80 | PLANNED | | record absence (http-01 is the real path) |
| MB13 | **ACME Phase 3 migration** (node site → Mosaic) | A real site fully served by Mosaic, node fallback removable | PREPPED | ⏳ awaiting supervised execution | Capability shipped (tls v0.3.3 + net-http v0.21.0 + web v0.33.0). **Runbook: [`acme-cutover-runbook.md`](./acme-cutover-runbook.md)** (pre-flight + staging dry-run + cutover + verify + rollback). NOT auto-executed — touches prod/DNS/real LE. Deadline ~2026-07-27 (day-30); expiry ~2026-08-27 |

### 4.C — Response middleware & headers

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MC1 | `SecurityHeaders.StrictHtml` / `StrictApi` presets | CSP/HSTS/XFO/nosniff/Referrer/Permissions/COOP/COEP stamped | SHIPPED | | functional; attacks in §7 |
| MC2 | CORS preflight + response decoration | 204 preflight, Allow-Origin/Vary correct | SHIPPED | | |
| MC3 | CSRF double-submit cookie | 403 on mismatch, safe methods bypass | SHIPPED | | form-body token = PARTIAL (header-only) |
| MC4 | RateLimit per-IP (memory backend) | 429 + Retry-After over limit | SHIPPED | | |
| MC5 | RateLimit Redis backend (multi-node) | Shared counter across instances | SHIPPED | | fail-open if Redis down — verify |
| MC6 | gzip Compression (`Accept-Encoding`) | text/* compressed, Vary: Accept-Encoding | SHIPPED | | |
| MC7 | PoweredBy (X-Powered-By + Server) | Present by default, toggleable | SHIPPED | | |
| MC8 | Handler-wins precedence | Handler's own headers not overwritten by middleware | SHIPPED | | |

### 4.D — Auth & sessions

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MD1 | BasicAuth (RFC 7617) + realm-injection guard | 401 + `WWW-Authenticate`, CRLF in realm stripped | SHIPPED | | |
| MD2 | JWT HS256 bearer (sig + exp + nbf) | Valid token passes, expired/nbf rejected | SHIPPED | | alg=none attack in §7 |
| MD3 | OAuth2 auth-code (Github + Google) | StartLogin redirect + HandleCallback userinfo | SHIPPED | | needs real OAuth app creds |
| MD4 | `ProtectedRoutes` group gating | Protected routes 401 without auth | SHIPPED | | |
| MD5 | Session — MemorySessionStore | 256-bit id, HttpOnly+Secure+SameSite, survives across reqs | SHIPPED | | |
| MD6 | Session — SignedCookie (HMAC, signed-only) | Tamper → rejected; plaintext visible (documented) | SHIPPED | | |
| MD7 | Session — Encrypted cookie (AES-256-GCM) | Confidential, AEAD tag verified, nonce unique per write | SHIPPED | | |
| MD8 | Session — RedisSessionStore | Shared session across instances | SHIPPED | | |
| MD9 | JWT RS256 / JWKS (multi-issuer) | RS256 verified per issuer | PLANNED | | record absence (v0.17) |
| MD10 | OAuth2 PKCE + OIDC id_token | PKCE challenge + id_token validated | PLANNED | | record absence (v0.18) |
| MD11 | WebAuthn / passkeys | Passwordless registration + assertion | PLANNED | | record absence (Phase 2) |
| MD12 | TOTP / 2FA (RFC 6238) | Time-based OTP verified | PLANNED | | record absence (Phase 2) |

### 4.E — Content, streaming & uploads

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| ME1 | Static serving (`public/` auto-pairing) | Correct MIME (~35 types), prefix routing | SHIPPED | | |
| ME2 | ETag + If-None-Match → 304 | Conditional GET returns 304 | SHIPPED | | |
| ME3 | Last-Modified + If-Modified-Since → 304 | Date-based conditional GET | SHIPPED | | v0.27.0 |
| ME4 | Range requests (206 / 416) | Single-range partial content; 416 unsatisfiable | SHIPPED | | v0.27.0 |
| ME5 | Pre-compressed `.gz` variant selection | Serves `.gz` when present + Accept-Encoding | SHIPPED | | v0.27.0 |
| ME6 | Template engine auto-escape (`{{x}}` vs `{{{x}}}`) | HTML-escaped by default, raw opt-out explicit | SHIPPED | | XSS context probe in §7 |
| ME7 | Template sections / inverted / partials | `{{#}}`,`{{^}}`,`{{>}}` render correctly | SHIPPED | | list iteration = limitation |
| ME8 | Multipart upload (`ctx.Multipart()`) | Binary-safe file parts, fields extracted | SHIPPED | | filename sanitization probe in §7 |
| ME9 | Server-Sent Events (`WebApp.Sse`) | Long-lived `text/event-stream`, push until disconnect | SHIPPED | | v0.28.0 |
| ME10 | WebSocket text frames (RFC 6455) | Handshake + echo text | SHIPPED | | |
| ME11 | WebSocket subprotocol negotiation | `Sec-WebSocket-Protocol` selected | SHIPPED | | v0.17.0 |
| ME12 | WebSocket binary frames + fragmentation | Binary send/recv, multi-frame messages | PLANNED | | record absence |
| ME13 | `sendfile(2)` zero-copy for large static | No full-body malloc on large files | PLANNED | | perf note |

### 4.F — Forms, mail & integrations

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MF1 | Contact form end-to-end | reCAPTCHA siteverify OK + SMTP-TLS mail received | SHIPPED | | net-smtp 465 |
| MF2 | SMTP HTML + attachment + accented Subject (RFC2047) | Mail delivered with PJ + accents intact | SHIPPED | | net-smtp v0.2.2 |
| MF3 | Database backend smoke (sqlite + 1 server DB) | Query round-trip, typed results | SHIPPED | | optional: pg/mysql/redis |

### 4.G — Observability & config

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MG1 | Access log (enriched, one line/request) | method/path/status/duration/remote/UA logged | SHIPPED | | `/var/log/mosaic-sites.log` |
| MG2 | Request-ID generation + `X-Request-ID` propagation | Stable id per request through chain | PARTIAL | | wiring TODO — verify |
| MG3 | JSON log format | Structured JSON lines | SHIPPED | ✅ PASS | web v0.33.0 `LogConfig.WithFormat("json")` → one JSON object/line via `Log.Raw` (logging v0.2.0). Local audit 2026-06-05: fields JSON-escaped — **CR/LF/quote log-injection neutralized** (unit test on `jsonEsc`). Re-run on VPS log pipeline once provisioned |
| MG4 | TOML 3-layer config (toml < env < flag) via `FromMap` | Each layer overrides correctly | PARTIAL | | TOML→FromMap wiring partly doc-only — verify |
| MG5 | `/healthz` + `/readyz` probes | Liveness/readiness endpoints | SHIPPED | ✅ PASS | web v0.33.0 `WithObservability` — public 200 liveness/readiness. Unit test green |
| MG6 | Prometheus `/metrics` | Request histograms, in-flight, queue depth | SHIPPED | ✅ PASS | web v0.33.0 — requests by status class + total + in-flight gauge + duration sum, ServeMt-safe counters. **SECURITY: private-by-default** — served only to loopback/RFC1918/ULA or with bearer token; public caller → 404 (endpoint not disclosed). Abuse test green (public→404, token→200, wrong-token→404) |
| MG7 | OpenTelemetry tracing | Spans propagated through middleware | PLANNED | | record absence (Phase 3) |

### 4.H — CLI, build & deploy

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MH1 | `mosaic dev` hot reload | File watch → dlopen reload, livereload | SHIPPED | | |
| MH2 | `mosaic build --mono` single static binary | One binary, no `.so`, no fs deps beyond `data/` | SHIPPED | | |
| MH3 | `mosaic build --modular` + `mosaic serve` | `app.so` + SIGHUP reload | SHIPPED | | |
| MH4 | `mosaic service` (systemd) | `enabled`+`active`, survives reboot, Type=notify | SHIPPED | | |
| MH5 | Worker-pool concurrency (bounded MPSC) | N workers, queue depth bounded | SHIPPED | | |

### 4.I — Beyond-HTTP / reverse proxy (separate roadmap)

| # | Test | Success criterion | Status | Result | Notes |
|---|---|---|---|---|---|
| MI1 | Reverse proxy (`amalgame-net-proxy`) | Path-based dispatch, X-Forwarded-* injected | PARTIAL | | verify current state of net-proxy |
| MI2 | Load balancing (round-robin / least-conn / health) | Backend pool, health checks, circuit breaker | PLANNED | | record state |
| MI3 | TCP/UDP raw pass-through proxy | Stream proxy for DB/broker | TCP SHIPPED | ✅ PASS | `amalgame-net-stream` v0.1.0 `TcpProxy`. Local audit 2026-06-05 all green: **binary-safety** (all 256 byte values + embedded NULs round-trip byte-exact — the bundled `Amalgame.Net` TcpConn would `strlen`-truncate), **per-source-IP cap** (over-cap conn dropped, admitted conn served), **graceful SIGTERM**. Security by default: SIGPIPE-safe, idle+connect timeouts, global+per-IP caps. SSRF guard intentionally N-A (upstream operator-fixed). TODO: UDP/LB/IPv6/TLS (v0.2-v0.3). Re-run on VPS once provisioned. |
| MI4 | gRPC server (HTTP/2 + protobuf) | grpc-gen + serve | TRANSPORT+CORE | 🟡 partial | 3 layers shipped (local audit 2026-06-05): `amalgame-formats-protobuf` v0.1.0 (proto3 codec, 7/7) + `amalgame-net-grpc` v0.1.0 (framing/status/path/dispatch, 7/7) + **`amalgame-net-http` v0.22.0 H2 gRPC primitives** (`RespondGrpc` trailers + `BodyByteAt` binary body). **Transport gap CLOSED + wire-proven**: a real nghttp2 client smoke gets `:status 200` + `application/grpc` + `grpc-status`/`grpc-message` TRAILERS + NUL/0xFF body byte-exact. Remaining: `GrpcServer.ServeH2c` serve loop + grpcurl e2e + `.proto` codegen |

---

## 5. Pollen test plan (`vps-pollen-mgr` + 5 exec nodes, distributed)

| # | Test | Success criterion | Status | Notes |
|---|---|---|---|---|
| P1 | Node bootstrap + mutual discovery | All nodes see each other (mesh) | | |
| P2 | **TCP transport** (canonical decision) | Inter-node traffic over TCP; document if installer still forces UDP | | |
| P3 | Workflow tree (if/while/for/set) cross-host | Tree executes across ≥3 physical hosts, coherent treePath | | |
| P4 | Capability discovery | A node advertises a capability, others see it | | |
| P5 | Power-of-two load balancing | Load spread ~evenly across capable nodes | | |
| P6 | HA failover | Kill a node → work resumes elsewhere, workflow continues | | |
| P7 | Remote Manager UI | Edit workflow.json + view DAG/flowchart from manager VPS | | |
| P8 | Real multi-region latency | Inter-DC measurements, acceptable behaviour | | |

> Known gaps to record as findings (not blockers): program still defaults to UDP; manager
> DAG needs a global rethink (do **not** patch it one more time during the audit).

---

## 6. Correction / re-test loop

For every failure:
1. **Capture** — logs + exact command + exact output (no "doesn't work").
2. **Issue** — GitHub issue on the right repo (amc / amalgame-web / pollen) with minimal repro.
3. **Fix** — feature branch + PR (reminder: `develop` is protected, no direct push).
4. **Redeploy** — new release/binary to the affected VPS.
5. **Re-test** — replay the test + a non-regression check.
6. **Record** — log before/after status, PR number, fixed version in §8.

---

## 7. Cross-cutting audit dimensions

1. **Functional** — the tables above.
2. **Security (deep)** — full penetration battery, §7-bis below.
3. **Performance** — throughput, latency, memory (GC) footprint.
4. **Reliability / HA** — reboot, crash, failover.
5. **Observability** — logs, diagnosability.

---

## 7-bis. Cybersecurity penetration battery (deep)

This is an **active** security audit against the live Mosaic site(s) on `vps-mosaic-01/02`,
plus a source-informed review of the stack. Each probe states the expected **secure**
behaviour; a deviation is a FINDING (severity in the last column) that feeds §6 and §8.

> ⚠️ Only run against our own VPS. Run destructive/fuzzing probes against the **staging
> domain** first. Snapshot the VPS before the battery so we can roll back.

**Toolbox (install on a dedicated `vps-redteam` or run from a laptop):** `nmap`,
`testssl.sh`, `sslyze`, `openssl s_client`, OWASP `ZAP`, `nikto`, `ffuf`/`gobuster`,
`sqlmap` (for the DB-backed form), `h2spec`, `nuclei`, `wrk`/`vegeta` (load), and the
HTTP-smuggling probe `smuggler.py` (defaforce) — plus small custom `curl` scripts.

### S1 — TLS / transport

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S1.1 | `testssl.sh` / `sslyze` full scan | No SSLv3/TLS1.0/1.1, no NULL/EXPORT/RC4/3DES, FS ciphers only | High | | |
| S1.2 | TLS ≤1.1 downgrade attempt | Handshake refused | High | | |
| S1.3 | Cert chain + hostname + expiry (`s_client`) | Valid LE chain, SAN matches, not staging | High | | |
| S1.4 | Cipher suite control gap | Record: ciphers are OpenSSL-default, not app-configurable | Info | | known gap |
| S1.5 | OCSP stapling present? | Record absence (planned) | Low | | |
| S1.6 | TLS session resumption / 0-RTT replay | If 1.3 0-RTT enabled, non-idempotent requests safe | Med | | verify tickets behaviour |
| S1.7 | SNI: request wrong host on a cert | Correct cert per SNI, no default-cert leak | Med | | |

### S2 — HTTP request hardening / smuggling

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S2.1 | **Content-Length + Transfer-Encoding both set** | Reject (400) or RFC-7230 TE-wins; no desync | **High** | | flagged weakness — behaviour undocumented |
| S2.2 | `smuggler.py` CL.TE / TE.CL / TE.TE suite | All report "no smuggling" | **High** | | |
| S2.3 | Oversized headers (> max_header_bytes) | 431/400, connection closed, no OOM | Med | | |
| S2.4 | Oversized body (> max_body_bytes) | 413, no OOM | Med | | |
| S2.5 | Single huge chunk near body limit | Limit enforced per total, no OOM | Med | | no per-chunk cap — probe |
| S2.6 | Slowloris (drip headers) | Cut by header/idle timeout | High | | |
| S2.7 | Slow body (drip body) | Cut by body timeout | High | | async path only? verify sync |
| S2.8 | Many idle keep-alive connections | Bounded by timeout; no connection exhaustion | Med | | no connection-level cap — probe |
| S2.9 | Request-line with double spaces / bare LF | Strict parse or safe handling, no proxy desync | Med | | lenient parser — probe |
| S2.10 | Method fuzzing (`GETT`, junk verbs) | 400/405, never mis-dispatched | Low | | |

### S3 — Injection & output encoding

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S3.1 | Reflected XSS via query/body into template | `{{x}}` escaped; payload inert in HTML context | High | | |
| S3.2 | **XSS in JS/CSS/URL context** (`<script>{{x}}</script>`, `href="{{x}}"`) | Probe: escaper is HTML-context-only → may be unsafe in JS/attr | **High** | | context-unaware escaping — document + test |
| S3.3 | Response header injection (`\r\n` in header value) | CRLF stripped, header dropped (guard) | High | | |
| S3.4 | Path traversal in static (`../`, encoded `%2e%2e`) | 403, never serves outside root | High | | |
| S3.5 | **Path traversal via symlink** (`public/link -> ../`) | Probe: no symlink-escape guard → test reachability | **High** | | flagged weakness |
| S3.6 | Open redirect (`Redirect(userUrl)`) | External/`//evil.com` redirect refused or validated | High | | Phase-1 hardening item |
| S3.7 | SSRF via any server-side fetch (`HttpClient.Get`) | RFC1918 / link-local refused by default | High | | Phase-1 hardening item |
| S3.8 | Multipart `filename="../../etc/passwd"` | App-side `SaveTo` ignores client path; no traversal | Med | | filename unvalidated — verify usage |
| S3.9 | SQL injection on the DB-backed contact form (`sqlmap`) | Parameterized queries; no injection | High | | |
| S3.10 | Host header injection (cache poisoning / link gen) | Host validated against site allow-list | Med | | |

### S4 — Auth, session & tokens

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S4.1 | **JWT `alg=none`** forged token | Rejected (alg whitelist HS256) | **High** | | confirm reject |
| S4.2 | JWT signature tamper / bit-flip | Rejected (constant-time HMAC compare) | High | | |
| S4.3 | JWT expired / nbf-future | Rejected | High | | |
| S4.4 | JWT HS256 key confusion (pass pubkey as secret) | N/A (RS256 absent) — record | Info | | |
| S4.5 | Session id entropy | 256-bit CSPRNG, unpredictable | High | | |
| S4.6 | Session fixation (reuse pre-login id) | Id rotated on privilege change | Med | | verify app pattern |
| S4.7 | Cookie flags (HttpOnly/Secure/SameSite) | Session: all set; CSRF cookie HttpOnly=false by design | Med | | |
| S4.8 | Encrypted-cookie tamper (flip ciphertext/tag) | GCM tag fails → rejected | High | | |
| S4.9 | Encrypted-cookie **nonce uniqueness** | Distinct nonce every write (GCM safety) | High | | |
| S4.10 | Signed-only cookie confidentiality | Confirm data is plaintext-visible (documented), no secrets stored there | Med | | |
| S4.11 | BasicAuth over plaintext HTTP | Only offered over TLS; creds never on `:80` | High | | |
| S4.12 | Brute-force login (no lockout) | RateLimit covers auth endpoints | Med | | layer rate-limit on auth |
| S4.13 | OAuth2 `state` CSRF / fixation | State cookie verified on callback | High | | |

### S5 — Access control / middleware logic

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S5.1 | Reach a `Protected` route without creds | 401, never leaks body | High | | |
| S5.2 | CSRF bypass (missing/forged token on POST) | 403; OPTIONS/GET bypass is intentional | High | | |
| S5.3 | CSRF exempt-path over-match (prefix) | Only intended paths exempt | Med | | prefix-match coarseness |
| S5.4 | CORS wildcard + credentials | Wildcard NOT echoed when credentials | High | | |
| S5.5 | CORS origin reflection (arbitrary Origin echoed) | Only allow-listed origins echoed | High | | |
| S5.6 | CSP actually blocks inline script | `default-src 'self'` blocks inline | Med | | no nonce support — note |
| S5.7 | Security headers present on every route + errors | Headers on 404/500 too, not just 200 | Med | | |
| S5.8 | HSTS only on HTTPS (not on `:80`) | No HSTS on plain HTTP | Low | | |

### S6 — Rate limiting / DoS

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S6.1 | Exceed per-IP limit | 429 + Retry-After | Med | | |
| S6.2 | **Fixed-window boundary burst** (2× at window edge) | Record over-admit (≤2×) — known limit | Med | | sliding-window planned |
| S6.3 | **X-Forwarded-For spoof** behind proxy | Key must be real client IP, not spoofable / not collapsed to proxy IP | **High** | | trusted-proxy handling — flagged |
| S6.4 | Redis backend down → fail-open | Record: traffic allowed when Redis down | Med | | fail-open by design — document |
| S6.5 | Redis INCR/EXPIRE race (no TTL key) | Keys always get TTL | Low | | race window — probe |
| S6.6 | App-layer flood / `vegeta` sustained load | Stays up, degrades gracefully, recovers | Med | | |

### S7 — Infrastructure & hygiene

| # | Probe | Expected secure behaviour | Sev | Status | Notes |
|---|---|---|---|---|---|
| S7.1 | `nmap` full port scan of each VPS | Only intended ports open (22/80/443 + Pollen) | High | | |
| S7.2 | SSH posture | Key-only, no root login, fail2ban/limits | High | | |
| S7.3 | Firewall default-deny | Inbound default-deny, egress sane | High | | |
| S7.4 | Secret hygiene (repo + deployed configs) | No secrets in git history or world-readable on disk | High | | purge before exposing |
| S7.5 | File perms / run-as-non-root | Service runs as non-root, data dir not world-writable | Med | | |
| S7.6 | Unattended security upgrades | Enabled | Low | | |
| S7.7 | `nikto` + `nuclei` baseline | No known-CVE / misconfig hits | Med | | |
| S7.8 | Error pages don't leak internals | No stack traces / paths / versions in 5xx | Med | | PoweredBy reveals server id by design — note |
| S7.9 | ACME http-01 port-80 exposure window | Port 80 only open during challenge, minimal surface | Low | | |

### Severity & reporting

- Severities: **High / Med / Low / Info**. Every **High** finding blocks the "production-ready"
  claim until fixed (→ §6 loop) and gets a row in §8.
- The probes flagged **bold High** (S2.1/S2.2 smuggling, S3.2 context-escaping, S3.5 symlink
  traversal, S4.1 JWT alg=none, S6.3 XFF spoof) are the **a-priori weak spots** from the
  source review — prioritize these first.
- Deliverable: a one-page security scorecard (counts per severity, per pillar) at the top of
  §6 of the final report.

---

## 7-ter. Security remediation backlog — gaps with NO current defense

These are real gaps found during source review (verified June 2026). They have **no
mitigation today**; several are **not even on the roadmap**. Goal = zero identified gap left
open before the audit signs off. Each item: build the defense (target repo) **and** prove it
with the linked §7-bis probe (which must flip from FINDING → secure).

> **Operational bar (per "aucune faille"):** absolute zero-vuln is not provable. The
> achievable bar is: **every gap below closed + defense-in-depth + the §7-bis battery gates
> the release (any open High = no "production-ready") + adversarial re-verification.**

| ID | Gap | Build (repo / file) | Proves (§7-bis) | Sev | Status |
|---|---|---|---|---|---|
| **G1** | **Email header injection** — reject CR/LF in From/To/Reply-To/Subject; sanitize attachment filenames; dot-stuff DATA | amalgame-net-smtp `mail.am` | `examples/mail_security.am` | **High — LIVE on prod contact form** | ✅ **DONE** v0.2.4 (branch `security/mail-header-injection`). Also fixed a Message-ID re-injection the test caught. **TODO: redeploy belfort site against v0.2.4.** |
| **G2** | **Password hashing** — Argon2id (+ verify), constant-time | amalgame-crypto | new S4.x | High (foundation) | ✅ **DONE** crypto v0.4.0 (PR #4): `Password.Hash/Verify`, scrypt backend (OpenSSL 3.0; Argon2id-swappable), self-describing string; +6 tests. NB: scrypt not Argon2id (no OpenSSL 3.2/libargon2 here) |
| **G3** | **Constant-time compare** helper exposed | amalgame-crypto | S4.2 | Med | ✅ **DONE** crypto v0.4.0 (PR #4): `Bytes.ConstantTimeEquals`; +3 tests |
| **G4** | **WebSocket Origin allow-list** (anti-CSWSH) — `Ws.ServeWithOrigins` / config | amalgame-net-http (WS) + web | new S5.x | High | ✅ **DONE** net-http v0.18.0 (PR #19): `Ws.ServeWithOrigins`, off-list Origin→403; 6 smoke tests |
| **G5** | **Static symlink-escape guard** — `realpath`/`lstat` containment, not lexical-only | web `static.am` | S3.5 | High | ✅ **DONE** web v0.29.0 (PR #39): realpath(3) containment; test `static_test.am` symlink→403 |
| **G6** | **Static dotfile + source/config block** — deny `.env`,`.git/*`,`.am`,`.lock`,`.htpasswd`… (keep `.well-known`) | web `static.am` | new S3.x | High | ✅ **DONE** web v0.29.0 (PR #39): dotfiles/.am/.lock→403, .well-known→200 |
| **G7** | **Context-aware escaping** — JS/attribute/URL escapers + `{{x\|js}}`/`{{x\|attr}}`/`{{x\|url}}` filters | web `template.am` | S3.2 | High | ✅ **DONE** web v0.29.0 (PR #39): js/attr/url/raw filters, unknown→HTML fail-safe; +11 tests |
| **G8** | **Request smuggling defense** — reject CL+TE both present; strict request-line parse | amalgame-net-http | S2.1/S2.2/S2.9 | High | ✅ **DONE** net-http v0.18.0 (PR #19): `amalgame_h1_framing_ok` rejects CL+TE/TE/dup-CL/non-numeric + empty request-line; 7 smoke tests |
| **G9** | **Host header allow-list** middleware | web | S3.10 | Med | ✅ **DONE** web v0.30.0 (PR #40): `WithHostGuard`/`HostGuard`, forged Host→400; 11 tests |
| **G10** | **Connection-level limits** — max concurrent conns | amalgame-net-http | S2.8 | Med | ✅ **DONE** net-http v0.20.0 (PR #21): `Http1.SetMaxConnections` across all MT accept loops; smoke tests |
| **G11** | **Auth brute-force throttle / lockout** (auth-aware) | web (login_guard) | S4.12 | Med | ✅ **DONE** web v0.31.0 (PR #41): `LoginGuard` per-key lockout (429+Retry-After); 11 tests |
| **G12** | **Upload validation** — per-file size, extension/MIME allow-list, filename sanitize | net-http (multipart) | S3.8 | Med | ✅ **DONE** net-http v0.19.0 (PR #20): `SafeFilename`/`Extension`/`SaveToDir`; 8 tests |
| **G13** | **Log field sanitization** — strip/encode CR/LF in logged values (anti log-forging) | web `log_config.am` + net-http | new S7.x | Med | ✅ **DONE** web v0.30.0 (PR #40): `WebApp.logSafe` strips CR/LF/tab from Host/path/method in access log |
| **G14** | **CSP nonce support** — per-request nonce so strict CSP works without `unsafe-inline` | web `security_headers.am` + web_app | S5.6 | Med | ✅ **DONE** web v0.32.0 (PR #42): `WithCspNonce`/`ctx.CspNonce`; 4 tests |
| **G15** | **Open-redirect guard** — opt-in `RedirectLocal()` (same-origin only) | net-http | S3.6 | Med | ✅ **DONE** net-http v0.19.0 (PR #20): `HttpResponse.RedirectLocal`; 4 tests |
| **G16** | **SSRF guard** — opt-in `HttpClient.GetGuarded` refuses RFC1918/loopback/link-local/metadata | amalgame-net-http | S3.7 | Med | ✅ **DONE** net-http v0.19.0 (PR #20): `GetGuarded`/`HostIsPublic`; 6 tests |

**Already planned (track, don't duplicate):** JWT RS256/JWKS (MD9), OAuth2 PKCE/OIDC (MD10),
sliding-window rate-limit (S6.2), trusted-proxy XFF (S6.3), form-body CSRF (MC3), OCSP (MB10),
dns-01 (MB11). These have a documented limitation/plan; the audit records their state.

**Build order (severity × blast-radius × containment):** G1 (live, contained) → G5/G6 (static,
contained) → G7 (escaping) → G8 (smuggling) → G4 (WS origin) → G2/G3 (crypto foundation) →
G9–G14 → G15/G16 (already-roadmapped). Each fix lands as a feature branch + PR on its repo,
re-tested per §6, logged in §8.

---

## 8. Bugs found & fixed

| # | Component | Symptom (repro) | Issue | PR | Fixed in version | Re-test |
|---|---|---|---|---|---|---|
| G1 | amalgame-net-smtp | Email header injection: CRLF in From/To/Reply-To/Subject → inject Bcc/headers/body (incl. a Message-ID re-injection path); no DATA dot-stuffing | — | (branch `security/mail-header-injection`) | v0.2.4 | ✅ `examples/mail_security.am` 7/7 + full suite green |

---

## 9. Pollen multi-region demo video

Scenario (needs the 5-node multi-region layout):
- Exec nodes across DE/ES/UK/US, manager UI showing geolocated nodes.
- A visual workflow (flowchart with arrows) executing live across continents, execution
  path highlighted in the manager.
- **Live HA demo**: kill a node on camera → workflow reroutes, manager shows the failover.
- Bonus: a *useful* workflow (fan-out / data aggregation) rather than a toy.

GTM artefact (Pollen Manager = the commercial spearhead). Can be coordinated with the
weather-station open-source showcase if filming together.

---

## 10. Timeline (indicative)

- **D0** — order VPS, DNS records, base hardening (SSH key-only, firewall, upgrades).
- **D1** — Phase A: AMC + Mosaic (fastest to validate) + start ACME Phase 3 migration.
- **D2–D3** — Phase B: distributed Pollen + correction loops.
- **D4** — Pollen video shoot + report write-up.

---

## Appendix A — raw commands & logs

_(fill as tests run)_

## Appendix B — configs

_(sanitized configs per host)_
