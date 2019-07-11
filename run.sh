# Shell To run bbnOS

make clean

# For bootsect
make                            # For now we only have bootsect
sudo mount -o loop deps/floppy_boot.img mntfloppy/
sudo cp output/os.bin mntfloppy/boot.com
sudo umount mntfloppy
qemu-system-i386 -fda deps/FLOPPY.img -fdb deps/floppy_boot.img