#!/usr/bin/env bash
# One-shot MCU toolchain setup for amc embedded targets (docs/proposals/amc-embedded.md).
# Installs the cross toolchain + debugger + flasher + emulator, and builds
# libopencm3 for STM32F7 (needed by the amalgame-mcu-nucleo-f767zi board).
#
# Run it yourself (it uses sudo for the apt step):  tools/mcu-setup.sh
# amc's --target preflight points here when a tool is missing.
set -euo pipefail

# --- apt packages (Debian/Ubuntu) ---------------------------------------
#   gcc-arm-none-eabi + binutils : cross-compile (.elf/.bin/.hex)
#   openocd                      : flash + on-chip GDB server (SWD/JTAG)
#   gdb-multiarch                : the debugger (source-level on .am)
#   stlink-tools                 : st-info/st-flash + ST-LINK udev rules
#   qemu-system-arm              : run/debug without hardware (cortex-m3)
PKGS=(gcc-arm-none-eabi binutils-arm-none-eabi openocd gdb-multiarch stlink-tools qemu-system-arm git make)
missing=()
for p in "${PKGS[@]}"; do dpkg -s "$p" >/dev/null 2>&1 || missing+=("$p"); done

if [ "${#missing[@]}" -gt 0 ]; then
  if command -v apt-get >/dev/null 2>&1; then
    echo ">> installing: ${missing[*]}"
    sudo apt-get update -qq
    sudo apt-get install -y "${missing[@]}"
  else
    echo "!! non-apt system. Install these with your package manager: ${missing[*]}"
    echo "   (arm-none-eabi gcc+binutils, openocd, gdb-multiarch, qemu-system-arm)"
    exit 1
  fi
else
  echo ">> apt toolchain already present (${PKGS[*]})"
fi

# --- udev rules: ST-LINK access without sudo ----------------------------
# On a fresh system the ST-LINK USB device is root-only, so flashing/debugging
# would need sudo. openocd ships udev rules; install them + add the user to the
# probe groups. (No-op on non-Linux / if already present.)
if command -v apt-get >/dev/null 2>&1 && [ -d /etc/udev/rules.d ]; then
  RULES_SRC=""
  for c in /usr/share/openocd/contrib/60-openocd.rules /lib/udev/rules.d/*stlink* /usr/lib/udev/rules.d/*stlink*; do
    [ -f "$c" ] && RULES_SRC="$c" && break
  done
  if [ -n "$RULES_SRC" ] && [ ! -f /etc/udev/rules.d/60-openocd.rules ]; then
    echo ">> installing ST-LINK udev rules ($RULES_SRC)"
    sudo cp "$RULES_SRC" /etc/udev/rules.d/60-openocd.rules
    sudo udevadm control --reload-rules && sudo udevadm trigger || true
  fi
  for g in plugdev dialout; do getent group "$g" >/dev/null 2>&1 && sudo usermod -aG "$g" "$USER" || true; done
  echo "   (if the probe is still root-only: unplug/replug the board, or log out/in for group changes)"
fi

# --- libopencm3 (STM32F7) -----------------------------------------------
LIBOPENCM3="${LIBOPENCM3:-$HOME/libopencm3}"
if [ -f "$LIBOPENCM3/lib/libopencm3_stm32f7.a" ]; then
  echo ">> libopencm3 already built at $LIBOPENCM3"
else
  echo ">> cloning + building libopencm3 (stm32/f7) into $LIBOPENCM3"
  [ -d "$LIBOPENCM3/.git" ] || git clone --depth 1 https://github.com/libopencm3/libopencm3 "$LIBOPENCM3"
  make -C "$LIBOPENCM3" TARGETS=stm32/f7
fi

cat <<EOF

✓ MCU toolchain ready.
  For board builds against libopencm3, export:
    export AMC_EMBED_INC="$LIBOPENCM3/include"
    export AMC_EMBED_LIB="$LIBOPENCM3/lib"
  Then:
    amc build --target=cortex-m7 --board=<amalgame-mcu-pkg> --flash app.am -o fw
    amc dap   --target=cortex-m7 --openocd=board/st_nucleo_f7.cfg     # on-chip debug
EOF
