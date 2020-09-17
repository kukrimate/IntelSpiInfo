LIBEFI = ..
include ../tools/Makefile.efi

# Subsystem ID (EFI Application)
SUBSYSTEM := 10

all: IntelSpiInfo.efi

IntelSpiInfo.efi: IntelSpiInfo.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

copy: IntelSpiInfo.efi
	sudo mount /dev/sdc1 /mnt
	sudo cp IntelSpiInfo.efi /mnt/efi/boot/bootx64.efi
	sudo umount /mnt

clean:
	rm -f *.o *.efi
