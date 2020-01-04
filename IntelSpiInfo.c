#include <efi.h>
#include <efiutil.h>
#include "EfiPrint.h"
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
	uint8_t  byte_offset;
	uint32_t val;

	byte_offset = addr & 2;
	val         = PciCfgRead32(addr);

	switch (byte_offset) {
	case 0:
		return (uint16_t) (val & 0xffff);
	case 2:
		return (uint16_t) ((val & 0xffff0000) >> 16);
	}
}

static uint8_t PciCfgRead8(uint32_t addr)
{
	uint8_t  byte_offset;
	uint32_t val;

	byte_offset = addr & 3;
	val         = PciCfgRead32(addr);

	switch (byte_offset) {
	case 0:
		return (uint8_t) (val & 0xff);
	case 1:
		return (uint8_t) ((val & 0xff00) >> 8);
	case 2:
		return (uint8_t) ((val & 0xff0000) >> 16);
	case 3:
		return (uint8_t) ((val & 0xff000000) >> 24);
	}
}

/* SPI controller registers */
#define HSFS 0x3804
#define PR0  0x3878
#define PR1  0x387c
#define PR2  0x3880
#define PR3  0x3884

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

efi_status_t
efi_func
efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
	efi_status_t status;
	uintn_t index;
	efi_in_key_t key;

	uint8_t  bios_cntl;
	uint32_t rcba;

	uint16_t hsfs;

	size_t i;
	uint32_t pr[4];

	status = EFI_SUCCESS;
	init_util(image_handle, system_table);
	Print(L"IntelSpiInfo (U)EFI\r\n");
	Print(L"(C) Mate Kukri, 2020\r\n");

	bios_cntl = PciCfgRead8(BIOS_CNTL) & 0x3f;
	rcba      = PciCfgRead32(RCBA)     & 0xffffc000;

	Print(L"LPC Controller registers:\r\n");
	Print(L"  BIOS_CNTL: 0x%x\r\n", bios_cntl);
	Print(L"    SMM_BWP: %u\r\n", 0 < (bios_cntl & (1 << 5)));
	Print(L"    BLE:     %u\r\n", 0 < (bios_cntl & (1 << 1)));
	Print(L"    BIOSWE:  %u\r\n", 0 < (bios_cntl & (1 << 0)));
	Print(L"  RCBA:      0x%x\r\n", rcba);

	hsfs = MmioRead16(rcba + HSFS);

	Print(L"SPI Controller registers:\r\n");
	Print(L"  HSFS:      0x%x\r\n", hsfs);
	Print(L"    FLOCKDN: %u\r\n", 0 < (hsfs & (1 << 15)));
	for (i = 0; i < 4; ++i) {
		pr[i] = MmioRead32(rcba + PR0 + i * 4);
		Print(L"  PR%u:       0x%x\r\n", i, pr[i]);
		Print(L"    WP/RP:   %u/%u\r\n",
			0 < (pr[i] & 0x80000000), 0 < (pr[i] & 0x8000));
		Print(L"    Range:   0x%x-0x%x\r\n",
			(pr[i] & 0x1fff) << 12, (pr[i] & 0x1fff0000) >> 4 | 0xfff);
	}

	/* Wait for a keypress */
	Print(L"Press any key to exit\r\n");
	bs->wait_for_event(1, &st->con_in->wait_for_key, &index);
	st->con_in->read_key(st->con_in, &key);

done:
	fini_util();
	return status;
}
