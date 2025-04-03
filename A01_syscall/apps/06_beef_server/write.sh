set -xue

# rm -rf /Volumes/OS/*
make clean
make deploy
ls -lh test

cp ../../../fs/fs.img /Volumes/KX/
cp ~/x86_64_jisaku_os_4_samples/A01_syscall/kernel.bin /Volumes/KX/
cp ~/Downloads/poiboot_20180422/poiboot.conf /Volumes/KX/poiboot.conf
mkdir -p /Volumes/KX/EFI/BOOT
cp ~/x86_64_jisaku_os_2_samples/012_pass_rsdp/poiboot.efi /Volumes/KX/EFI/BOOT/BOOTX64.EFI
diskutil unmount /Volumes/KX
echo "done"
