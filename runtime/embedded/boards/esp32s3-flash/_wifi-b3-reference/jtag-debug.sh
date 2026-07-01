#!/bin/bash
# Run AFTER connecting the ESP32-S3 NATIVE USB port (appears as 303a:1001).
set -e
OCD="$HOME/.platformio/packages/tool-openocd/bin/openocd"
GDB="$HOME/.platformio/packages/tool-xtensa-esp-elf-gdb/bin/xtensa-esp32s3-elf-gdb"
cd "$HOME/.cache/amalgame-esp32s3"
echo "[jtag] starting OpenOCD..."; "$OCD" -f esp32s3-builtin.cfg >/tmp/ocd.log 2>&1 &
OCDPID=$!; sleep 2
grep -qiE "Listening on port 3333|Info : \[esp32s3" /tmp/ocd.log && echo "[jtag] OpenOCD up" || { echo "[jtag] OpenOCD failed:"; tail -15 /tmp/ocd.log; kill $OCDPID 2>/dev/null; exit 1; }
# GDB: connect, break at the panic entry, continue, backtrace + regs when it hits
"$GDB" wifi.elf \
  -ex "target extended-remote :3333" \
  -ex "monitor reset halt" \
  -ex "hb _amc_panic" \
  -ex "hb _amc_user_dispatch" \
  -ex "continue"
kill $OCDPID 2>/dev/null
