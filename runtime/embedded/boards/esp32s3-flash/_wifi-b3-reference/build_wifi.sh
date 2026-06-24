#!/bin/bash
set -e
export PATH="$HOME/.platformio/packages/toolchain-xtensa-esp-elf/bin:$HOME/.platformio/penv/bin:$PATH"
RT="$HOME/.local/share/amalgame/runtime/embedded"; BD="$RT/boards/esp32s3-flash"; REF="$BD/_wifi-b2-reference"; B1="$BD/_wifi-b1-reference"
IDF=/home/neitsab/.platformio/packages/framework-espidf
BLOB=$IDF/components/esp_wifi/lib/esp32s3; PHYLIB=$IDF/components/esp_phy/lib/esp32s3
ROMLD=$IDF/components/esp_rom/esp32s3/ld
CF="-mlongcalls -Os -ffreestanding -ffunction-sections -fdata-sections -I$RT"
gcc(){ xtensa-esp32s3-elf-gcc "$@"; }
gcc $CF -c "$BD/crt0.S" -o crt0.o
gcc $CF -c probe_startup.c -o probe_startup.o
gcc $CF -c "$BD/vectors.S" -o vectors.o
gcc $CF -mtext-section-literals -c "$BD/interrupts.c" -o interrupts.o
gcc $CF -c "$BD/heap.c" -o heap.o
gcc $CF -c "$REF/osi.c" -o osi.o
gcc $CF -c "$REF/sched.c" -o sched.o
gcc $CF -c "$REF/sched_boot.S" -o sched_boot.o
gcc $CF -c "$REF/prims.c" -o prims.o
gcc $CF -c "$REF/timers.c" -o timers.o
gcc $CF -c "$REF/linkstubs.c" -o linkstubs.o
gcc $CF -c "$B1/phy_adapter.c" -o phy_adapter.o
gcc $CF -c wlog.c -o wlog.o
gcc $CF -c wifi_drive.c -o wifi_drive.o
# phy_init_data.o already built (B1)
OURS="crt0.o probe_startup.o vectors.o interrupts.o heap.o osi.o sched.o sched_boot.o prims.o timers.o linkstubs.o phy_adapter.o phy_init_data.o wlog.o wifi_drive.o"
xtensa-esp32s3-elf-gcc -mlongcalls -nostartfiles -Wl,--gc-sections -T "$BD/board.ld" \
  -L"$ROMLD" -T esp32s3.rom.ld -T esp32s3.rom.libc.ld -T esp32s3.rom.libgcc.ld -T esp32s3.rom.newlib.ld -T esp32s3.rom.version.ld \
  -Wl,--start-group $OURS "$BLOB/libnet80211.a" "$BLOB/libpp.a" "$BLOB/libcore.a" "$PHYLIB/libphy.a" -lgcc -Wl,--end-group \
  -o wifi.elf 2>&1 | grep -viE "l32r target section before" | head -40
echo "link rc=${PIPESTATUS[0]}"
