#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
source "$SCRIPT_DIR/image-common.sh"

require_boot_bin
prepare_local_disks

echo "writing $BOOT_BIN -> $DISK1_IMG"
dd if="$BOOT_BIN" of="$DISK1_IMG" bs=512 conv=notrunc count=1
