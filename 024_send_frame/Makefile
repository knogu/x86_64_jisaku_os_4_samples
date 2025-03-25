TARGET = kernel.bin
CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -Iinclude
LDFLAGS = -Map kernel.map -s -x -T kernel.ld
OBJS = main.o syscall.o proc.o iv.o fbcon.o fb.o font.o kbc.o x86.o	\
	intr.o pic.o sched.o hpet.o acpi.o nic.o pci.o handler.o fs.o	\
	common.o

$(TARGET): $(OBJS)
	x86_64-elf-ld $(LDFLAGS) -o $@ $+

%.o: %.c
	x86_64-elf-gcc $(CFLAGS) -c -o $@ $<
%.o: %.s
	x86_64-elf-gcc $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	cp $(TARGET) ../fs/
	qemu-system-x86_64 -m 4G \
	-drive if=pflash,format=raw,readonly=on,file=$$HOME/ovmf/OVMF_CODE.fd \
	-drive if=pflash,format=raw,file=$$HOME/ovmf/OVMF_VARS.fd \
	-netdev vmnet-bridged,id=net0,ifname=en0 \
	-device e1000,netdev=net0 \
	-hda fat:rw:../fs

clean:
	rm -f *~ *.o *.map $(TARGET) include/*~

.PHONY: run clean
