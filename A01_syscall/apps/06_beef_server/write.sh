make clean
make deploy
cp ./fs.img /Volumes/OS/
cp ~/x86_64_jisaku_os_4_samples/fs/kernel.bin /Volumes/OS/
cp ~/x86_64_jisaku_os_4_samples/fs/poiboot.conf /Volumes/OS/
mkdir -p /Volumes/OS/EFI/BOOT
cp ~/x86_64_jisaku_os_4_samples/fs/EFI/BOOT/BOOTX64.EFI /Volumes/OS/EFI/BOOT/BOOTX64.EFI
diskutil unmount /Volumes/OS
echo "done"
