# ACME Phase-3 production cutover — runbook

**Status:** prepared 2026-06-05 (unattended). **Execution requires a
human** — it touches production (live sites, DNS, real Let's Encrypt
certs) and is intentionally NOT automated.

**Deadline:** the live cert hits its day-30 renewal threshold around
**2026-07-27** (hard expiry ~2026-08-27). Cut over before the threshold
so the first real Mosaic-native renewal happens with margin. See
`[[roadmap_acme_autorenew_timer]]`.

## What this is

Move the production sites (belfort, musicall) from the current topology —
**Node/pm2 holds :80 + :443**, Mosaic runs behind it — to
**Mosaic terminating its own public HTTPS** via the native ACME client
(`AcmeNative.EnsureCert` + `WebApp.ServeHttps`/`MosaicServer` + the
embedded auto-renew timer). Node/pm2 stays installed as the rollback net.

Everything needed is already shipped:
- `amalgame-tls` ≥ v0.3.3 — native RFC 8555 ACME (http-01), `CertDaysRemaining`/`NeedsRenewal`, auto-renew thread.
- `amalgame-net-http` ≥ v0.21.0 — HTTPS-H1 + SNI + **IPv6 dual-stack** + keep-alive.
- `amalgame-web` ≥ v0.33.0 — `MosaicServer` Host dispatch + `WithObservability` (`/healthz` for the LB/uptime probe).

## Pre-flight (safe — do these first, no prod impact)

1. **Confirm DNS** A/AAAA records for each domain point at the prod host
   (now that the listener is IPv6 dual-stack, add AAAA if the host has a
   public v6 address — optional).
2. **Reachability:** port 80 must be reachable from the Internet for the
   http-01 challenge. If Node currently owns :80, the cutover step swaps
   it; verify no other process will fight for :80.
3. **Staging dry-run** (NO rate limits, untrusted certs): run the Mosaic
   binary with `ACME_STAGING=1` against the real domains on a spare port
   or a maintenance window. Confirm a staging cert is issued and cached
   under `data/acme/<domain>/`. Only then go to production.
4. **Backup** the current Node/pm2 config and any existing certs.

## Cutover (maintenance window, ~10 min, reversible)

1. `git pull` the Sites (Mosaic) repo on the prod host; `mosaic build
   --mono` (or rebuild the site binary) with the floors above.
2. **Stop Node holding :80/:443** (`pm2 stop <apps>`), freeing the ports.
   Keep pm2 config intact for rollback.
3. Start the Mosaic binary bound to `:80` + `:443` with ACME enabled for
   the production domains (production directory — drop `ACME_STAGING`).
   First real :80 hit solves http-01; cert caches to `data/acme/`.
4. **Verify** (see checklist) before leaving the window.
5. Register the Mosaic binary as a systemd service
   (`amalgame-service` / `mosaic service`) with `Restart=on-failure` so it
   survives reboots; enable it.

## Verification checklist

- [ ] `curl -I https://<domain>` → 200, valid (trusted) cert chain.
- [ ] `openssl s_client -connect <domain>:443 -servername <domain>` →
      correct cert per SNI domain; chain complete.
- [ ] `curl -I http://<domain>` → 301/308 redirect to https.
- [ ] `curl -6 -I https://<domain>` (if AAAA set) → 200 over IPv6.
- [ ] `/healthz` returns 200 (wire it to uptime monitoring).
- [ ] `/metrics` is NOT reachable from the public Internet (private-by-
      default — confirm a public curl gets 404; scrape locally or via a
      bearer token only). See web v0.33.0.
- [ ] The contact form (belfort: reCAPTCHA + SMTP) still works end-to-end.
- [ ] `data/acme/<domain>/` holds the issued cert; `CertDaysRemaining` is
      ~90; the auto-renew thread is running.

## Rollback (if anything fails)

1. Stop the Mosaic binary.
2. `pm2 start <apps>` — Node reclaims :80/:443 with the previous certs.
3. Investigate from logs (`journalctl -u <mosaic-service>`, access log),
   fix, retry in the next window. The day-30 margin (≈ until 2026-07-27)
   means one failed attempt is not an outage emergency.

## Post-cutover (within a few weeks)

- Watch the FIRST real auto-renewal fire (around day 30) and confirm it
  swaps the cert without a restart hiccup.
- Once stable, decommission the Node/pm2 layer for these sites.
- Log the result in `audit-vps-2026-06.md` §4.B (MB-series) + §8.

## Security notes (per the mandatory-security protocol)

- ACME is the **native pure-AM** client — no `certbot`/shell subprocess,
  so no command-injection surface from the domain string.
- http-01 only today: keep :80 reachable. tls-alpn-01 / dns-01 (wildcards)
  are roadmap (§21 items 4.2/4.3) — not required for this cutover.
- Keep `/metrics` private (default) on the public box; expose `/healthz`
  only (it leaks nothing beyond up/down).
