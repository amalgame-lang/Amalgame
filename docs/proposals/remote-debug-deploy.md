# Proposal: Remote debug + remote deploy for `amc`

**Status:** design (2026-06-16).
**Builds on:** [`dap-strategy.md`](dap-strategy.md) — the `amc dap` MI
bridge (Phases 1-7, shipped, default since v0.8.46). This doc is the
"Remote track" that picks up after Phase 8 (lldb).
**Goal:** debug an Amalgame program running on a *remote* machine from
VS Code on a *local* machine — including when the two run different
operating systems — and ship only the delta needed to make that run
possible.

## Why this doc

The bridge gives a rich local debug session: pretty-printed runtime
types, frame filtering, `#line`-driven `.am`↔`.c` mapping. Everything it
does is **independent of where the binary runs** — it's pure DAP↔MI
translation. What's missing to debug a remote process is three things,
in three independent layers:

1. **Transport** — get the DAP/MI stream to a debugger sitting next to
   the remote binary.
2. **Deploy** — put the right binary + `.am` sources + a debug server on
   the target, cheaply enough to iterate.
3. **Cross-OS** — make (1) and (2) work when local and remote disagree
   on OS/arch.

These layers are **orthogonal but version-coupled**: the only hard
contract between them is "the binary being debugged and the sources used
for line mapping are the same build." The rest composes freely.

## Non-goals

- Re-implementing the gdb remote protocol. gdbserver already does it
  better than we ever should; we orchestrate it, we don't replace it.
- A bespoke network DAP protocol. We forward DAP/MI over an existing
  tunnel; no new wire format.
- VPN / corporate-network policy. We document what passes a typical
  corp firewall (see §5) but we don't ship a tunnel daemon.

---

## Layer B — Transport (debug)

The standard, lowest-code path: **gdbserver on the target + a gdb that
speaks `target remote`.** We reuse 100% of the bridge's translation
layer; the only new code is *where gdb attaches*.

There are two placements of gdb, selected by whether local and remote
are homogeneous. This is the crux of the cross-OS question (§3).

### Placement 1 — gdb local, gdbserver remote (`--connect`)

```
VS Code ──DAP/stdio──▶ amc dap (local) ──MI──▶ gdb (local)
                                                  │ gdb remote protocol
                                                  ▼  (over tunnel)
                                             gdbserver (remote) ──▶ binary
```

`amc dap --connect host:port` keeps the bridge exactly as today, but
instead of issuing `-exec-run` to a freshly forked local gdb, it issues
`-target-select remote host:port` (gdb MI). gdb owns the remote
protocol; the bridge never sees it. Estimated change: ~50 LoC in
`RunBridge()` ([src/dap.am:462](../../src/dap.am#L462)) — a new branch
that swaps the run command and skips the local `fork+exec` of the
inferior.

**Constraint:** the *local* gdb must understand the target's
architecture + object format. Same-OS/arch → trivial. Cross-OS → needs
`gdb-multiarch` / a cross-gdb locally (see §3, Placement 2 is usually
better there).

### Placement 2 — gdb + amc dap both remote (`--listen`)

```
VS Code ──DAP──▶ (tunnel) ──▶ amc dap (remote) ──MI──▶ gdb (remote) ──▶ binary
```

`amc dap --listen :PORT` runs the bridge **on the target**, next to a
native gdb of the target's OS. Only the DAP stream crosses the network.
The local machine runs nothing but VS Code. This resembles "Approche C"
that `dap-strategy.md` set aside for local use — but for **cross-OS** it
is the right tool, because gdb and the binary always share a platform so
there is zero cross-arch gdb to install. Requires a native `amc` on the
target (we already cross-compile for cortex-m7, so a native Linux/Win
`amc` on a server is not a new capability).

### MCU (already cross today)

`arm-none-eabi-gdb` + OpenOCD is already wired
([src/dap.am:70](../../src/dap.am#L70), `--target`/`--openocd`). It is a
cross-gdb running locally — i.e. Placement 1, cross, already proven.
Nothing new here; listed so the matrix is complete.

### Selection matrix

| Local vs remote | Placement | gdb runs | Network carries |
|---|---|---|---|
| Same OS + arch | 1 (`--connect`) | local | gdb remote protocol |
| Different OS/arch | **2 (`--listen`)** | remote | DAP stream |
| MCU on-chip | 1 (cross, today) | local (`arm-none-eabi-gdb`) | OpenOCD (local) |

---

## Layer 3 — Deploy (delta + per-OS hooks)

Layer B assumes the target already has: the `-g` binary, the `.am`
sources (for `#line` mapping), and a running debug server. Putting them
there cheaply enough to iterate is `amc deploy`.

### Delta push

`amc deploy <target>` computes a manifest `{path, sha256, mtime}` on
both ends and pushes only what differs. The runtime already has crypto
for the hashes and `Amalgame.Net` for transport; `rsync`/`scp` can be
shelled out when present, but the delta is computable without them so we
don't take a hard external dep. Re-pushing everything per iteration
would defeat the whole point of remote debug, so the delta is not
optional polish — it's what makes Layer B usable day to day.

### Per-OS hooks

A `deploy.toml` declares pre/post commands **per target platform**.
`amc deploy` detects the remote OS at handshake and runs the matching
block. This is also exactly how Layer B's debug server gets launched —
the command differs by OS, which is the case that justifies declaring it
rather than hard-coding glue:

```toml
[deploy.linux]
post = "gdbserver :4711 ./myapp"
[deploy.windows]
post = "gdbserver.exe :4711 .\\myapp.exe"
```

Declare commands once; let `amc deploy` choose. Do **not** hand-maintain
parallel `.sh`/`.ps1` trees — one source of truth, dispatched by
detected OS.

### Build-on-target vs cross-compile

A deploy-time decision, recorded per target:

- **Same-arch server:** push the `-g` binary + sources; no toolchain on
  the target.
- **Build on target:** push sources, build remotely (needs gcc + amc
  there). Simpler line mapping (paths are native to the build host).
- **MCU / different arch:** cross-compile locally, push binary +
  sources; mapping handled as below.

---

## Layer 3 ↔ Cross-OS — the `sourceFileMap` contract

The one cross-OS gotcha that actually bites. `#line` directives embed
the **path at build time**. Compile on a Linux remote →
`/home/.../src/foo.am`; open in VS Code on Windows → `C:\dev\...\foo.am`.
gdb resolves the right line, but the editor opens the wrong (or no) file
→ breakpoints show "unverified."

Standard DAP fix: a `sourceFileMap` in `launch.json` mapping the remote
prefix to the local checkout. `amc deploy` is the right component to
generate it — it knows both ends at push time. This is emitted
automatically by `amc deploy --debug`.

---

## The version contract (the only hard coupling)

gdb pointing at line N of a `.am` that has drifted from the deployed
binary is silently wrong — the worst failure mode. So:

- Layer 3 embeds a **build hash** in the deployed manifest.
- `amc dap --connect`/`--listen` reads it at handshake and **refuses to
  arm breakpoints** if it doesn't match the local sources' hash.

Everything else between the layers is free composition; this is the one
invariant both sides must honor.

---

## §5 — Getting out of a corporate network

Validate this **before** writing code — it can veto the whole approach.

- **Outbound SSH (22) is usually closed** on corp networks. Don't design
  around it.
- **Outbound 443 via the company proxy almost always passes.** Put the
  debug stream on 443, not a custom port — *from the corp side*.

The relay plays **two distinct roles**. Decouple them — conflating them
is what created the false "needs Mosaic" assumption.

### Role 1 — terminate TLS + auth: lives in `amc`, not Mosaic

`amc dap --listen` encrypts and authenticates **itself** via the
`amalgame-tls` stack. It never delegates TLS to a Mosaic relay:

```
amc dap --listen :PORT --tls --mtls --ca client-ca.pem
```

mTLS + token + localhost bind all live in the `amc` process — one fewer
dependency, end-to-end encryption (any intermediary sees only opaque
ciphertext), and consistent with dogfooding the language. **No Mosaic
required.**

### Role 2 — the rendezvous to cross the corp firewall

Independent of Mosaic. Three tiers, least-infra → most-controlled:

- **Tier 0 — no relay of yours (`cloudflared` / `ngrok`).** The server
  dials **out** to a third-party edge; you connect to that edge. You
  host nothing. Cost: your stream transits a third party — which is
  exactly why TLS terminates in `amc` (Role 1), so the third party only
  ever relays ciphertext.
- **Tier 1 — your own minimal relay (any box, not Mosaic).** A small
  TCP/TLS forwarder on *any* VPS you own (need not be the target). The
  target dials out to it; you connect to it; it splices the two. ~100
  LoC over `Amalgame.Net` + `amalgame-tls` — one rendezvous point reused
  for *all* your servers, unrelated to Mosaic.
- **Tier 2 — direct, no relay.** Server has a public IP and an inbound
  port *you* control (home/personal box — not a locked-down corp
  server). `amc dap --listen` binds directly, mTLS. Only when the
  server-side firewall is yours.

### Special case — Mosaic already on the server's 443

*Only* if the target already runs Mosaic on 443 and you want to share
that port: SNI-multiplex via net-http v0.12.0 `HttpsH1Server.AddSni`
(`debug.example.com` → relay, `www.example.com` → site). This is a
convenience for that situation, **not** the central mechanism. Otherwise
ignore it.

### Ports

443 is the **corp-friendly default the corp machine dials out to**,
never a hard-coded bind on the server. The rendezvous endpoint
(host + port) is configuration (`--relay-port`, default 443); if you move
off 443, verify it passes the corp proxy (most allow only 80/443
outbound via `CONNECT`).

### Alternative shape

**VS Code Remote Tunnels (`code tunnel`)** — purpose-built: all traffic
over Microsoft infra on 443, no inbound port, no relay of yours. Natural
fit if the deploy also runs the editor server remotely.

⚠️ **Policy, not just ports.** A debug tunnel out of the corp network
may breach IT policy even when technically possible. Quick feasibility
probe from the corp machine before investing:
`curl -v https://<server>:443` and check whether `ngrok`/`cloudflared`
are tolerated.

---

## §6 — Security model (merge gate, not optional)

The threat is blunt and must be stated first:

> **gdbserver is an *unauthenticated RCE by design*.** The gdb remote
> protocol has no auth and no encryption; anyone who opens the socket can
> read/write the inferior's memory and **call arbitrary functions**
> (`call system("...")`). An exposed gdbserver — or `amc dap --listen`
> — socket is a root shell on the box.

So rule #1 is **never expose the raw socket**, not "add a token." The
token is defense-in-depth *above* the tunnel, never a substitute for it.

Defense layers, most-important first:

1. **Bind localhost-only, never `0.0.0.0`.** gdbserver and `--listen`
   bind `127.0.0.1`; the only way in from outside is *through* the
   tunnel. Non-negotiable; neutralizes most of the risk on its own.
2. **TLS mandatory, mTLS preferred.** The stream runs inside
   `amalgame-tls` (terminated in `amc` itself — §5 Role 1). Under
   **mTLS** the server only serves a client bearing *your* cert — that
   is the real authentication: a key, not a replayable password. mTLS is
   **required** for `--listen`; plain TLS + token is the floor.
3. **Single-shot / ephemeral.** gdbserver `--once`; `--listen` accepts
   **one** connection then closes. Socket + token are per-session, stood
   up by the deploy post-hook and torn down at session end. No
   long-lived daemon, no third-party reconnect.
4. **Token at handshake (defense-in-depth).** A pre-shared secret in the
   DAP `initialize`; `--listen` rejects and closes on absent/wrong. Cheap
   cover for "tunnel compromised but key intact."
5. **Dedicated unprivileged user.** The debug server runs as e.g.
   `amc-debug`, **never root** — bounds the blast radius of the inherent
   RCE. A restricting namespace/cgroup is better still.
6. **Rate-limit + audit log.** A connection attempt with no valid token →
   log + backoff, recorded under `docs/audit/` per the abuse-test
   checklist.

Recommended stack:

```
corp ──mTLS (your cert)──▶ rendezvous (§5) ──tunnel──▶ 127.0.0.1:PORT
                                                         amc dap --listen
                                                         (amalgame-tls mTLS,
                                                          user amc-debug, --once,
                                                          token at handshake)
                                                              │
                                                         gdb (localhost) ──▶ binary
```

Auth = **mTLS (who)** + **token (defense-in-depth)**; containment =
**localhost + dedicated user + --once**. No classic user/password — a
debugger is not a web service, and a replayable password over an
RCE-grade channel is the worst of both worlds.

**Merge gate:** `--listen` must be incapable of binding a non-loopback
interface without explicit mTLS configured. Ship the abuse test
(connect without cert / without token / from a non-loopback bind) green
before the feature lands.

---

## Command surface (target state)

One command chains all three layers; VS Code sees an ordinary debug:

```
amc deploy --debug <target>
  = delta-push (Layer 3)
  → post-hook launches gdbserver|amc dap (Layer 3, per-OS)
  → amc dap --connect host:port   (homogeneous, Placement 1)
    OR amc dap --listen on remote  (cross-OS, Placement 2)
  → sourceFileMap written into launch.json (Layer 3)
  → F5
```

New flags:

- `amc dap --connect host:port` — attach local gdb to remote gdbserver.
- `amc dap --listen :port --tls --mtls --ca <ca.pem>` — run the bridge on
  the target over mTLS (§6); refuses a non-loopback bind without mTLS.
- `amc deploy <target>` — delta push.
- `amc deploy --debug <target> [--relay-port N]` — push + launch debug
  server + emit `launch.json`. `--relay-port` defaults to 443 (§5).

---

## Phasing (continues `dap-strategy.md` Phases 1-8)

9. **`--connect` MVP.** Branch in `RunBridge()` that does
   `-target-select remote`. Manual gdbserver on the target, manual
   `scp`. Proves Placement 1 end-to-end. (~50 LoC + a test.)
10. **`amc deploy` delta.** Manifest + hash + push. No hooks yet.
11. **Per-OS hooks + `--debug` glue.** `deploy.toml`, remote-OS detect,
    launch gdbserver as post-hook, write `launch.json`.
12. **`--listen` (cross-OS, Placement 2).** Bridge on a socket instead
    of stdio; run on target; forward DAP over the tunnel.
13. **`sourceFileMap` autogen + version-contract enforcement.** The two
    cross-cutting guarantees, once both transports exist.

Phase 9 is shippable alone (homogeneous remote debug, manual deploy) and
already delivers value. 10-11 make it ergonomic. 12-13 unlock cross-OS.

## Open questions

- Rendezvous default (§5 Role 2): start on `cloudflared`/`ngrok`
  (Tier 0, zero infra) or invest early in the homegrown Tier-1 relay on
  the TLS+Net stack? (Time-to-first-session vs dogfooding.) Security
  posture is settled either way since TLS terminates in `amc` (§6).
- Build-on-target vs cross-compile default per target class — config, or
  inferred from arch mismatch?
- mTLS cert provisioning UX: does `amc deploy` mint/rotate the client CA
  + per-session token, or expect a pre-provisioned CA? (Bootstrapping the
  first trust anchor without a relay yet in place.)
