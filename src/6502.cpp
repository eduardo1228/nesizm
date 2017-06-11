
// main 6502 interpreter and emulation
#include "platform.h"
#include "debug.h"

#define NES 1
#include "6502.h"

#if TARGET_WINSIM
#define TRACE_INSTRUCTIONS 1
#else
#define TRACE_INSTRUCTIONS 0
#endif

#if DEBUG
static unsigned int cpuBreakpoint = 0x0000;
#endif

#if TRACE_INSTRUCTIONS
static cpu_instr_history traceHistory[100] = { 0 };
static unsigned int traceNum;
void HitBreakpoint();
#endif

#ifdef LITTLE_E
#define PTR_TO_BYTE(x) ((unsigned char*) &x)
#else
#define PTR_TO_BYTE(x) (((unsigned char*) &x) + 3)
#endif

unsigned int clockTable[256] = { 0 };

void cpu6502_Init() {
	#define OPCODE(op,str,clk,sz,spc) clockTable[op] = clk;
	#include "6502_opcodes.inl"
}

// the low 5 bits of the opcode determines the addressing mode with only 5 instruction exceptions (noted)
static int modeTable[32] = {
	AM_None,		// 00
	AM_IndirectX,	// 01
	AM_None,		// 02 (mostly unused)
	AM_None,		// 03 (unused)
	AM_Zero,		// 04
	AM_Zero,		// 05
	AM_Zero,		// 06
	AM_None,		// 07 (unused)

	AM_None,		// 08 (all implied)
	AM_None,		// 09 (all immediate)
	AM_None,		// 0A (A register or implied)
	AM_None,		// 0B (unused)
	AM_Absolute,	// 0C (exceptions at 4C and 6C with JMPs)
	AM_Absolute,	// 0D 
	AM_Absolute,	// 0E
	AM_None,		// 0F (unused)

	AM_None,		// 10 (all relative JMPs and branches)
	AM_IndirectY,	// 11
	AM_None,		// 12 (unused)
	AM_None,		// 13 (unused)
	AM_ZeroX,		// 14 (only 94 and B4)
	AM_ZeroX,		// 15
	AM_ZeroX,		// 16 (exceptions at 96 and B6 which are ZeroY)
	AM_None,		// 17 (unused)

	AM_None,		// 18 (all implied)
	AM_AbsoluteY,	// 19
	AM_None,		// 1A (only used for 9A and BA)
	AM_None,		// 1B (unused)
	AM_AbsoluteX,	// 1C (only used for BC)
	AM_AbsoluteX,	// 1D
	AM_AbsoluteX,	// 1E (exception at BE which is AbsoluteY)
	AM_None,		// 1F (unused)
};

void cpu6502_Step() {
#if TRACE_INSTRUCTIONS
	cpu_instr_history hist;
	memcpy(&hist.regs, &mainCPU, sizeof(cpu_6502));
#endif

	// TODO : cache into single read (with instruction overlap in bank pages)
	unsigned int instr = mainCPU.read(mainCPU.PC);
	unsigned int data1 = mainCPU.read(mainCPU.PC + 1);
	unsigned int data2 = mainCPU.read(mainCPU.PC + 2);

	// effective address
	unsigned int effAddr = 0;
	unsigned char* operand;
	unsigned int isSpecial = 0;
	unsigned int isWrite = 0;

	// instruction base clocks
	mainCPU.clocks += clockTable[instr];

	// TODO : perf (try labels here)
	switch (modeTable[instr & 0x1F]) {
		case AM_Absolute:
			effAddr = data1 + (data2 << 8);
			operand = mainCPU.getByte(effAddr, isSpecial);
			mainCPU.PC += 3;
			break;
		case AM_AbsoluteX:
			effAddr = data1 + (data2 << 8) + (mainCPU.X);
			operand = mainCPU.getByte(effAddr, isSpecial);
			mainCPU.PC += 3;
			break;
		case AM_AbsoluteY:
			effAddr = data1 + (data2 << 8) + (mainCPU.Y);
			operand = mainCPU.getByte(effAddr, isSpecial);
			mainCPU.PC += 3;
			break;
		case AM_IndirectX:
		{
			int target = (data1 + mainCPU.X) & 0xFF;
			effAddr = mainCPU.RAM[target] + (mainCPU.RAM[(target + 1) & 0xFF] << 8);
			operand = mainCPU.getByte(effAddr, isSpecial);
			mainCPU.PC += 2;
			break;
		}
		case AM_IndirectY:
		{
			effAddr = mainCPU.RAM[data1] + (mainCPU.RAM[(data1 + 1) & 0xFF] << 8) + mainCPU.Y;
			operand = mainCPU.getByte(effAddr, isSpecial);
			mainCPU.PC += 2;
			break;
		}
		case AM_Zero:
			effAddr = data1;
			operand = &mainCPU.RAM[effAddr];
			mainCPU.PC += 2;
			break;
		case AM_ZeroX:
			effAddr = (data1 + mainCPU.X) & 0xFF;
			operand = &mainCPU.RAM[effAddr];
			mainCPU.PC += 2;
			break;
		default:
			DebugAssert(modeTable[instr & 0x1F] == AM_None);
			mainCPU.PC += 1;
			break;
	}

#if TRACE_INSTRUCTIONS
	hist.instr = instr;
	hist.data1 = data1;
	hist.data2 = data2;
	if (modeTable[instr & 0x1F] != AM_None) {
		hist.effByte = *operand;
	}
	/*
	hist.output();
	if (mainCPU.clocks > 3000) {
		DebugBreak();
	}
	*/

	traceHistory[traceNum++] = hist;
	if (traceNum == 100) traceNum = 0;

	if (hist.regs.PC == cpuBreakpoint) {
		HitBreakpoint();
	}
#endif

	unsigned int result;	// used in some instructions

	switch (instr) {
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STORE / LOAD

		case 0xA9:
			// LDA #$NN (load A immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1:
			// LDA (load accumulator)
			mainCPU.A = *operand;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.A == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0xA2:
			// LDX #$NN (load X immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xA6: case 0xB6: case 0xAE: case 0xBE:
			// LDX (load X)
			mainCPU.X = *operand;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((mainCPU.X & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.X == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0xA0:
			// LDY #$NN (load Y immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xA4: case 0xB4: case 0xAC: case 0xBC:
			// LDY (load Y)
			mainCPU.Y = *operand;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((mainCPU.Y & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.Y == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91:
			// STA (store accumulator)
			result = mainCPU.A;
			isWrite = 1;
			break;

		case 0x86: case 0x96: case 0x8E:
			// STX (store X)
			result = mainCPU.X;
			isWrite = 1;
			break;

		case 0x84: case 0x94: case 0x8C:
			// STY (store Y)
			result = mainCPU.Y;
			isWrite = 1;
			break;

		case 0xAA:
			// TAX (A -> X)
			mainCPU.X = mainCPU.A;
			goto trxFlags;
		case 0x8A:
			// TXA (X -> A)
			mainCPU.A = mainCPU.X;
			goto trxFlags;
		case 0xA8:
			// TAY (A -> Y)
			mainCPU.Y = mainCPU.A;
			goto trxFlags;
		case 0x98:
			// TYA (Y -> A)
			mainCPU.A = mainCPU.Y;
		trxFlags:
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.A == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0xBA:
			// TSX (SP -> X)
			mainCPU.X = mainCPU.SP;
		case 0x9A:
			// TXS (X -> SP)
			mainCPU.SP = mainCPU.X;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((mainCPU.X & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.X == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0x48:
			// PHA (push A)
			mainCPU.push(mainCPU.A);
			break;

		case 0x68:
			// PLA (pull A)
			mainCPU.A = mainCPU.pop();
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.A == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0x08:
			// PHP (push processor status)
			mainCPU.push(mainCPU.P | ST_UNUSED | ST_BRK);
			break;

		case 0x28:
			// PLP (pop processor status ignoring bits 4 & 5)
			mainCPU.P &= (ST_BRK | ST_UNUSED);
			mainCPU.P |= mainCPU.pop() & ~(ST_BRK | ST_UNUSED);
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ALU

		case 0x69:
			// ADC #$NN (or immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71:
			// ADC
			// TODO : possibly move overflow calculation to flag resolve?
			result = mainCPU.A + *operand + (mainCPU.P & ST_CRY);
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BRK | ST_BCD)) |								// keep flags
				((result & 0x100) >> (8 - ST_CRY_BIT)) |								// carry
				((result & 0xFF) == 0 ? ST_ZRO : 0) |									// zero
				(((mainCPU.A^result)&(*operand^result) & 0x80) >> (7 - ST_OVR_BIT)) |	// overflow (http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html)
				((result & 0x80) >> (7 - ST_NEG_BIT));									// negative (sign)
			mainCPU.A = result & 0xFF;
			break;
			
		case 0xE9:
			// SBC #$NN (or immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1:
			// SBC
			// TODO : possibly move overflow calculation to flag resolve?
			result = mainCPU.A - *operand - (mainCPU.P & ST_CRY);
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BRK | ST_BCD)) |									// keep flags
				((result & 0x100) >> (8 - ST_CRY_BIT)) |									// carry
				((result & 0xFF) == 0 ? ST_ZRO : 0) |										// zero
				(((mainCPU.A^result)&((~(*operand))^result) & 0x80) >> (7 - ST_OVR_BIT)) |	// overflow (http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html)
				((result & 0x80) >> (7 - ST_NEG_BIT));										// negative (sign)
			mainCPU.A = result & 0xFF;
			break;

		case 0xC6: case 0xD6: case 0xCE: case 0xDE:
			// DEC
			result = (*operand - 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_CRY)) |			// keep flags
				((result & 0x80) >> (7 - ST_NEG_BIT)) |									// sign flag
				(result == 0x00 ? ST_ZRO : 0);											// zero flag
			isWrite = 1;
			break;

		case 0xE6: case 0xF6: case 0xEE: case 0xFE:
			// INC
			result = (*operand + 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_NEG)) |			// keep flags
				((result & 0x80) >> (7 - ST_NEG_BIT)) |									// sign flag
				(result == 0x00 ? ST_ZRO : 0);											// zero flag
			isWrite = 1;
			break;

		case 0xCA: 
			// TODO (perf) (try -= 2 and skip flags and break?)
			// DEX (decrement X)
			mainCPU.X = (mainCPU.X - 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_CRY)) |			// keep flags
				((mainCPU.X & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.X == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0xE8: 
			// INX (increment X)
			mainCPU.X = (mainCPU.X + 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_CRY)) |			// keep flags
				((mainCPU.X & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.X == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0x88: 
			// DEY (decrement Y)
			// TODO (perf) (try -= 2 and skip flags and break?)
			mainCPU.Y = (mainCPU.Y - 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_CRY)) |			// keep flags
				((mainCPU.Y & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.Y == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		case 0xC8: 
			// INY (increment Y)
			mainCPU.Y = (mainCPU.Y + 1) & 0xFF;
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR | ST_CRY)) |			// keep flags
				((mainCPU.Y & 0x80) >> (7 - ST_NEG_BIT)) |								// sign flag
				((mainCPU.Y == 0x00) ? ST_ZRO : 0);										// zero flag
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// BITWISE
			
		case 0x09:
			// ORA #$NN (or immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11: 
			// ORA
			mainCPU.A = mainCPU.A | *operand;
			mainCPU.P = 
				(mainCPU.P & (ST_CRY | ST_INT | ST_BCD | ST_BRK | ST_OVR)) |			// keep flags
				((mainCPU.A == 0) ? ST_ZRO : 0) |										// zero
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT));								// negative (sign)
			break;

		case 0x29:
			// AND #$NN (and immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31: 
			// AND
			mainCPU.A = mainCPU.A & *operand;
			mainCPU.P = 
				(mainCPU.P & (ST_CRY | ST_INT | ST_BCD | ST_BRK | ST_OVR)) |			// keep flags
				((mainCPU.A == 0) ? ST_ZRO : 0) |										// zero
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT));								// negative (sign)
			break;

		case 0x49:
			// EOR #$NN (xor immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51: 
			// EOR
			mainCPU.A = mainCPU.A ^ *operand;
			mainCPU.P = 
				(mainCPU.P & (ST_CRY | ST_INT | ST_BCD | ST_BRK | ST_OVR)) |			// keep flags
				((mainCPU.A == 0) ? ST_ZRO : 0) |										// zero
				((mainCPU.A & 0x80) >> (7 - ST_NEG_BIT));								// negative (sign)
			break;

		case 0x0A:
			// ASL A
			operand = PTR_TO_BYTE(mainCPU.A);
		case 0x06: case 0x16: case 0x0E: case 0x1E:
			// ASL
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand & 0x80) >> (7 - ST_CRY_BIT)) |								// carry
				(((*operand & 0x7F) == 0) ? ST_ZRO : 0) |								// zero
				((*operand & 0x40) << (ST_NEG_BIT - 6));								// negative (sign)
			result = (*operand << 1) & 0xFF;
			isWrite = 1;
			break;
			
		case 0x4A:
			// LSR A
			operand = PTR_TO_BYTE(mainCPU.A);
		case 0x46: case 0x56: case 0x4E: case 0x5E:
			// LSR
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand & 0x01) >> (0 - ST_CRY_BIT)) |								// carry
				(((*operand & 0xFE) == 0) ? ST_ZRO : 0) |								// zero
				0;																		// negative (sign) always cleared
			result = *operand >> 1;
			isWrite = 1;
			break;

		case 0x2A:
			// ROL A
			operand = PTR_TO_BYTE(mainCPU.A);
		case 0x26: case 0x36: case 0x2E: case 0x3E:
			// ROL 
			result = (*operand << 1) | (mainCPU.P & ST_CRY);
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((result & 0x100) >> (8 - ST_CRY_BIT)) |								// carry
				(((result & 0xFF) == 0) ? ST_ZRO : 0) |									// zero
				((result & 0x80) >> (7 - ST_NEG_BIT));									// negative (sign)
			isWrite = 1;
			break;

		case 0x6A:
			// ROR A
			operand = PTR_TO_BYTE(mainCPU.A);
		case 0x66: case 0x76: case 0x6E: case 0x7E:
			result = (*operand >> 1) | ((mainCPU.P & ST_CRY) << (7 - ST_CRY_BIT));
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand & 0x01) >> (0 - ST_CRY_BIT)) |								// carry
				((result == 0) ? ST_ZRO : 0) |											// zero
				((result & 0x80) >> (7 - ST_NEG_BIT));									// negative (sign)
			isWrite = 1;
			break;

		case 0x24: case 0x2C:
			// BIT
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_CRY)) |						// keep flags
				(((*operand & mainCPU.A) == 0) ? ST_ZRO : 0) |							// zero flag
				(*operand & (ST_OVR | ST_NEG));											// these are copied in (bits 6-7)
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// BRANCH / JUMP

		case 0x10:	
			// BPL
			result = (~mainCPU.P) & ST_NEG;
			goto finishBranch;
		case 0x30:
			// BMI
			result = mainCPU.P & ST_NEG;
			goto finishBranch;
		case 0x50:
			// BVC
			result = (~mainCPU.P) & ST_OVR;
			goto finishBranch;
		case 0x70:
			// BVS
			result = mainCPU.P & ST_OVR;
			goto finishBranch;
		case 0x90:
			// BCC
			result = (~mainCPU.P) & ST_CRY;
			goto finishBranch;
		case 0xB0:
			// BCS
			result = mainCPU.P & ST_CRY;
			goto finishBranch;
		case 0xD0:
			// BNE
			result = (~mainCPU.P) & ST_ZRO;
			goto finishBranch;
		case 0xF0:
			// BEQ
			result = mainCPU.P & ST_ZRO;
		finishBranch:
			mainCPU.PC++;
			if (result) {
				unsigned int oldPC = mainCPU.PC;
				mainCPU.PC += (char) (data1);
				mainCPU.clocks += 1 + ((oldPC & 0xFF00) != (mainCPU.PC & 0xFF00) ? 1 : 0);
			}
			break;

		case 0x4C:
			// JMP (absolute)
			mainCPU.PC = data1 + (data2 << 8);
			break;

		case 0x6C:
			// JMP (indirect)
			DebugAssert(data1 != 0xFF);			// don't support jumps at end of a page 
			mainCPU.PC = operand[0] + (operand[1] << 8);
			break;

		case 0x20:
			// JSR (subroutine)
			mainCPU.PC += 1;
			mainCPU.push(mainCPU.PC >> 8);
			mainCPU.push(mainCPU.PC & 0xFF);

			mainCPU.PC = data1 + (data2 << 8);
			break;

		case 0x40:
			// RTI (return from interrupt)
			// TODO : Non- delayed IRQ response behavior?
			mainCPU.P &= (ST_BRK | ST_UNUSED);
			mainCPU.P |= mainCPU.pop() & ~(ST_BRK | ST_UNUSED);
			mainCPU.PC = mainCPU.pop() | (mainCPU.pop() << 8);
			break;

		case 0x60:
			// RTS
			mainCPU.PC = mainCPU.pop() | (mainCPU.pop() << 8);
			mainCPU.PC++;
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// COMPARE / TEST

		case 0xC9:
			// CMP #$NN (compare immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xC5: case 0xD5: case 0xCD: case 0xDD: case 0xD9: case 0xC1: case 0xD1:
			// CMP
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand > mainCPU.A) ? ST_CRY : 0) |									// carry flag
				((*operand == mainCPU.A) ? ST_ZRO : 0) |								// zero flag
				(((mainCPU.A - *operand) & 0x80) >> (7 - ST_NEG_BIT));					// negative (sign) flag
			break;

		case 0xE0:
			// CPX #$NN (compare immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xE4: case 0xEC:
			// CPX
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand > mainCPU.X) ? ST_CRY : 0) |									// carry flag
				((*operand == mainCPU.X) ? ST_ZRO : 0) |								// zero flag
				(((mainCPU.X - *operand) & 0x80) >> (7 - ST_NEG_BIT));					// negative (sign) flag
			break;
			
		case 0xC0:
			// CPY #$NN (compare immediate)
			mainCPU.PC++;
			operand = PTR_TO_BYTE(data1);
		case 0xC4: case 0xCC:
			// CPY
			mainCPU.P =
				(mainCPU.P & (ST_INT | ST_BCD | ST_BRK | ST_OVR)) |						// keep flags
				((*operand > mainCPU.Y) ? ST_CRY : 0) |									// carry flag
				((*operand == mainCPU.Y) ? ST_ZRO : 0) |								// zero flag
				(((mainCPU.Y - *operand) & 0x80) >> (7 - ST_NEG_BIT));					// negative (sign) flag
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATUS FLAGS

		case 0x18:
			// CLC (clear carry)
			mainCPU.P &= ~ST_CRY;
			break;

		case 0x38:
			// SEC (set carry)
			mainCPU.P |= ST_CRY;
			break;

		case 0x58:
			// CLI (clear interrupt)
			// TODO : Delayed IRQ response behavior?
			mainCPU.P &= ~ST_INT;
			break;

		case 0x78:
			// SEI (set interrupt)
			mainCPU.P |= ST_INT;
			break;

		case 0xB8:
			// CLV (clear overflow)
			mainCPU.P &= ~ST_OVR;
			break;

		case 0xD8:
			// CLD (clear decimal)
			mainCPU.P &= ~ST_BCD;
			break;

		case 0xF8:
			// SED (set decimal)
			mainCPU.P |= ST_BCD;
			break;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MISC

		case 0x00:
			// BRK
			mainCPU.PC++;
			cpu6502_SoftwareInterrupt(0xFFFE);
			break;

		case 0xEA:
			// NOP
			break;

		default:
			// unhandled instruction
			DebugAssert(false);
	}

	if (isWrite) {
		// DebugAssert((result & 0xFF) == result && effAddr != 0xFFFFFFFF);
		if (effAddr >= 0x2000) {
			mainCPU.writeSpecial(effAddr, result);
		} else {
			*operand = result;
		}
	} else if (isSpecial) {
		DebugAssert(effAddr != 0xFFFFFFFF);
		mainCPU.postSpecialRead(effAddr);
	}
}

void cpu6502_DeviceInterrupt(unsigned int vectorAddress, bool masked) {
	if (!masked || (mainCPU.P & ST_INT) == 0) {
		// interrupts are enabled

		// push PC and P (without BRK) onto stack
		mainCPU.push(mainCPU.PC >> 8);
		mainCPU.push(mainCPU.PC & 0xFF);
		mainCPU.push((mainCPU.P & ~ST_BRK) | ST_UNUSED);

		// switch PC to vector address and disable interrupts 
		mainCPU.PC = mainCPU.read(vectorAddress) | (mainCPU.read(vectorAddress+1) << 8);
		mainCPU.P |= ST_INT;

		// whole process takes 7 clocks
		mainCPU.clocks += 7;
	}
}

void cpu6502_SoftwareInterrupt(unsigned int vectorAddress) {
	// push PC and P (with BRK) onto stack
	mainCPU.push(mainCPU.PC >> 8);
	mainCPU.push(mainCPU.PC & 0xFF);
	mainCPU.push(mainCPU.P | ST_BRK | ST_UNUSED);

	// switch PC to vector address and disable interrupts 
	mainCPU.PC = mainCPU.read(vectorAddress) | (mainCPU.read(vectorAddress+1) << 8);
	mainCPU.P |= ST_INT;

	// whole process takes 7 clocks
	mainCPU.clocks += 7;
}

void cpu_instr_history::output() {
	// output is set up to match fceux for easy comparison

#if DEBUG
	static bool showClocks = true;
	if (showClocks) {
		OutputLog("c%-11d", regs.clocks);
	}
	static bool showRegs = true;
	if (showRegs) {
		OutputLog(" A:%02X X:%02X Y:%02X S:%02X P:%s%su%s%s%s%s%s  ",
			regs.A,
			regs.X,
			regs.Y,
			regs.SP,
			regs.P & ST_NEG ? "N" : "n",
			regs.P & ST_OVR ? "V" : "v",
			regs.P & ST_BRK ? "B" : "b",
			regs.P & ST_BCD ? "D" : "d",
			regs.P & ST_INT ? "I" : "i",
			regs.P & ST_ZRO ? "Z" : "z",
			regs.P & ST_CRY ? "C" : "c");
	}

#define OPCODE_0W(op,str,clk,sz,spc) if (instr == op) OutputLog("$%04X:%02X        " str, regs.PC, instr);
#define OPCODE_1W(op,str,clk,sz,spc) if (instr == op) OutputLog("$%04X:%02X %02X     " str, regs.PC, instr, data1, data1);
#define OPCODE_2W(op,str,clk,sz,spc) if (instr == op) OutputLog("$%04X:%02X %02X %02X  " str, regs.PC, instr, data1, data2, data1 + (data2 << 8));
#define OPCODE_REL(op,str,clk,sz,spc) if (instr == op) OutputLog("$%04X:%02X %02X     " str, regs.PC, instr, data1, regs.PC+2+((signed char)data1));
#include "6502_opcodes.inl"
#endif

	if (modeTable[instr & 0x1F] != AM_None) {
		OutputLog(" = #$%02X", effByte);
	}
	OutputLog("\n");
}

#if TRACE_INSTRUCTIONS
void HitBreakpoint() {
	OutputLog("CPU Instruction Trace:\n");
	for (int i = 99; i >= 0; i--) {
		traceHistory[(traceNum - i + 100) % 100].output();
	}
	OutputLog("Hit breakpoint at %04x!\n", cpuBreakpoint);

#if TARGET_WINSIM
	DebugBreak();
#endif
}
#endif
