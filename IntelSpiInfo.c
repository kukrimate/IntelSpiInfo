#include <efi.h>
#include <efiutil.h>
#include "PortIo.h"

#define PCI_ADDR(bus, dev, fun, offs) \
	(bus << 16 | dev << 11 | fun << 8 | offs)

/* LPC bridge registers */
#define BIOS_CNTL PCI_ADDR(0, 0x1f, 0, 0xdc)
#define RCBA      PCI_ADDR(0, 0x1f, 0, 0xf0)

/* PCI config space access */
static uint32_t PciCfgRead32(uint32_t addr)
{
	outl(0xcf8, 0x80000000 | addr & 0xfffffffc);
	return inl(0xcfc);
}

static uint16_t PciCfgRead16(uint32_t addr)
{
	outl(0xcf8, 0x80000000 | addr & 0xfffffffc);
	return inw(0xcfc + (addr & 2));
}

static uint8_t PciCfgRead8(uint32_t addr)
{
	outl(0xcf8, 0x80000000 | addr & 0xfffffffc);
	return inb(0xcfc + (addr & 3));
}

/* SPI controller registers */
#define HSFS 0x3804
#define PR0  0x3874
#define PR1  0x3878
#define PR2  0x387c
#define PR3  0x3880
#define PR4  0x3884

/* MMIO access */
static uint32_t MmioRead32(uint32_t addr)
{
	uintptr_t p;

	p = addr;
	return *(uint32_t *) p;
}

static uint16_t MmioRead16(uint32_t addr)
{
	uintptr_t p;

	p = addr;
	return *(uint16_t *) p;
}

static uint8_t MmioRead8(uint32_t addr)
{
	uintptr_t p;

	p = addr;
	return *(uint8_t *) p;
}

efi_status
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;
	efi_size index;
	efi_in_key key;

	uint8_t  bios_cntl;
	uint32_t rcba;

	uint16_t hsfs;

	size_t i;
	uint32_t pr[5];

	status = EFI_SUCCESS;
	efi_init(image_handle, system_table);

	efi_print(L"IntelSpiInfo (U)EFI\n");
	efi_print(L"(C) Mate Kukri, 2020\n");

	bios_cntl = PciCfgRead8(BIOS_CNTL) & 0x3f;
	rcba      = PciCfgRead32(RCBA)     & 0xffffc000;

	efi_print(L"LPC Controller registers:\n");
	efi_print(L"  BIOS_CNTL: 0x%x\n", bios_cntl);
	efi_print(L"    SMM_BWP: %d\n", 0 != (bios_cntl & (1 << 5)));
	efi_print(L"    BLE:     %d\n", 0 != (bios_cntl & (1 << 1)));
	efi_print(L"    BIOSWE:  %d\n", 0 != (bios_cntl & (1 << 0)));
	efi_print(L"  RCBA:      0x%x\n", rcba);

	hsfs = MmioRead16(rcba + HSFS);

	efi_print(L"SPI Controller registers:\n");
	efi_print(L"  HSFS:      0x%x\n", hsfs);
	efi_print(L"    FLOCKDN: %d\n", 0 != (hsfs & (1 << 15)));
	for (i = 0; i < 5; ++i) {
		pr[i] = MmioRead32(rcba + PR0 + i * 4);
		efi_print(L"  PR%d:       0x%x\n", i, pr[i]);
		efi_print(L"    WP/RP:   %d/%d\n",
			0 < (pr[i] & 0x80000000), 0 < (pr[i] & 0x8000));
		efi_print(L"    Range:   0x%x-0x%x\n",
			(pr[i] & 0x1fff) << 12, (pr[i] & 0x1fff0000) >> 4 | 0xfff);
	}

	/* Wait for a keypress */
	efi_print(L"Press any key to exit\n");
	bs->wait_for_event(1, &st->con_in->wait_for_key, &index);
	st->con_in->read_key(st->con_in, &key);

done:
	return status;
}
