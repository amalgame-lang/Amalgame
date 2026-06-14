# Héberger ses sites avec Mosaic

Le chapitre 11 montrait comment écrire un serveur Mosaic. Ce chapitre est
l'autre moitié : le mettre sur un vrai serveur — **plusieurs sites sur une
machine**, **certificats Let's Encrypt**, **tourner en service**, et
**renouveler les certs sans surveillance**.

Tout ici vient d'un déploiement réel : un seul binaire Mosaic servant huit
noms d'hôte (sites statiques, formulaire de contact, app de sondage en
direct) sur une machine, sans nginx devant. Il a remplacé un montage
Node/Express.

> Prérequis : le chapitre 11 (le framework), les packages
> `amalgame-net-http` et `amalgame-tls`, et un hôte dont l'IP publique
> correspond aux enregistrements DNS `A` de vos domaines.

---

## Un binaire, plusieurs sites (dispatch par Host)

`amalgame-web` n'a pas de couche vhost native : un `WebApp` = un site. Pour
servir plusieurs domaines depuis un process, on tient une table
`host → WebApp` et on choisit selon l'en-tête `Host` de la requête.

```amalgame
public class SiteServer {
    public Apps: Map<string, WebApp>
    public SiteServer() { this.Apps = new Map<string, WebApp>() }

    public void Register(List<string> hosts, WebApp app) {
        var i = 0
        while (i < hosts.Count()) { this.Apps.Set(hosts.Get(i), app); i = i + 1 }
    }

    // Retire le port, cherche l'hôte ; null ⇒ hôte inconnu (404).
    public WebApp Pick(string host) {
        var h: string = host
        let colon: int = String_IndexOf(h, ":")
        if (colon >= 0) { h = String_Substring(h, 0, colon) }
        if (this.Apps.Has(h)) { return this.Apps.Get(h) }
        return null
    }
}
```

On construit chaque `WebApp` une fois (routes + un mount `Static` pour son
`public/`), on l'enregistre sous ses noms d'hôte (`example.com` **et**
`www.example.com`), et la boucle d'accept dispatche sur `Host`. Deux sites
statiques + un troisième avec une route de contact `POST`, c'est trois
appels `Register`.

---

## TLS en production : Let's Encrypt + SNI

Pour de vrais navigateurs, il faut de vrais certificats. `amalgame-tls` les
émet et renouvelle depuis Let's Encrypt avec `AcmeNative.EnsureCert`, et
`HttpsH1Server` sert un certificat différent par domaine via **SNI**.

```amalgame
// Au démarrage, AVANT le bind de :443 : obtenir/renouveler un cert par
// domaine. Challenge HTTP-01 — EnsureCert prend :80 transitoirement, donc
// rien d'autre ne doit le tenir à cet instant. NeedsRenewal permet de
// sauter un cert valide (zéro appel ACME inutile, zéro risque de rate-limit).
for d in domains {
    let certPath: string = certDir + "/" + d + "/fullchain.pem"
    if (AcmeNative.NeedsRenewal(certPath, 30)) {
        AcmeNative.EnsureCert(d, email, certDir, AcmeNative.LeProd())
    }
}
```

Puis un seul listener HTTPS porte tous les domaines, un cert chacun :

```amalgame
let srv = HttpsH1Server.Listen(443, defaultCert, defaultKey, 0)
for d in extraDomains {                       // les autres, par SNI
    HttpsH1Server.AddSni(srv, d, certOf(d), keyOf(d))
}
```

Un second thread sert un listener `:80` en clair qui redirige (301) vers
`https://`, pour qu'aucun visiteur n'atterrisse en clair :

```amalgame
let redirect = conn => {
    let host: string = H1Conn.Header(conn, "host")
    let path: string = H1Conn.Path(conn)
    HttpResponse.New().Redirect("https://" + host + path, true).WriteToH1Conn(conn)
    return 0
}
Threading.ThreadSpawn((_a: int) => { Http1.Serve(80, redirect); return 0 }, 0)
```

> **Staging d'abord.** `AcmeNative.LeStaging()` émet des certs non fiables
> mais sans rate-limit — pour valider tout le flux ACME + SNI, puis passer
> à `LeProd()`. La prod Let's Encrypt a de vrais rate-limits ; le garde-fou
> `NeedsRenewal` évite de ré-émettre des certs encore valides.

---

## Tourner en service (systemd)

Un `./server` en avant-plan meurt avec votre session SSH et ne revient pas
après un reboot. Reliez-le à systemd pour qu'il démarre au boot, redémarre
en cas de crash, et logue durablement. Les ports < 1024 et le challenge
ACME demandent root.

> **Les déploiements config-driven sautent cette étape.** Si vous lancez
> le binaire `mosaic serve` depuis un `mosaic.toml` (pas un `./server`
> bâti à la main), `mosaic service install` génère et enregistre l'unit
> ci-dessous pour vous en une commande — voir [Mosaic → Configuration →
> Tourner en service](mosaic/03-configuration.fr.md#4-tourner-en-service).
> L'unit manuelle ici est pour un `./server` custom que vous avez compilé
> vous-même.

```ini
# /etc/systemd/system/mosaic-sites.service
[Unit]
Description=Serveur web multi-sites Mosaic
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

Passez les secrets (clé reCAPTCHA, token admin) en lignes `Environment=`
ou via un fichier de config gitignoré lu au démarrage — jamais en dur dans
le binaire, jamais commités.

---

## Renouvellement des certs (là où tout le monde se trompe)

Deux faits décident du design :

1. **`HttpsH1Server` charge chaque certificat une seule fois**, au
   `Listen`/`AddSni`, dans un `SSL_CTX` en mémoire. Il **ne relit pas** le
   fichier par handshake — donc un fichier renouvelé sur disque n'est
   **pas** pris en compte par le process en marche. Un cert renouvelé ne
   prend effet qu'après un **redémarrage**.
2. **`EnsureCert` a besoin de `:80`** pour le challenge HTTP-01, mais le
   serveur en marche tient déjà `:80` (le listener de redirect). On ne peut
   donc **pas** renouveler *à chaud* en servant.

Les deux mènent à la même réponse : **renouveler au démarrage, et
redémarrer pour renouveler.** Le démarrage exécute `EnsureCert` avant le
bind de `:80`, donc le port est libre ; et un process frais charge le cert
frais. Reste *quand* redémarrer — on veut le faire à faible trafic, et
seulement quand un cert approche vraiment de l'expiration. Un timer
systemd fait exactement ça :

```bash
# /usr/local/bin/mosaic-cert-check.sh — restart seulement si un cert < 30 j
#!/bin/bash
need=0
for c in /var/mosaic/sites/certs/*/fullchain.pem; do
  [ -f "$c" ] || continue
  openssl x509 -checkend 2592000 -noout -in "$c" >/dev/null 2>&1 || need=1
done
[ "$need" = 1 ] && systemctl restart mosaic-sites.service
```

```ini
# /etc/systemd/system/mosaic-cert-check.timer — chaque jour à 04:00
[Unit]
Description=Vérif des certs Mosaic, restart si proche expiration
[Timer]
OnCalendar=*-*-* 04:00:00
Persistent=true
[Install]
WantedBy=timers.target
```

(plus un `mosaic-cert-check.service` `Type=oneshot` d'une ligne qui lance
le script). `Persistent=true` rattrape après une coupure. Résultat : une
vérif chaque nuit, un restart seulement les quelques fois par an où un cert
renouvelle vraiment, toujours à 04:00.

> N'allez pas mettre un thread de renouvellement *dans le process* (24 h).
> Il ne peut pas prendre `:80` en servant, ne peut de toute façon pas
> recharger le cert à chaud sans redémarrage, et n'a pas d'horloge pour
> viser « faible trafic » — systemd est le bon outil.

---

## Deux pièges de production

- **Traitement multi-thread.** Mosaic sert chaque connexion sur son propre
  thread. Un handler qui fait du read-modify-write sur un fichier partagé
  (le compteur de votes d'un sondage, par exemple) va courir et perdre des
  mises à jour sous charge concurrente. amc n'a pas de champ statique
  mutable, donc protégez l'état partagé par un mutex `Threading` capturé
  par les closures de routes, ou passez par un channel. Un site écrit pour
  un `WebApp.Serve` mono-thread n'est **pas** automatiquement sûr une fois
  qu'il est un site parmi d'autres dans un hôte multi-thread.

- **Reverse-proxy vs intégration.** Si une autre app sert déjà du HTTP sur
  un port local, vous *pouvez* la relayer depuis le handler de dispatch
  avec `HttpClient`. Mais pour une app à état, intégrer ses routes
  directement dans le binaire multi-sites (compiler ses fichiers `.am`
  avec les vôtres — `amc a.am b.am c.am -o server`) est plus propre : un
  process, un cert, pas de port en plus — au prix du câblage sûr de son
  état (voir ci-dessus).

---

## Récap

| Besoin | Mécanisme |
|--------|-----------|
| Plusieurs sites, un process | table `host → WebApp` + `Pick(Host)` |
| Cert par domaine | `HttpsH1Server.Listen` + `AddSni` |
| Émettre / renouveler les certs | `AcmeNative.EnsureCert` au démarrage (HTTP-01 sur `:80`) |
| Forcer HTTPS | thread `:80` qui redirige (301) vers `https://` |
| Rester en ligne | service systemd, `Restart=always` |
| Renouveler sans surveiller | timer nocturne → restart seulement si un cert `< 30 jours` |
| État mutable partagé | mutex `Threading` (hôte multi-thread) |

C'est exactement ainsi que `amalgame.me` et ses sites voisins sont hébergés
— la vitrine du framework tourne sur Mosaic.
