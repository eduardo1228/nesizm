
// Macro'd opcode table for various expansion behavior as needed

// Opcodes are defined by addressing mode :
//  OPCODE_NON		none or implied
//  OPCODE_IMM		immediate
//  OPCODE_REL		relative
//  OPCODE_ABS		absolute
//  OPCODE_ABX		absolute X
//  OPCODE_ABY		absolute Y
//  OPCODE_IND		indirect
//  OPCODE_INX		indirect X
//  OPCODE_INY		indirect Y
//  OPCODE_ZRO		zero page
//  OPCODE_ZRX		zero page X
//  OPCODE_ZRY		zero page Y

// if the individual macro's aren't defined, then we are looking for other behavior:

// opcode based on data width
#if !defined(OPCODE_NONE) && defined(OPCODE_0W)
#define OPCODE_NON		OPCODE_0W
#define OPCODE_IMM		OPCODE_1W
#ifndef OPCODE_REL
#define OPCODE_REL		OPCODE_1W
#endif
#define OPCODE_ABS		OPCODE_2W
#define OPCODE_ABX		OPCODE_2W
#define OPCODE_ABY		OPCODE_2W
#define OPCODE_IND		OPCODE_2W
#define OPCODE_INX		OPCODE_1W
#define OPCODE_INY		OPCODE_1W
#define OPCODE_ZRO		OPCODE_1W
#define OPCODE_ZRX		OPCODE_1W
#define OPCODE_ZRY		OPCODE_1W
#endif

// generic opcode
#if !defined(OPCODE_NONE) && defined(OPCODE)
#define OPCODE_NON(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(NON,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_IMM(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(IMM,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_REL(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(REL,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ABS(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ABS,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ABX(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ABX,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ABY(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ABY,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_IND(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(IND,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_INX(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(INX,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_INY(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(INY,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ZRO(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ZRO,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ZRX(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ZRX,Opcode,Str,Clks,Size,Page,Instr,Special)
#define OPCODE_ZRY(Opcode,Str,Clks,Size,Page,Instr,Special)		OPCODE(ZRY,Opcode,Str,Clks,Size,Page,Instr,Special)
#endif

// actual table, arguments are
// OPCODE(OpcodeNum, InstructionString, BaseClocks, Size, PageCross, InstrName, SpecialCode)

OPCODE_NON(	0x00,		"BRK",				0,	2,	0, BRK, {}		)
OPCODE_INX(	0x01,		"ORA ($%02X,X)",	6,	2,	1, ORA, {}		)
OPCODE_ZRO(	0x05,		"ORA $%04X",		3,	2,	0, ORA, {}		)
OPCODE_ZRO(	0x06,		"ASL $%02X",		5,	2,	0, ASL, {}		)
OPCODE_NON(	0x08,		"PHP",				3,	1,	0, PHP, {}		)
OPCODE_IMM( 0x09,		"ORA #$%02X",		2,	2,	0, ORA, {}		)
OPCODE_NON( 0x0a,		"ASL",				2,	1,	0, ASL, {}		)
OPCODE_ABS( 0x0d,		"ORA $%04X",		4,	3,  0, ORA, {}		)
OPCODE_ABS( 0x0e,		"ASL $%04X",		6,	3,	0, ASL, {}		)

OPCODE_REL( 0x10,		"BPL $%04X",		2,	2,	0, BPL, {}		)
OPCODE_INY( 0x11,		"ORA ($%02X,Y)",    5,	2,	1, ORA, {}		)
OPCODE_ZRX( 0x15,		"ORA $%02X,X",		4,	2,	0, ORA, {}		)
OPCODE_ZRX( 0x16,		"ASL $%02X,X",		6,	2,	0, ASL, {}		)
OPCODE_NON( 0x18,		"CLC",				2,	1,	0, CLC, {}		)
OPCODE_ABY( 0x19,		"ORA $%04X,Y",		4,	3,	1, ORA, {}		)
OPCODE_ABX( 0x1d,		"ORA $%04X,X",		4,	3,	1, ORA, {}		)
OPCODE_ABX( 0x1e,		"ASL $%04X,X",		7,	3,	0, ASL, {}		)

OPCODE_ABS( 0x20,		"JSR $%04X",		6,	2,	0, JSR, {}		)  // set to 2 to make correct PC push to stack
OPCODE_INX( 0x21,		"AND ($%02X,X)",	6,	2,	0, AND, {}		)
OPCODE_ZRO( 0x24,		"BIT $%02X",		3,	2,	0, BIT, {}		)
OPCODE_ZRO( 0x25,		"AND $%04X",		3,	2,	0, AND, {}		)
OPCODE_ZRO( 0x26,		"ROL $%04X",		5,	2,	0, ROL, {}		)
OPCODE_NON( 0x28,		"PLP",				4,	1,	0, PLP, {}		)
OPCODE_IMM( 0x29,		"AND #$%02X",		2,	2,	0, AND, {}		)
OPCODE_NON( 0x2a,		"ROL A",			2,	1,	0, ROL, {}		)
OPCODE_ABS( 0x2c,		"BIT $%04X",		4,	3,	0, BIT, {}		)
OPCODE_ABS( 0x2d,		"AND $%04X",		4,	3,	0, AND, {}		)
OPCODE_ABS( 0x2e,		"ROL $%04X",		6,	3,	0, ROL, {}		)

OPCODE_REL( 0x30,		"BMI $%04X",		2,	2,	0, BMI, {}		)
OPCODE_INY( 0x31,		"AND ($%02X),Y",	5,	2,	1, AND, {}		)
OPCODE_ZRX( 0x35,		"AND $%02X,X",		4,	2,	0, AND, {}		)
OPCODE_ZRX( 0x36,		"ROL $%02X,X",		6,	2,	0, ROL, {}		)
OPCODE_NON( 0x38,		"SEC",				2,	1,	0, SEC, {}		)
OPCODE_ABY( 0x39,		"AND $%04X,Y",		4,	3,	1, AND, {}		)
OPCODE_ABX( 0x3d,		"AND $%04X,X",		4,	3,	1, AND, {}		)
OPCODE_ABX( 0x3e,		"ROL $%04X,X",		7,	3,	0, ROL, {}		)

OPCODE_NON( 0x40,		"RTI",				6,	1,	0, RTI, {}		)
OPCODE_INX( 0x41,		"EOR ($%02X,X)",	6,	2,	0, EOR, {}		)
OPCODE_ZRO( 0x45,		"EOR $%04X",		3,	2,	0, EOR, {}		)
OPCODE_ZRO( 0x46,		"LSR $%04X",		5,	2,	0, LSR, {}		)
OPCODE_NON( 0x48,		"PHA",				3,	1,	0, PHA, {}		)
OPCODE_IMM( 0x49,		"EOR #$%02X",		2,	2,	0, EOR, {}		)
OPCODE_NON( 0x4a,		"LSR",				2,	1,	0, LSR, {}		)
OPCODE_ABS( 0x4c,		"JMP $%04X",		3,	3,	0, JMP, {}		)
OPCODE_ABS( 0x4d,		"EOR $%04X",		4,	3,	0, EOR, {}		)
OPCODE_ABS( 0x4e,		"LSR $%04X",		6,	3,	0, LSR, {}		)
													
OPCODE_REL( 0x50,		"BVC $%04X",		2,	2,	0, BVC, {}		)
OPCODE_INY( 0x51,		"EOR ($%02X),Y",	5,	2,	1, EOR, {}		)
OPCODE_ZRX( 0x55,		"EOR $%02X,X",		4,	2,	0, EOR, {}		)
OPCODE_ZRX( 0x56,		"LSR $%02X,X",		6,	2,	0, LSR, {}		)
OPCODE_NON( 0x58,		"CLI",				2,	1,	0, CLI, {}		)
OPCODE_ABY( 0x59,		"EOR $%04X,Y",		4,	3,	1, EOR, {}		)
OPCODE_ABX( 0x5d,		"EOR $%04X,X",		4,	3,	1, EOR, {}		)
OPCODE_ABX( 0x5e,		"LSR $%04X,X",		7,	3,	0, LSR, {}		)
													
OPCODE_NON( 0x60,		"RTS",				6,	1,	0, RTS, {}		)
OPCODE_INX( 0x61,		"ADC ($%02X,X)",	6,	2,	0, ADC, {}		)
OPCODE_ZRO( 0x65,		"ADC $%04X",		3,	2,	0, ADC, {}		)
OPCODE_ZRO( 0x66,		"ROR $%04X",		5,	2,	0, ROR, {}		)
OPCODE_NON( 0x68,		"PLA",				4,	1,	0, PLA, {}		)
OPCODE_IMM( 0x69,		"ADC #$%02X",		2,	2,	0, ADC, {}		)
OPCODE_NON( 0x6a,		"ROR",				2,	1,	0, ROR, {}		)
OPCODE_IND( 0x6c,		"JMP ($%04X)",		5,	3,	0, JMP, {}		)
OPCODE_ABS( 0x6d,		"ADC $%04X",		4,	3,	0, ADC, {}		)
OPCODE_ABS( 0x6e,		"ROR $%04X",		6,	3,	0, ROR, {}		)
													
OPCODE_REL( 0x70,		"BVS $%04X",		2,	2,	0, BVS, {}		)
OPCODE_INY( 0x71,		"ADC ($%02X),Y",	5,	2,	1, ADC, {}		)
OPCODE_ZRX( 0x75,		"ADC $%02X,X",		4,	2,	0, ADC, {}		)
OPCODE_ZRX( 0x76,		"ROR $%02X,X",		6,	2,	0, ROR, {}		)
OPCODE_NON( 0x78,		"SEI",				2,	1,	0, SEI, {}		)
OPCODE_ABY( 0x79,		"ADC $%04X,Y",		4,	3,	1, ADC, {}		)
OPCODE_ABX( 0x7d,		"ADC $%04X,X",		4,	3,	1, ADC, {}		)
OPCODE_ABX( 0x7e,		"ROR $%04X,X",		7,	3,	0, ROR, {}		)
													
OPCODE_INX( 0x81,		"STA ($%02X,X)",	6,	2,	0, STA, {}		)
OPCODE_ZRO( 0x84,		"STY $%04X",		3,	2,	0, STY, {}		)
OPCODE_ZRO( 0x85,		"STA $%04X",		3,	2,	0, STA, {}		)
OPCODE_ZRO( 0x86,		"STX $%04X",		3,	2,	0, STX, {}		)
OPCODE_NON( 0x88,		"DEY",				2,	1,	0, DEY, {}		)
OPCODE_NON( 0x8a,		"TXA",				2,	1,	0, TXA, {}		)
OPCODE_ABS( 0x8c,		"STY $%04X",		4,	3,	0, STY, {}		)
OPCODE_ABS( 0x8d,		"STA $%04X",		4,	3,	0, STA, {}		)
OPCODE_ABS( 0x8e,		"STX $%04X",		4,	3,	0, STX, {}		)
													
OPCODE_REL( 0x90,		"BCC $%04X",		2,	2,	0, BCC, {}		)
OPCODE_INY( 0x91,		"STA ($%02X),Y",	6,	2,	0, STA, {}		)
OPCODE_ZRX( 0x94,		"STY $%02X,X",		4,	2,	0, STY, {}		)
OPCODE_ZRX( 0x95,		"STA $%02X,X",		4,	2,	0, STA, {}		)
OPCODE_ZRY( 0x96,		"STX $%02X,Y",		4,	2,	0, STX, {}		)
OPCODE_NON( 0x98,		"TYA",				2,	1,	0, TYA, {}		)
OPCODE_ABY( 0x99,		"STA $%04X,Y",		5,	3,	0, STA, {}		)
OPCODE_NON( 0x9a,		"TXS",				2,	1,	0, TXS, {}		)
OPCODE_ABX( 0x9d,		"STA $%04X,X",		5,	3,	0, STA, {}		)
													
OPCODE_IMM( 0xa0,		"LDY #$%02X",		2,	2,	0, LDY, {}		)
OPCODE_INX( 0xa1,		"LDA ($%02X,X)",	6,	2,	0, LDA, {}		)
OPCODE_IMM( 0xa2,		"LDX #$%02X",		2,	2,	0, LDX, {}		)
OPCODE_ZRO( 0xa4,		"LDY $%04X",		3,	2,	0, LDY, {}		)
OPCODE_ZRO( 0xa5,		"LDA $%04X",		3,	2,	0, LDA, {}		)
OPCODE_ZRO( 0xa6,		"LDX $%04X",		3,	2,	0, LDX, {}		)
OPCODE_NON( 0xa8,		"TAY",				2,	1,	0, TAY, {}		)
OPCODE_IMM( 0xa9,		"LDA #$%02X",		2,	2,	0, LDA, {}		)
OPCODE_NON( 0xaa,		"TAX",				2,	1,	0, TAX, {}		)
OPCODE_ABS( 0xac,		"LDY $%04X",		4,	3,	0, LDY, {}		)
OPCODE_ABS( 0xad,		"LDA $%04X",		4,	3,	0, LDA, {}		)
OPCODE_ABS( 0xae,		"LDX $%04X",		4,	3,	0, LDX, {}		)
													 
OPCODE_REL( 0xb0,		"BCS $%04X",		2,	2,	0, BCS, {}		)
OPCODE_INY( 0xb1,		"LDA ($%02X),Y",	5,	2,	1, LDA, {}		)
OPCODE_ZRX( 0xb4,		"LDY $%02X,X",		4,	2,	0, LDY, {}		)
OPCODE_ZRX( 0xb5,		"LDA $%02X,X",		4,	2,	0, LDA, {}		)
OPCODE_ZRY( 0xb6,		"LDX $%02X,Y",		4,	2,	0, LDX, {}		)
OPCODE_NON( 0xb8,		"CLV",				2,	1,	0, CLV, {}		)
OPCODE_ABY( 0xb9,		"LDA $%04X,Y",		4,	3,	1, LDA, {}		)
OPCODE_NON( 0xba,		"TSX",				2,	1,	0, TSX, {}		)
OPCODE_ABX( 0xbc,		"LDY $%04X,X",		4,	3,	1, LDY, {}		)
OPCODE_ABX( 0xbd,		"LDA $%04X,X",		4,	3,	1, LDA, {}		)
OPCODE_ABY( 0xbe,		"LDX $%04X,Y",		4,	3,	1, LDX, {}		)
													
OPCODE_IMM( 0xc0,		"CPY #$%02X",		2,	2,	0, CPY, {}		)
OPCODE_INX( 0xc1,		"CMP ($%02X,X)",	6,	2,	0, CMP, {}		)
OPCODE_ZRO( 0xc4,		"CPY $%02X",		3,	2,	0, CPY, {}		)
OPCODE_ZRO( 0xc5,		"CMP $%04X",		3,	2,	0, CMP, {}		)
OPCODE_ZRO( 0xc6,		"DEC $%04X",		5,	2,	0, DEC, {}		)
OPCODE_NON( 0xc8,		"INY",				2,	1,	0, INY, {}		)
OPCODE_IMM( 0xc9,		"CMP #$%02X",		2,	2,	0, CMP, {}		)
OPCODE_NON( 0xca,		"DEX",				2,	1,	0, DEX, {}		)
OPCODE_ABS( 0xcc,		"CPY $%04X",		4,	3,	0, CPY, {}		)
OPCODE_ABS( 0xcd,		"CMP $%04X",		4,	3,	0, CMP, {}		)
OPCODE_ABS( 0xce,		"DEC $%04X",		6,	3,	0, DEC, {}		)
													
OPCODE_REL( 0xd0,		"BNE $%04X",		2,	2,	0, BNE, {}		)
OPCODE_INY( 0xd1,		"CMP ($%02X),Y",	5,	2,	1, CMP, {}		)
OPCODE_ZRX( 0xd5,		"CMP $%02X,X",		4,	2,	0, CMP, {}		)
OPCODE_ZRX( 0xd6,		"DEC $%02X,X",		6,	2,	0, DEC, {}		)
OPCODE_NON( 0xd8,		"CLD",				2,	1,	0, CLD, {}		)
OPCODE_ABY( 0xd9,		"CMP $%04X,Y",		4,	3,	1, CMP, {}		)
OPCODE_ABX( 0xdd,		"CMP $%04X,X",		4,	3,	1, CMP, {}		)
OPCODE_ABX( 0xde,		"DEC $%04X,X",		7,	3,	0, DEC, {}		)
													
OPCODE_IMM( 0xe0,		"CPX #$%02X",		2,	2,	0, CPX, {}		)
OPCODE_INX( 0xe1,		"SBC ($%02X,X)",	6,	2,	0, SBC, {}		)
OPCODE_ZRO( 0xe4,		"CPX $%02X",		3,	2,	0, CPX, {}		)
OPCODE_ZRO( 0xe5,		"SBC $%02X",		3,	2,	0, SBC, {}		)
OPCODE_ZRO( 0xe6,		"INC $%04X",		5,	2,	0, INC, {}		)
OPCODE_NON( 0xe8,		"INX",				2,	1,	0, INX, {}		)
OPCODE_IMM( 0xe9,		"SBC #$%02X",		2,	2,	0, SBC, {}		)
OPCODE_NON( 0xea,		"NOP",				2,	1,	0, NOP, {}		)
OPCODE_ABS( 0xec,		"CPX $%04X",		4,	3,	0, CPX, {}		)
OPCODE_ABS( 0xed,		"SBC $%04X",		4,	3,	0, SBC, {}		)
OPCODE_ABS( 0xee,		"INC $%04X",		6,	3,	0, INC, {}		)
													
OPCODE_REL( 0xf0,		"BEQ $%04X",		2,	2,	0, BEQ, {}		)
OPCODE_INY( 0xf1,		"SBC ($%02X),Y",	5,	2,	1, SBC, {}		)
OPCODE_ZRX( 0xf5,		"SBC $%02X,X",		4,	2,	0, SBC, {}		)
OPCODE_ZRX( 0xf6,		"INC $%02X,X",		6,	2,	0, INC, {}		)
OPCODE_NON( 0xf8,		"SED",				2,	1,	0, SED, {}		)
OPCODE_ABY( 0xf9,		"SBC $%04X,Y",		4,	3,	1, SBC, {}		)
OPCODE_ABX( 0xfd,		"SBC $%04X,X",		4,	3,	1, SBC, {}		)
OPCODE_ABX( 0xfe,		"INC $%04X,Y",		7,	3,	0, INC, {}		)

#undef OPCODE
#undef OPCODE_0W
#undef OPCODE_1W
#undef OPCODE_2W
#undef OPCODE_NON
#undef OPCODE_IMM
#undef OPCODE_REL
#undef OPCODE_ABS
#undef OPCODE_ABX
#undef OPCODE_ABY
#undef OPCODE_IND
#undef OPCODE_INX
#undef OPCODE_INY
#undef OPCODE_ZRO
#undef OPCODE_ZRX
#undef OPCODE_ZRY