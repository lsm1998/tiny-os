#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
source "$SCRIPT_DIR/image-common.sh"

require_boot_bin
require_loader_bin
prepare_local_disks

echo "writing $BOOT_BIN -> $DISK1_IMG"
dd if="$BOOT_BIN" of="$DISK1_IMG" bs=512 conv=notrunc count=1 status=none

echo "writing $LOADER_BIN -> $DISK1_IMG (sector 1)"
dd if="$LOADER_BIN" of="$DISK1_IMG" bs=512 conv=notrunc seek=1 status=none
