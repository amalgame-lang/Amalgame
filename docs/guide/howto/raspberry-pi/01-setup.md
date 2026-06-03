# Set up your Pi

You do this once. At the end you'll have `amc` on the Pi, the
hardware interfaces enabled, and the `libgpiod` system library ready
for the `amalgame-hardware-gpio` package.

## 1 · Flash a 64-bit OS

Amalgame ships a native **`linux-arm64`** build. Make sure your Pi
runs a **64-bit** OS — `armv7l`/`armv6l` (32-bit) is not supported.

Use **Raspberry Pi Imager**, pick *Raspberry Pi OS (64-bit)*, and
write it to your SD card. After booting, check:

```sh
uname -m        # must print: aarch64
```

If it prints `armv7l`, reflash with the 64-bit image.

## 2 · Install amc

The one-liner installs the native ARM64 binary:

```sh
sudo apt install -y build-essential libgc-dev libssl-dev curl
curl -fsSL https://raw.githubusercontent.com/amalgame-lang/Amalgame/main/install/install.sh | sh
amc --version
```

`build-essential` gives you the `gcc` that `amc build` calls under the
hood (Amalgame compiles to C, then to a native binary).

## 3 · Enable the hardware interfaces

GPIO works out of the box. **I²C** and **SPI** are off by default —
turn them on with `raspi-config`:

```sh
sudo raspi-config
#   → 3 Interface Options
#       → I5 I2C   → Enable
#       → I4 SPI   → Enable
sudo reboot
```

(Equivalently, add `dtparam=i2c_arm=on` and `dtparam=spi=on` to
`/boot/firmware/config.txt`.) After rebooting you should see the
device nodes:

```sh
ls /dev/i2c-* /dev/spidev*
# /dev/i2c-1  /dev/spidev0.0  /dev/spidev0.1
```

## 4 · Install libgpiod v2 (the one real gotcha)

`amalgame-hardware-gpio` links **libgpiod 2.x**. Check what your
distro ships:

```sh
pkg-config --modversion libgpiod   # you want 2.x
```

| Distro | libgpiod in `apt` | What to do |
|---|---|---|
| Debian Trixie+, Ubuntu 24.10+ | 2.x | `sudo apt install libgpiod-dev` |
| **Raspberry Pi OS / Debian Bookworm, Ubuntu 24.04 LTS** | **1.6 (incompatible)** | build 2.x from source ↓ |

Raspberry Pi OS (Bookworm) only has **1.6 in apt**, and its API is
incompatible with 2.x — so on a Pi today you almost always build 2.x
from source. It takes about a minute:

```sh
sudo apt install -y build-essential autoconf autoconf-archive \
                    automake libtool pkg-config
curl -sSLO https://mirrors.edge.kernel.org/pub/software/libs/libgpiod/libgpiod-2.2.4.tar.gz
tar xzf libgpiod-2.2.4.tar.gz && cd libgpiod-2.2.4
./configure --prefix=/usr/local --enable-tools=yes
make && sudo make install && sudo ldconfig
pkg-config --modversion libgpiod   # now prints 2.2.4
```

> libgpiod 1.x and 2.x have *incompatible* APIs; this package targets
> 2.x only. There is no 1.6 fallback shim.

## 5 · Permissions: the `gpio`/`i2c`/`spi` groups

Reading and writing the device nodes needs either `sudo` or
membership in the right group. On Raspberry Pi OS the default user is
usually already in `gpio`, `i2c`, and `spi`. Confirm with:

```sh
groups        # look for: gpio i2c spi
```

If a group is missing, add yourself and log out/in:

```sh
sudo usermod -aG gpio,i2c,spi "$USER"
```

Otherwise just run your programs with `sudo`.

---

That's the whole setup. Next: **[Your first program](02-first-program.md)**.
