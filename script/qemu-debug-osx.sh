#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
source "$SCRIPT_DIR/image-common.sh"

bash "$SCRIPT_DIR/img-write-osx.sh"

exec qemu-system-i386 -m 128M -s -S \
    -drive file="$DISK1_IMG",index=0,media=disk,format=raw \
    -drive file="$DISK2_IMG",index=1,media=disk,format=raw \
    -d pcall,page,mmu,cpu_reset,guest_errors,page,trace:ps2_keyboard_set_translation
