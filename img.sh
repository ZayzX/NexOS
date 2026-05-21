#!/bin/bash

set -e

BOOT=out/bootloader.bin
KERNEL=out/kernel_main.bin
IMG=NexOS.img

echo "🧹 suppression ancienne image..."
rm -f $IMG

echo "📦 création image disque vide..."
dd if=/dev/zero of=out/os/$IMG bs=512 count=65536 status=none

echo "⚙️ écriture bootloader (sector 0)..."
dd if=$BOOT of=out/os/$IMG bs=512 conv=notrunc status=none

echo "⚙️ écriture kernel (sector 1)..."
dd if=$KERNEL of=out/os/$IMG bs=512 seek=1 conv=notrunc status=none

echo "🚀 image créée : $IMG"

echo "▶ lancement QEMU..."
qemu-system-i386 -drive format=raw,file=out/os/$IMG