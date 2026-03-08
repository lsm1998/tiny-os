#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE_DIR="${IMAGE_DIR:-$PROJECT_DIR/image}"
BUILD_DIR="$PROJECT_DIR/build"
BOOT_ELF="$BUILD_DIR/source/boot/boot.elf"
QEMU_BIN="${QEMU_BIN:-qemu-system-i386}"
GDB_BIN="${GDB_BIN:-gdb}"
GDB_PORT="${GDB_PORT:-1234}"
GDB_BATCH="${GDB_BATCH:-0}"
QEMU_PID=""
QEMU_LOG="$(mktemp /tmp/diy-x86os-qemu.XXXXXX.log)"

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "error: $1 not found"
        exit 1
    fi
}

cleanup() {
    if [ -n "$QEMU_PID" ] && kill -0 "$QEMU_PID" >/dev/null 2>&1; then
        kill "$QEMU_PID" >/dev/null 2>&1 || true
        wait "$QEMU_PID" 2>/dev/null || true
    fi

    rm -f "$QEMU_LOG"
}

wait_for_gdbstub() {
    local retries=50

    while ! nc -z 127.0.0.1 "$GDB_PORT" >/dev/null 2>&1; do
        if [ -n "$QEMU_PID" ] && ! kill -0 "$QEMU_PID" >/dev/null 2>&1; then
            echo "error: QEMU exited before the gdb stub was ready"
            cat "$QEMU_LOG"
            exit 1
        fi

        retries=$((retries - 1))
        if [ "$retries" -le 0 ]; then
            echo "error: timed out waiting for QEMU gdb stub on port $GDB_PORT"
            cat "$QEMU_LOG"
            exit 1
        fi

        sleep 0.1
    done
}

ensure_port_free() {
    if lsof -nPiTCP:"$GDB_PORT" -sTCP:LISTEN >/dev/null 2>&1; then
        echo "error: TCP port $GDB_PORT is already in use"
        echo "hint: stop the existing QEMU or set GDB_PORT to another port"
        exit 1
    fi
}

trap cleanup EXIT INT TERM

require_cmd make
require_cmd nc
require_cmd lsof
require_cmd "$QEMU_BIN"
require_cmd "$GDB_BIN"

GDB_ARGS=()
if [ "$GDB_BATCH" = "1" ]; then
    GDB_ARGS+=(--batch)
fi

echo "[1/4] Building boot image"
make -C "$PROJECT_DIR" build

echo "[2/4] Writing boot.bin into $IMAGE_DIR/disk1.img"
(
    bash "$SCRIPT_DIR/img-write-osx.sh"
)

if [ ! -f "$BOOT_ELF" ]; then
    echo "error: $BOOT_ELF not found"
    exit 1
fi

ensure_port_free

echo "[3/4] Starting QEMU paused on gdb port $GDB_PORT"
"$QEMU_BIN" \
    -m 128M \
    -gdb "tcp::${GDB_PORT}" \
    -S \
    -display none \
    -monitor none \
    -serial none \
    -drive file="$IMAGE_DIR/disk1.img",index=0,media=disk,format=raw \
    -drive file="$IMAGE_DIR/disk2.img",index=1,media=disk,format=raw \
    >"$QEMU_LOG" 2>&1 &
QEMU_PID=$!

wait_for_gdbstub

echo "[4/4] Launching GDB"
echo "Breakpoint: 0x7c00 (_start)"
echo "Useful commands: si, ni, info registers, x/10i \$pc"
echo "Note: if GDB shows odd real-mode disassembly, compare with $BUILD_DIR/source/boot/boot_dis.txt"

"$GDB_BIN" "${GDB_ARGS[@]}" "$BOOT_ELF" \
    -ex "set architecture i8086" \
    -ex "target remote 127.0.0.1:$GDB_PORT" \
    -ex "set disassemble-next-line on" \
    -ex "display/i \$pc" \
    -ex "break *0x7c00" \
    -ex "continue"
