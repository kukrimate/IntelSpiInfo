ARCH   = x86_64
LIBEFI = libefi
include libefi/tools/Makefile-$(ARCH).efi

# Subsystem ID (EFI Application)
SUBSYSTEM = 10

APP = IntelSpiInfo$(EXT)
OBJ = IntelSpiInfo.o

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.efi
