include ../tools/Makefile.efi

all: IntelSpiInfo.efi

IntelSpiInfo.efi: IntelSpiInfo.o EfiPrint.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

copy: IntelSpiInfo.efi
	sudo mount /dev/sdc1 /mnt
	sudo cp IntelSpiInfo.efi /mnt/efi/boot/bootx64.efi
	sudo umount /mnt

clean:
	rm -f *.o *.efi
