
#pragma once

// NES emulator specific header

#define NES 1
#include "6502.h"

#define NUM_CACHED_ROM_BANKS 20

// specifications about the cart, rom file, mapper, etc
struct nes_cart {
	nes_cart();

	int handle;						// current file handle
	char file[128];					// file name

	int mapper;						// mapper ID used with ROM
	int numPRGBanks;				// num 16 kb Program ROM banks
	int numCHRBanks;				// num 8 kb CHR ROM banks
	int numRAMBanks;				// num 8 kb RAM banks

	int isBatteryBacked;			// 0 if no battery backup
	int isPAL;						// 1 if PAL, 0 if NTSC

	// 8kb ROM banks for various usages based on mapper (allocated on stack due to Prizm deficiencies
	unsigned char* romBanks[NUM_CACHED_ROM_BANKS];	

	// sets up ROM bank pointers with the given allocated data
	void allocateROMBanks(unsigned char* withAlloced);

	// Loads a ROM from the given file path
	bool loadROM(const char* withFile);

	// Sets up loaded ROM File with the selected mapper (returns false if unsupported)
	bool setupMapper();

	// various mapper setups
	void setupMapper0_NROM();
};

struct nes_nametable {
	unsigned char table[960];
	unsigned char attr[64];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PPU

#define PPUCTRL_FLIPXTBL    (1 << 0)		// flip the x axis nametable lookup? (doesn't matter for horizontal lookup)
#define PPUCTRL_FLIPYTBL    (1 << 1)		// flip the y axix nametable lookup? (doesn't matter for vertical lookup)
#define PPUCTRL_VRAMINC		(1 << 2)		// 0 = +1, 1 = +32
#define PPUCTRL_OAMTABLE	(1 << 3)		// 1 = 0x1000
#define PPUCTRL_BGDTABLE	(1 << 4)		// 1 = 0x1000
#define PPUCTRL_SPRSIZE		(1 << 5)		// 1 = 8x16
#define PPUCTRL_SLAVE		(1 << 6)
#define PPUCTRL_NMI			(1 << 7)

#define PPUMASK_GRAYSCALE	(1 << 0)
#define PPUMASK_SHOWLEFTBG	(1 << 1)		// if on, show left 8 pixels of background screen
#define PPUMASK_SHOWLEFTOBJ (1 << 2)		// if on, show left 8 pixels of objects screen
#define PPUMASK_SHOWBG		(1 << 3)
#define PPUMASK_SHOWOBJ		(1 << 4)
#define PPUMASK_EMPHRED		(1 << 5)
#define PPUMASK_EMPHGREEN	(1 << 6)
#define PPUMASK_EMPHBLUE	(1 << 7)

enum nes_mirror_type {
	MT_UNSET,
	MT_HORIZONTAL,
	MT_VERTICAL,
	MT_4PANE
};

struct ppu_registers_type {
	// registers (some of them map to $2000-$2007, but this is handled case by case)
	unsigned char PPUCTRL;			// $2000
	unsigned char PPUMASK;			// $2001
	unsigned char PPUSTATUS;		// $2002
	unsigned char OAMADDR;			// $2003
									// unsigned char OAMDATA;		// $2004 (unused with latch behavior)
	unsigned char SCROLLX;			// $2005 (a)
	unsigned char SCROLLY;			// $2005 (b)
	unsigned char ADDRHI;			// $2006 (a)
	unsigned char ADDRLO;			// $2006 (b)
									// unsigned char DATA;			// $2007 (unused with latch behavior)

	unsigned char* latch;			// cur latch pointer (to the registers above)

	unsigned char* latchReg(unsigned int regNum);
	void postReadLatch();
	void writeReg(unsigned int regNum, unsigned char value);

	ppu_registers_type() {
		memset(this, 0, sizeof(ppu_registers_type));
		latch = &PPUCTRL;
	}
};

// main ppu registers (2000-2007 and emulated latch)
extern ppu_registers_type ppu_registers;

// oam data
extern unsigned char ppu_oam[0x100];

// up to four name tables potentially (most games use 2)
extern nes_nametable ppu_nameTables[4];

// palette ram (first 16 bytes are BG, second are OBJ palettes)
extern unsigned char ppu_palette[0x20];

// all 8 kb mapped for character memory at once (0x0000 - 0x2000)
extern unsigned char* ppu_chrMap;

extern void ppu_setMirrorType(nes_mirror_type withType);

// reading / writing
extern unsigned char* ppu_resolveMemoryAddress(unsigned int addr);

extern void ppu_step();

extern nes_cart nesCart;