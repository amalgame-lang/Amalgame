# Hosting your sites with Mosaic

Chapter 11 showed how to write a Mosaic server. This chapter is the other
half: taking it to a real server — **several sites on one machine**,
**Let's Encrypt certificates**, **running as a service**, and **renewing
certs without babysitting**.

Everything here is drawn from a production deployment: a single Mosaic
binary serving eight hostnames (static sites, a contact form, and a
live-poll app) on one host, with no nginx in front. It replaced a
Node/Express setup.

> Prerequisites: chapter 11 (the framework), the `amalgame-net-http` and
> `amalgame-tls` packages, and a host whose public IP your domains' DNS
> `A` records point to.

---

## One binary, many sites (dispatch by Host)

`amalgame-web` has no built-in virtual-host layer: one `WebApp` is one
site. To serve several domains from one process, keep a `host → WebApp`
map and pick by the request's `Host` header.

```amalgame
public class SiteServer {
    public Apps: Map<string, WebApp>
    public SiteServer() { this.Apps = new Map<string, WebApp>() }

    public void Register(List<string> hosts, WebApp app) {
        var i = 0
        while (i < hosts.Count()) { this.Apps.Set(hosts.Get(i), app); i = i + 1 }
    }

    // Strip the port, look the host up; null ⇒ unknown host (404).
    public WebApp Pick(string host) {
        var h: string = host
        let colon: int = String_IndexOf(h, ":")
        if (colon >= 0) { h = String_Substring(h, 0, colon) }
        if (this.Apps.Has(h)) { return this.Apps.Get(h) }
        return null
    }
}
```

You build each site's `WebApp` once (routes + a `Static` mount for its
`public/`), register it under its hostnames (`example.com` **and**
`www.example.com`), and the accept loop dispatches on `Host`. Two static
sites and a third with a contact `POST` route is just three `Register`
calls.

---

## TLS in production: Let's Encrypt + SNI

For real browsers you need real certificates. `amalgame-tls` issues and
renews them from Let's Encrypt with `AcmeNative.EnsureCert`, and
`HttpsH1Server` serves a different certificate per domain via **SNI**.

```amalgame
// At startup, before binding :443: obtain/renew a cert per domain.
// HTTP-01 challenge — EnsureCert transiently grabs :80, so nothing else
// must hold it at this moment. NeedsRenewal lets you skip a valid cert
// (no needless ACME calls, no rate-limit risk).
for d in domains {
    let certPath: string = certDir + "/" + d + "/fullchain.pem"
    if (AcmeNative.NeedsRenewal(certPath, 30)) {
        AcmeNative.EnsureCert(d, email, certDir, AcmeNative.LeProd())
    }
}
```

Then one HTTPS listener carries every domain, one cert each:

```amalgame
let srv = HttpsH1Server.Listen(443, defaultCert, defaultKey, 0)
for d in extraDomains {                       // the rest, by SNI
    HttpsH1Server.AddSni(srv, d, certOf(d), keyOf(d))
}
```

A second thread serves a plain `:80` listener that 301-redirects to
`https://` so visitors never land on cleartext:

```amalgame
let redirect = conn => {
    let host: string = H1Conn.Header(conn, "host")
    let path: string = H1Conn.Path(conn)
    HttpResponse.New().Redirect("https://" + host + path, true).WriteToH1Conn(conn)
    return 0
}
Threading.ThreadSpawn((_a: int) => { Http1.Serve(80, redirect); return 0 }, 0)
```

> **Staging first.** `AcmeNative.LeStaging()` issues untrusted-but-rate-
> limit-free certs — use it to validate the whole ACME + SNI flow, then
> switch to `LeProd()`. Let's Encrypt production has real rate limits;
> a `NeedsRenewal` gate keeps you from re-issuing valid certs.

---

## Running as a service (systemd)

A foreground `./server` dies with your SSH session and won't come back
after a reboot. Bind it to systemd so it starts on boot, restarts on
crash, and logs somewhere durable. Ports < 1024 and the ACME challenge
need root.

```ini
# /etc/systemd/system/mosaic-sites.service
[Unit]
Description=Mosaic multi-site web server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/var/mosaic/sites
ExecStart=/var/mosaic/sites/server
Restart=always
RestartSec=2
StandardOutput=append:/var/log/mosaic-sites.log
StandardError=append:/var/log/mosaic-sites.log

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now mosaic-sites.service
systemctl status mosaic-sites          # active (running)
```

Pass secrets (a reCAPTCHA key, an admin token) as `Environment=` lines or
a gitignored config file the server reads at startup — never bake them
into the binary or commit them.

---

## Certificate renewal (the part everyone gets wrong)

Two facts decide the design:

1. **`HttpsH1Server` loads each certificate once**, at `Listen`/`AddSni`,
   into an in-memory `SSL_CTX`. It does **not** re-read the file per
   handshake — so a renewed file on disk is **not** picked up by the
   running process. A renewed cert only takes effect after a **restart**.
2. **`EnsureCert` needs `:80`** for the HTTP-01 challenge, but the running
   server already holds `:80` (the redirect listener). So you cannot renew
   *in place* while serving.

Both point to the same answer: **renew at startup, and restart to renew.**
Startup runs `EnsureCert` before the `:80` redirect binds, so the port is
free; and a fresh process loads the fresh cert. The only question is *when*
to restart — you want it at low traffic, and only when a cert is actually
near expiry. A systemd timer does exactly that:

```bash
# /usr/local/bin/mosaic-cert-check.sh — restart only if a cert is < 30 days
#!/bin/bash
need=0
for c in /var/mosaic/sites/certs/*/fullchain.pem; do
  [ -f "$c" ] || continue
  openssl x509 -checkend 2592000 -noout -in "$c" >/dev/null 2>&1 || need=1
done
[ "$need" = 1 ] && systemctl restart mosaic-sites.service
```

```ini
# /etc/systemd/system/mosaic-cert-check.timer — daily at 04:00
[Unit]
Description=Check Mosaic certs, restart to renew if near expiry
[Timer]
OnCalendar=*-*-* 04:00:00
Persistent=true
[Install]
WantedBy=timers.target
```

(plus a one-line `Type=oneshot` `mosaic-cert-check.service` that runs the
script). `Persistent=true` catches up after downtime. The result: a check
every night, a restart only the few times a year a cert actually renews,
always at 04:00.

> Don't reach for a 24 h *in-process* renewal thread. It can't grab `:80`
> while serving, can't hot-reload the cert without a restart anyway, and
> has no clock to target "low traffic" — systemd is the right tool.

---

## Two production gotchas

- **Multi-threaded request handling.** Mosaic serves each connection on
  its own thread. A handler that does read-modify-write on a shared file
  (a poll's vote tally, say) will race and lose updates under concurrent
  load. amc has no mutable static fields, so guard shared state with a
  `Threading` mutex captured by the route closures, or move it behind a
  channel. A site written for a single-threaded `WebApp.Serve` is **not**
  automatically safe once it's one site among many in a multi-threaded
  host.

- **Reverse-proxying vs. integrating.** If a separate app already serves
  HTTP on a local port, you *can* forward to it from the dispatch handler
  with `HttpClient`. But for a stateful app, integrating its routes
  directly into the multi-site binary (compile its `.am` files alongside
  yours — `amc a.am b.am c.am -o server`) is cleaner: one process, one
  cert, no extra port — at the cost of wiring its state safely (see
  above).

---

## Recap

| Concern | Mechanism |
|---------|-----------|
| Many sites, one process | `host → WebApp` map + `Pick(Host)` |
| Per-domain certs | `HttpsH1Server.Listen` + `AddSni` |
| Issue / renew certs | `AcmeNative.EnsureCert` at startup (HTTP-01 on `:80`) |
| Force HTTPS | `:80` thread that 301s to `https://` |
| Stay up | systemd service, `Restart=always` |
| Renew without babysitting | nightly timer → restart only if a cert is `< 30 days` |
| Shared mutable state | `Threading` mutex (multi-threaded host) |

This is precisely how `amalgame.me` and its sibling sites are hosted —
the framework's own showcase runs on Mosaic.
