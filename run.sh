# Shell To run bbnOS

make clean

# For bootsect
make                            # For now we only have bootsect
sudo mount -o loop floppy_boot.img mntfloppy/
sudo cp boot.bin mntfloppy/boot.com
sudo umount mntfloppy
qemu-system-i386 -fda FLOPPY.img -fdb floppy_boot.img