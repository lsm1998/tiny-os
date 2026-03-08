#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE_DIR="${IMAGE_DIR:-$PROJECT_DIR/image}"
BUILD_DIR="$PROJECT_DIR/build"
BOOT_ELF="$BUILD_DIR/src/boot/boot.elf"
QEMU_BIN="${QEMU_BIN:-qemu-system-i386}"
GDB_PORT="${GDB_PORT:-1234}"
PID_FILE="${PID_FILE:-$PROJECT_DIR/.vscode/qemu-debug-gui.pid}"
QEMU_LOG="${QEMU_LOG:-$PROJECT_DIR/.vscode/qemu-debug-gui.log}"
COMMAND="${1:-prepare}"

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "error: $1 not found"
        exit 1
    fi
}

process_comm() {
    ps -p "$1" -o comm= 2>/dev/null | tr -d '[:space:]'
}

read_pid() {
    if [ -f "$PID_FILE" ]; then
        tr -d '[:space:]' <"$PID_FILE"
    fi
}

is_managed_qemu_running() {
    local pid

    pid="$(read_pid)"
    if [ -z "$pid" ]; then
        return 1
    fi

    if ! kill -0 "$pid" >/dev/null 2>&1; then
        return 1
    fi

    process_comm "$pid" | grep -q "qemu-system-i386"
}

stop_managed_qemu() {
    local pid

    pid="$(read_pid)"
    if [ -z "$pid" ]; then
        rm -f "$PID_FILE"
        return 0
    fi

    if is_managed_qemu_running; then
        kill "$pid" >/dev/null 2>&1 || true
        for _ in $(seq 1 50); do
            if ! kill -0 "$pid" >/dev/null 2>&1; then
                break
            fi
            sleep 0.1
        done
        if kill -0 "$pid" >/dev/null 2>&1; then
            kill -9 "$pid" >/dev/null 2>&1 || true
        fi
    fi

    rm -f "$PID_FILE"
}

ensure_port_free() {
    if lsof -nPiTCP:"$GDB_PORT" -sTCP:LISTEN >/dev/null 2>&1; then
        echo "error: TCP port $GDB_PORT is already in use"
        echo "hint: run 'bash script/vscode-debug-osx-gui.sh stop' or choose another GDB_PORT"
        exit 1
    fi
}

stop_qemu_on_port_if_needed() {
    local pids
    local pid
    local comm

    pids="$(lsof -tiTCP:"$GDB_PORT" -sTCP:LISTEN 2>/dev/null || true)"
    if [ -z "$pids" ]; then
        return 0
    fi

    for pid in $pids; do
        comm="$(process_comm "$pid")"
        if [ "$comm" = "qemu-system-i386" ]; then
            echo "stale QEMU detected on port $GDB_PORT, stopping PID $pid"
            kill "$pid" >/dev/null 2>&1 || true
            for _ in $(seq 1 50); do
                if ! kill -0 "$pid" >/dev/null 2>&1; then
                    break
                fi
                sleep 0.1
            done
            if kill -0 "$pid" >/dev/null 2>&1; then
                kill -9 "$pid" >/dev/null 2>&1 || true
            fi
        else
            echo "error: TCP port $GDB_PORT is already in use by PID $pid ($comm)"
            echo "hint: stop that process or choose another GDB_PORT"
            exit 1
        fi
    done

    rm -f "$PID_FILE"
}

wait_for_gdbstub() {
    local retries=50
    local pid

    while ! nc -z 127.0.0.1 "$GDB_PORT" >/dev/null 2>&1; do
        pid="$(read_pid)"
        if [ -n "$pid" ] && ! kill -0 "$pid" >/dev/null 2>&1; then
            echo "error: QEMU exited before the gdb stub was ready"
            if [ -f "$QEMU_LOG" ]; then
                cat "$QEMU_LOG"
            fi
            exit 1
        fi

        retries=$((retries - 1))
        if [ "$retries" -le 0 ]; then
            echo "error: timed out waiting for QEMU gdb stub on port $GDB_PORT"
            if [ -f "$QEMU_LOG" ]; then
                cat "$QEMU_LOG"
            fi
            exit 1
        fi

        sleep 0.1
    done
}

prepare() {
    require_cmd make
    require_cmd nc
    require_cmd lsof
    require_cmd "$QEMU_BIN"

    mkdir -p "$PROJECT_DIR/.vscode"
    stop_managed_qemu
    stop_qemu_on_port_if_needed

    echo "[1/4] Building boot image"
    make -C "$PROJECT_DIR" build

    echo "[2/4] Writing boot/loader into $IMAGE_DIR/disk1.img"
    bash "$SCRIPT_DIR/img-write-osx.sh"

    if [ ! -f "$BOOT_ELF" ]; then
        echo "error: $BOOT_ELF not found"
        exit 1
    fi

    ensure_port_free
    rm -f "$QEMU_LOG"

    echo "[3/4] Starting QEMU with GUI on gdb port $GDB_PORT"
    "$QEMU_BIN" \
        -m 128M \
        -gdb "tcp::${GDB_PORT}" \
        -S \
        -monitor none \
        -serial none \
        -drive file="$IMAGE_DIR/disk1.img",index=0,media=disk,format=raw \
        -drive file="$IMAGE_DIR/disk2.img",index=1,media=disk,format=raw \
        </dev/null >"$QEMU_LOG" 2>&1 &
    echo "$!" >"$PID_FILE"

    wait_for_gdbstub

    echo "[4/4] QEMU GUI gdb stub is ready on 127.0.0.1:$GDB_PORT"
    echo "PID file: $PID_FILE"

    # Keep the preLaunchTask alive so VSCode does not reap the QEMU child.
    wait "$(read_pid)" || true
}

stop() {
    stop_managed_qemu
    stop_qemu_on_port_if_needed
    echo "QEMU GUI debug session stopped"
}

case "$COMMAND" in
    prepare)
        prepare
        ;;
    stop)
        stop
        ;;
    *)
        echo "usage: $0 {prepare|stop}"
        exit 1
        ;;
esac
