#!/bin/bash

set -e

echo "🗑️  suppréssion des fichiers déjà compilé..."
rm -rf out/**

echo "➕  création du dossier os dans le dossier out..."
mkdir -p out/os

GCC_FLAGS="-m32 -ffreestanding -fno-pic -fno-pie -fno-builtin \
-nostdlib -nostartfiles -O2 -Wall -Wextra"

# Compilation de tous les .c dans drivers/ et libs/
EXTRA_OBJECTS=""

for dir in src/kernel/drivers src/kernel/libs; do
  if [ -d "$dir" ]; then
    for src_file in "$dir"/*.c; do
      [ -f "$src_file" ] || continue
      base=$(basename "$src_file" .c)
      out_file="out/${base}.o"
      echo "⚙️  compilation de $src_file..."
      gcc $GCC_FLAGS -c "$src_file" -o "$out_file"
      EXTRA_OBJECTS="$EXTRA_OBJECTS $out_file"
    done
  fi
done

echo "⚙️  compilation du kernel entry..."
nasm -f elf32 src/bootloader/kernel_entry.asm -o out/kernel_entry.o

echo "⚙️  compilation du kernel main..."
gcc $GCC_FLAGS -c src/kernel/kernel_main.c -o out/kernel_main.o

echo "🪢  linker le code..."
ld -m elf_i386 -T src/linker.ld -o out/kernel_main.elf \
  out/kernel_entry.o out/kernel_main.o $EXTRA_OBJECTS

echo "🐦‍🔥 transformer le .elf en .bin..."
objcopy -O binary out/kernel_main.elf out/kernel_main.bin

KSECTORS=$(python3 -c "import os; print((os.path.getsize('out/kernel_main.bin') + 511)//512)")
echo "📦 Secteurs kernel: $KSECTORS"

echo "⚙️  compilation du bootloader..."
nasm -f bin -D KERNEL_SECTORS=$KSECTORS \
  src/bootloader/bootloader.asm -o out/bootloader.bin

echo "➕  création de l'image disque..."
cat out/bootloader.bin out/kernel_main.bin > out/os/nexOS.bin

echo "🚀 Lancement de qemu..."
qemu-system-i386 -drive format=raw,file=out/os/nexOS.bin \
  -boot c -no-reboot -no-shutdown -monitor stdio