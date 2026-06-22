#!/bin/bash
# esp32s3-flash.sh — build an .am for --target=esp32s3 and flash a real board.
#
# Why the copy dance: the Xtensa toolchain truncates non-ASCII paths, and this
# repo may live under an accented path (…/Développement/…). So we build in a
# plain-ASCII cache dir with the amc installed under ~/.local (whose runtime/
# board assets are also ASCII). Sources stay versioned here in the repo.
#
# Prereqs on PATH: xtensa-esp32s3-elf-gcc and esptool. E.g. from a PlatformIO
# ESP-IDF install:
#   export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$PATH"
#   export PATH="$HOME/.platformio/penv/bin:$PATH"   # or wherever esptool lives
#
# Usage: ./esp32s3-flash.sh [program.am] [/dev/ttyUSB0]
set -e

SRC="${1:-heartbeat.am}"
PORT="${2:-/dev/ttyUSB0}"
AMC="${AMC:-$HOME/.local/bin/amc}"
BUILD="${AMC_ESP32S3_BUILD:-$HOME/.cache/amalgame-esp32s3}"

here="$(cd "$(dirname "$0")" && pwd)"
name="$(basename "$SRC" .am)"

command -v xtensa-esp32s3-elf-gcc >/dev/null || { echo "xtensa-esp32s3-elf-gcc not on PATH"; exit 1; }
command -v esptool >/dev/null || { echo "esptool not on PATH"; exit 1; }
[ -x "$AMC" ] || { echo "amc not found at $AMC (run ./tools/install-local.sh)"; exit 1; }

mkdir -p "$BUILD"
cp "$here/$SRC" "$BUILD/"
cd "$BUILD"
echo "=== build $SRC (target=esp32s3) in $BUILD ==="
"$AMC" build --target=esp32s3 "$SRC" -o "$name"
echo "=== flash $name.bin → $PORT @0x0 ==="
esptool --chip esp32s3 -p "$PORT" -b 460800 write_flash 0x0 "$name.bin"
echo "done. serial console: 115200 8N1 on $PORT"
