# Préparer le Pi

À faire une seule fois. À la fin vous aurez `amc` sur le Pi, les
interfaces matérielles activées, et la bibliothèque système `libgpiod`
prête pour le package `amalgame-hardware-gpio`.

## 1 · Flasher un OS 64 bits

Amalgame fournit un binaire natif **`linux-arm64`**. Assurez-vous que
votre Pi tourne sous un OS **64 bits** — le 32 bits
(`armv7l`/`armv6l`) n'est pas supporté.

Avec **Raspberry Pi Imager**, choisissez *Raspberry Pi OS (64-bit)* et
écrivez-le sur la carte SD. Après le boot, vérifiez :

```sh
uname -m        # doit afficher : aarch64
```

S'il affiche `armv7l`, reflashez avec l'image 64 bits.

## 2 · Installer amc

La commande unique installe le binaire ARM64 natif :

```sh
sudo apt install -y build-essential libgc-dev libssl-dev curl
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
amc --version
```

`build-essential` fournit le `gcc` qu'`amc build` appelle en coulisses
(Amalgame compile vers du C, puis vers un binaire natif).

## 3 · Activer les interfaces matérielles

Le GPIO marche d'emblée. **I²C** et **SPI** sont désactivés par
défaut — activez-les avec `raspi-config` :

```sh
sudo raspi-config
#   → 3 Interface Options
#       → I5 I2C   → Enable
#       → I4 SPI   → Enable
sudo reboot
```

(De façon équivalente, ajoutez `dtparam=i2c_arm=on` et
`dtparam=spi=on` dans `/boot/firmware/config.txt`.) Après le redémarrage
vous devriez voir les nœuds périphériques :

```sh
ls /dev/i2c-* /dev/spidev*
# /dev/i2c-1  /dev/spidev0.0  /dev/spidev0.1
```

## 4 · Installer libgpiod v2 (le seul vrai piège)

`amalgame-hardware-gpio` se lie à **libgpiod 2.x**. Vérifiez ce que
votre distribution fournit :

```sh
pkg-config --modversion libgpiod   # il faut du 2.x
```

| Distribution | libgpiod dans `apt` | Que faire |
|---|---|---|
| Debian Trixie+, Ubuntu 24.10+ | 2.x | `sudo apt install libgpiod-dev` |
| **Raspberry Pi OS / Debian Bookworm, Ubuntu 24.04 LTS** | **1.6 (incompatible)** | compiler 2.x depuis les sources ↓ |

Raspberry Pi OS (Bookworm) n'a que la **1.6 dans apt**, dont l'API est
incompatible avec la 2.x — sur un Pi aujourd'hui vous compilez donc
presque toujours la 2.x depuis les sources. Ça prend environ une
minute :

```sh
sudo apt install -y build-essential autoconf autoconf-archive \
                    automake libtool pkg-config
curl -sSLO https://mirrors.edge.kernel.org/pub/software/libs/libgpiod/libgpiod-2.2.4.tar.gz
tar xzf libgpiod-2.2.4.tar.gz && cd libgpiod-2.2.4
./configure --prefix=/usr/local --enable-tools=yes
make && sudo make install && sudo ldconfig
pkg-config --modversion libgpiod   # affiche maintenant 2.2.4
```

> libgpiod 1.x et 2.x ont des API *incompatibles* ; ce package vise la
> 2.x uniquement. Il n'y a pas de couche de compatibilité 1.6.

## 5 · Permissions : les groupes `gpio`/`i2c`/`spi`

Lire et écrire les nœuds périphériques demande soit `sudo`, soit
l'appartenance au bon groupe. Sur Raspberry Pi OS, l'utilisateur par
défaut est généralement déjà dans `gpio`, `i2c` et `spi`. Vérifiez :

```sh
groups        # cherchez : gpio i2c spi
```

S'il manque un groupe, ajoutez-vous et déconnectez/reconnectez-vous :

```sh
sudo usermod -aG gpio,i2c,spi "$USER"
```

Sinon, lancez simplement vos programmes avec `sudo`.

---

Voilà toute la préparation. Ensuite :
**[Votre premier programme](02-first-program.md)**.
