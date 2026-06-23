#!/bin/bash
# esp32s3-flash-xip.sh — build an .am as a flash-XIP image and flash a real board
# (B0c). Two stages, no ESP-IDF bootloader, no FreeRTOS, no ESPHome:
#   - payload : the amc program, .text+.literal -> IROM (flash, I-cache),
#               .rodata -> DROM (flash, D-cache), .bss -> DRAM SRAM. Raw blob @0x80000.
#   - stub    : a tiny RAM-loaded image @0x0 that configures the I-cache, maps the
#               MMU (IROM/DROM -> the payload's flash pages), enables the cache,
#               and jumps to the payload. The ROM UART (115200) stays usable.
#
# Limit (current board layout): .text <= 64 KB and .rodata <= 64 KB (one MMU page
# each: IROM entry 0 -> flash page 8, DROM entry 1 -> flash page 9). Bigger images
# need more mapped pages — to be parameterised when the runtime grows (WiFi).
#
# Prereqs on PATH: xtensa-esp32s3-elf-gcc, esptool. amc installed at ~/.local.
# Usage: ./esp32s3-flash-xip.sh [program.am] [/dev/ttyUSB0]
set -e

SRC="${1:-heartbeat.am}"
PORT="${2:-/dev/ttyUSB0}"
AMC="${AMC:-$HOME/.local/bin/amc}"
BUILD="${AMC_ESP32S3_BUILD:-$HOME/.cache/amalgame-esp32s3}"
RT="$(dirname "$AMC")/../share/amalgame/runtime/embedded"
BD="$RT/boards/esp32s3-flash"
CC=xtensa-esp32s3-elf-gcc
OBJCOPY=xtensa-esp32s3-elf-objcopy
name="$(basename "$SRC" .am)"

command -v "$CC" >/dev/null || { echo "$CC not on PATH"; exit 1; }
command -v esptool >/dev/null || { echo "esptool not on PATH"; exit 1; }
[ -x "$AMC" ] || { echo "amc not found at $AMC (run ./tools/install-local.sh)"; exit 1; }
here="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$BUILD"; cp "$here/$SRC" "$BUILD/"; cd "$BUILD"

echo "=== amc: $SRC -> C ==="
"$AMC" build --target=esp32s3 "$SRC" -o "$name" >/dev/null    # emits $name.c (ramload .bin ignored)

echo "=== payload: $name.c -> flash-XIP blob (.text=IROM, .rodata+.data=DROM, vectors=IRAM) ==="
# Board sources folded in: crt0 (sets SP/PS, copies .data + the IRAM vector table)
# + startup, vectors.S + interrupts.c (B0b-2: the vector table runs from internal
# SRAM — crt0 copies it there and VECBASE points at its instruction-bus alias, so
# window/level-1 exceptions don't fault on flash), heap.c (B0b-3; dropped if unused).
#
# Compiled per-TU, not in one shot, because interrupts.c needs
# -mtext-section-literals (so amc_isr_level1's 32-bit constants are pooled INLINE in
# its .iram.text block, reachable by l32r from the copied IRAM vectors) while
# vectors.S must NOT have that flag (it would inject literals into the fixed-offset
# .org vector table → "attempt to move .org backwards"). board.ld KEEPs the table.
CF="-mlongcalls -Os -ffreestanding -ffunction-sections -fdata-sections -I$RT"
"$CC" $CF -c "$BD/crt0.S"                          -o "$name-crt0.o"
"$CC" $CF -c "$BD/vectors.S"                       -o "$name-vectors.o"   # no text-section-literals
"$CC" $CF -mtext-section-literals -c "$BD/interrupts.c" -o "$name-interrupts.o"
"$CC" $CF -c "$BD/startup.c"                       -o "$name-startup.o"
"$CC" $CF -c "$BD/heap.c"                          -o "$name-heap.o"
"$CC" $CF -c "$name.c"                             -o "$name-app.o"
"$CC" -mlongcalls -nostartfiles -Wl,--gc-sections -T "$BD/board.ld" \
  "$name-crt0.o" "$name-vectors.o" "$name-interrupts.o" "$name-startup.o" "$name-heap.o" "$name-app.o" \
  -o "$name-payload.elf"
"$OBJCOPY" -O binary "$name-payload.elf" "$name-payload.bin"
# guard the multi-page layout limit (8 IROM pages = 512 KB; 8 DROM pages = 512 KB)
tsz=$(xtensa-esp32s3-elf-size -A "$name-payload.elf" | awk '/^\.text/{print $2}')
echo "  .text=${tsz}B (must be <= 524288 for the 8-page IROM mapping)"

echo "=== stub: cache/MMU bring-up -> RAM-load image @0x0 ==="
"$CC" -mlongcalls -Os -ffreestanding -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections \
  -T "$BD/stub.ld" "$BD/stub.c" -o "$name-stub.elf"
esptool --chip esp32s3 elf2image --flash_mode dio --flash_freq 80m --flash_size 4MB -o "$name-stub.bin" "$name-stub.elf" >/dev/null

echo "=== flash stub@0x0 + payload@0x80000 -> $PORT ==="
esptool --chip esp32s3 -p "$PORT" -b 460800 write_flash 0x0 "$name-stub.bin" 0x80000 "$name-payload.bin"
echo "done. flash-XIP. serial console: 115200 8N1 on $PORT"
