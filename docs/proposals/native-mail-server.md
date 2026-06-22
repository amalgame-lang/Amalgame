# Native mail server — SMTP receive + IMAP + POP3

**Status:** proposal / not started. Drafted **2026-06-22**.
**Goal:** turn `family.neitsab.fr` into a *receiving* mail host so a
family member can "add an account in their mail client" and read/send
mail over the **same single account** they already use for files
(WebDAV), calendars (CalDAV) and contacts (CardDAV) — all backed by
`amalgame-auth`.

> This expands item **#4** of [`beyond-http.md`](beyond-http.md#L184)
> (previously "SMTP relay + IMAP/POP3 server — outbound client only").
> The outbound half shipped as `amalgame-net-smtp` v0.2.4 (TLS SMTP
> *client* + `Mail` builder). Everything below is the **inbound** half,
> still entirely unbuilt.

## TL;DR — the two independent problems

Building a mail server splits into **two problems that do not depend on
each other**, and the second is the real wall:

1. **The code** — SMTP/IMAP/POP3 servers + a message store. Tractable.
   Amalgame already has every building block (TLS, async, auth, crypto,
   filesystem, and a working RFC 5322/MIME *assembler* in net-smtp).
2. **Deliverability** — MX, PTR/rDNS, SPF, DKIM, DMARC, outbound :25,
   IP reputation. **This decides whether self-hosting mail on the IONOS
   IP is viable at all, and it is independent of native-vs-third-party.**
   No amount of clean Amalgame code fixes a blocked port 25 or a missing
   PTR record.

**Therefore the build proceeds entirely on loopback / a local port —
no internet exposure — and the public :25 listener goes live only at
the very end, behind a security-audit gate (user-set hard rule, see
below).** The non-exposing deliverability prep (PTR request, DNS
drafting, blocklist check) runs in parallel; if it shows the IONOS IP
can't reliably *send*, the native build still fully covers *receiving*
+ IMAP, with *sending* via a smarthost (the existing net-smtp client →
relay) — which only changes the outbound path, not the architecture.

## Package layout

Decided naming (net-* family, consistent with `beyond-http.md`):

| Package | Role | Status |
|---|---|---|
| `amalgame-net-smtp` | **extend**: add server side (receive :25 + submission :587) next to the existing client | exists @ v0.2.4 (client only) |
| `amalgame-net-imap` | IMAP4rev1 server (:143 STARTTLS, :993 implicit TLS) | new |
| `amalgame-net-pop3` | POP3 server (:110 STARTTLS, :995 implicit TLS) — least critical | new |

Plus shared concerns that should **not** be triplicated across the
three protocol servers (open decision — naming is yours):

| Concern | Candidate home | Rationale |
|---|---|---|
| RFC 5322 / MIME **parsing** | `amalgame-formats-mime` (new) | mirrors `amalgame-formats-xml`, built for CalDAV. net-smtp already *assembles* MIME; parsing is the inbound dual and a clean reusable unit. |
| **Maildir** store (read by IMAP+POP3, written by SMTP) | small `amalgame-mail-store` lib, or fold into net-smtp | POP3 and IMAP both read the store without needing the SMTP server; a standalone store avoids a net-imap→net-smtp dependency. |
| Auth | reuse `amalgame-auth` kernel (see below) | the unified-account payoff. |

**Recommendation:** `amalgame-formats-mime` (parser) + `amalgame-mail-store`
(Maildir) as the two shared foundations, then `net-imap` / `net-pop3` /
`net-smtp`-server on top. This keeps each protocol package thin and lets
the MIME parser be reused outside mail (e.g. parsing `.eml`, multipart
HTTP uploads).

## Auth — the unified-account payoff, and the one catch

The whole point is a single family account. `amalgame-auth` is already
the shared identity for WebDAV/CalDAV/CardDAV. Its kernel is
**protocol-neutral and directly reusable**:

- `Credentials.Verify(name, password)` — scrypt check, no HTTP.
- `LoginGuard` — failure counting / lockout windows (the
  `RecordFailure` / `IsLocked` core).

**Catch:** the *wrappers* are HTTP-shaped — `BasicAuth.Authenticate()`
takes `HttpRequest`, `BasicAuth.Reject()` / `LoginGuard.Check()` return
`HttpResponse`. Mail auth (SMTP `AUTH PLAIN`/`LOGIN`, IMAP `LOGIN`/
`AUTH`, POP3 `USER`/`PASS`) can't use those. So a small refactor in
`amalgame-auth`: extract a protocol-neutral result (e.g.
`AuthOutcome { ok, lockedSeconds, user }`) that both the HTTP wrappers
and the mail servers build on. This dovetails with the already-noted
"amalgame-auth extraction" work for the family NAS.

## Building blocks (all already in the workspace)

| Need | Package | Notes |
|---|---|---|
| TLS / STARTTLS | `amalgame-tls` | implicit TLS (465/993/995) + STARTTLS upgrade (587/143/110). |
| Concurrency | `amalgame-async` | one connection ⇒ one task; long-lived IMAP sessions. |
| Listener / accept loop | `amalgame-net-stream` / net-http server core | model for the C-runtime accept loop + thin AM façade (same pattern as net-smtp's header-only core). |
| Maildir I/O | `amalgame-io-filesystem` | atomic `tmp`→`new`→`cur` moves. |
| DKIM signing, SPF | `amalgame-crypto` | RSA/Ed25519 signing for outbound DKIM. |
| MIME assemble (reuse) | `amalgame-net-smtp` | `Mail` builder + dot-stuffing + header-injection hygiene already done — the parser is its inverse. |

Architectural note: net-smtp's pattern is a **header-only C runtime
core** (`Amalgame_Net_Smtp.h` does sockets/TLS) with a thin `.am`
façade calling it via `@c`. The three servers should follow the same
split: accept loop + line protocol in the runtime, ergonomic API in AM.

## Minimum viable surface (sketch)

```amalgame
import Amalgame.Net.Smtp        // server side, extended

let srv = new SmtpServer()
    .Bind(25)                    // MX inbound
    .Submission(587)             // authenticated send, STARTTLS
    .WithTls(tlsConfig)          // shared family.neitsab.fr cert
    .WithAuth(familyCredentials) // same amalgame-auth account
    .Store(maildir)              // deposit accepted mail
srv.OnRcpt(addr => maildir.HasMailbox(addr) ? Rcpt.Accept : Rcpt.Reject)
srv.Serve()
```

```amalgame
import Amalgame.Net.Imap
new ImapServer()
    .Bind(993).WithTls(tlsConfig)
    .WithAuth(familyCredentials)
    .Store(maildir)              // SELECT/FETCH/STORE over the same Maildir
    .Serve()
```

v0.1 scope per protocol: **SMTP** receive + submission, no anti-spam,
no queue retry beyond basic; **IMAP4rev1** SELECT/FETCH/STORE/SEARCH
(enough for Thunderbird/Apple Mail/K-9); **POP3** USER/PASS/LIST/RETR/
DELE. Realistic estimate: several weeks, IMAP being the largest surface.

## Security posture — port 25 stays closed until clean + audited

**Hard rule (user-set):** the public :25 listener is **not exposed to
the internet until the code is clean AND has passed a security audit.**
A receiving mail server on :25 is an unauthenticated, internet-facing
attack surface (open-relay abuse, MIME/parser exploits, resource
exhaustion); it is the *last* thing that goes live, behind an explicit
audit gate.

Consequence for development: **everything is built and tested over
loopback / a non-public port** (local Maildir, local IMAP/POP3 clients,
synthetic SMTP sessions). No internet exposure is required to build,
unit-test, or even integration-test the full stack. Exposure is a
deployment decision taken once, at the end.

## Deliverability — Phase 0 (the non-exposing parts can run now)

These can proceed in parallel with development because **none of them
opens a port**:

- [ ] **PTR / rDNS** — request `212.227.95.103 → family.neitsab.fr` in
      the IONOS panel (manual, user action; lead time varies). Gmail/
      Outlook reject mail from IPs without a matching PTR.
- [ ] **Blocklists** — is the IP already on Spamhaus / others? (passive
      lookup, no exposure.)
- [ ] **DNS records to draft** (publish later): `MX → family.neitsab.fr`;
      `SPF` (`v=spf1 a mx -all`); `DKIM` selector + public key (private
      key signs via amalgame-crypto); `DMARC` (`p=quarantine` to start).

Deferred until the audit gate (these *do* involve :25):

- [ ] **Outbound :25 reachability** — can the VPS reach a remote MX on
      :25? (Hosting IPs often block egress; if so → send via smarthost.)
- [ ] **Open the inbound :25 firewall / listener.**

If, at the gate, outbound :25 is open, PTR is granted, and the IP is
clean → native self-hosting is viable. Otherwise: **receive natively,
send via smarthost** (existing net-smtp client → a relay) — still a
fully useful "family mail" without fighting deliverability.

## Phasing

All development happens with **no public port open** (loopback only).

0. **Deliverability prep (non-exposing)** — PTR request, blocklist
   check, draft DNS records. Runs in parallel; no code, no exposure.
1. **`amalgame-formats-mime`** — RFC 5322 + MIME parser (inverse of the
   net-smtp assembler). Unit-testable in isolation, no network.
2. **`amalgame-mail-store`** — Maildir read/write, mailbox enumeration.
3. **`amalgame-net-smtp` server** — receive + submission → store; auth
   via the extracted `amalgame-auth` kernel. Tested on a local port.
4. **`amalgame-net-imap`** — IMAP4rev1 over the store; the protocol that
   actually lets a mail client connect. Largest surface. Local testing.
5. **`amalgame-net-pop3`** — POP3, smaller, optional/legacy.
6. **DKIM signing + outbound path.**
7. **🔒 Security audit gate** — full review of the parser, the :25 relay
   logic (no open relay), auth, and resource limits. *Nothing exposed
   before this passes.*
8. **Go live** — open :25 / submission, publish DNS, request outbound
   :25 unblock, wire into the `mosaic-family` service so mail rides the
   same hostname/cert/account as WebDAV/CalDAV/CardDAV.

## Priority / honest take

`beyond-http.md` rates this **LOW** for the general stack ("every team
needing mail uses Sendgrid/SES; self-hosted SMTP is a hostile
environment"). For the **general Amalgame ecosystem** that still holds.
For **this specific goal** (one unified family account spanning files +
calendar + contacts + mail) it's the natural capstone — but its success
is gated by *deliverability infrastructure*, not by the quality of the
Amalgame code. Hence Phase 0 before code.
