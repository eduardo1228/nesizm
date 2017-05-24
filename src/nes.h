
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

// ppu properties and vram
struct nes_ppu {
	enum mirror_type {
		MT_HORIZONTAL,
		MT_VERTICAL,
		MT_4PANE
	};

	// oam data
	unsigned char oam[0x100];

	// up to four name tables potentially (most games use 2)
	nes_nametable nameTables[4];

	// palette ram (first 16 bytes are BG, second are OBJ palettes)
	unsigned char palette[0x20];

	// all 8 kb mapped for character memory at once (0x0000 - 0x2000)
	unsigned char* chrMap;

	// registers (some of them map to $2000-$2007, but this is handled case by case)
	unsigned char PPUCTRL;			// $2000
	unsigned char PPUMASK;			// $2001
	unsigned char PPUSTATUS;		// $2002
	unsigned char OAMADDR;			// $2003
	unsigned char OAMDATA;			// $2004 (only used for latch behavior)
	unsigned char SCROLLX;			// $2005 (a)
	unsigned char SCROLLY;			// $2005 (b)
	unsigned char ADDRHI;			// $2006 (a)
	unsigned char ADDRLO;			// $2006 (b)
	unsigned char DATA;				// $2007 (only used for latch behavior)

	// cur latch pointer (to the registers above)
	unsigned char* latch;

	mirror_type mirror;


	nes_ppu();

	unsigned char* latchReg(unsigned int regNum);
	void writeReg(unsigned int regNum, unsigned char value);
};

extern nes_cart nesCart;
extern nes_ppu nesPPU;