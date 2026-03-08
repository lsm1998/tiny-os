#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE_DIR="${IMAGE_DIR:-$PROJECT_DIR/image}"
DISK1_IMG="${DISK1_IMG:-$IMAGE_DIR/disk1.img}"
DISK2_IMG="${DISK2_IMG:-$IMAGE_DIR/disk2.img}"
BOOT_BIN="${BOOT_BIN:-$IMAGE_DIR/boot.bin}"
DISK1_SIZE_MB="${DISK1_SIZE_MB:-16}"
DISK2_SIZE_MB="${DISK2_SIZE_MB:-32}"

ensure_image_dir() {
    mkdir -p "$IMAGE_DIR"
}

create_raw_disk_if_missing() {
    local path="$1"
    local size_mb="$2"

    if [ -f "$path" ]; then
        return
    fi

    dd if=/dev/zero of="$path" bs=1048576 count="$size_mb" >/dev/null 2>&1
}

prepare_local_disks() {
    ensure_image_dir
    create_raw_disk_if_missing "$DISK1_IMG" "$DISK1_SIZE_MB"
    create_raw_disk_if_missing "$DISK2_IMG" "$DISK2_SIZE_MB"
}

require_boot_bin() {
    if [ ! -f "$BOOT_BIN" ]; then
        echo "error: $BOOT_BIN not found"
        echo "hint: run 'make build' first"
        exit 1
    fi
}
