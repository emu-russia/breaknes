// Nintendulator-compatible CPU instruction trace log.
// The disassembly tables and the trace line format are ported from the Nintendulator
// emulator debugger (Debugger.cpp, DecodeInstruction / AddInst), see
// https://github.com/quietust/nintendulator

#include "pch.h"

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "NintendulatorLog.h"
#include "BoardLog.h"

namespace
{
	enum AddrMode
	{
		IMP, ACC, IMM, ADR, ABS, IND, REL, ABX, ABY, ZPG, ZPX, ZPY, INX, INY, ERR, NUM_ADDR_MODES
	};

	// Addressing mode of each opcode.
	const AddrMode TraceAddrMode[256] =
	{
		IMM, INX, ERR, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		ADR, INX, ERR, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		IMP, INX, ERR, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ADR, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		IMP, INX, ERR, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, IND, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		IMM, INX, IMM, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
		IMM, INX, IMM, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
		IMM, INX, IMM, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		IMM, INX, IMM, INX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS, REL, INY, ERR, INY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX
	};

	// Number of bytes each addressing mode occupies in memory.
	const uint8_t AddrBytes[NUM_ADDR_MODES] = { 1, 1, 2, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2, 1 };

	// Mnemonics (same names as Nintendulator, including the '*' prefix of the unofficial opcodes).
	const char* const TraceNames[256] =
	{
		"BRK", "ORA", "*HLT", "*SLO", "*NOP", "ORA", "ASL", "*SLO", "PHP", "ORA", "ASL", "*AAC", "*NOP", "ORA", "ASL", "*SLO",
		"BPL", "ORA", "*HLT", "*SLO", "*NOP", "ORA", "ASL", "*SLO", "CLC", "ORA", "*NOP", "*SLO", "*NOP", "ORA", "ASL", "*SLO",
		"JSR", "AND", "*HLT", "*RLA", "BIT", "AND", "ROL", "*RLA", "PLP", "AND", "ROL", "*AAC", "BIT", "AND", "ROL", "*RLA",
		"BMI", "AND", "*HLT", "*RLA", "*NOP", "AND", "ROL", "*RLA", "SEC", "AND", "*NOP", "*RLA", "*NOP", "AND", "ROL", "*RLA",
		"RTI", "EOR", "*HLT", "*SRE", "*NOP", "EOR", "LSR", "*SRE", "PHA", "EOR", "LSR", "*ASR", "JMP", "EOR", "LSR", "*SRE",
		"BVC", "EOR", "*HLT", "*SRE", "*NOP", "EOR", "LSR", "*SRE", "CLI", "EOR", "*NOP", "*SRE", "*NOP", "EOR", "LSR", "*SRE",
		"RTS", "ADC", "*HLT", "*RRA", "*NOP", "ADC", "ROR", "*RRA", "PLA", "ADC", "ROR", "*ARR", "JMP", "ADC", "ROR", "*RRA",
		"BVS", "ADC", "*HLT", "*RRA", "*NOP", "ADC", "ROR", "*RRA", "SEI", "ADC", "*NOP", "*RRA", "*NOP", "ADC", "ROR", "*RRA",
		"*NOP", "STA", "*NOP", "*SAX", "STY", "STA", "STX", "*SAX", "DEY", "*NOP", "TXA", "???", "STY", "STA", "STX", "*SAX",
		"BCC", "STA", "*HLT", "???", "STY", "STA", "STX", "*SAX", "TYA", "STA", "TXS", "???", "???", "STA", "???", "???",
		"LDY", "LDA", "LDX", "*LAX", "LDY", "LDA", "LDX", "*LAX", "TAY", "LDA", "TAX", "*ATX", "LDY", "LDA", "LDX", "*LAX",
		"BCS", "LDA", "*HLT", "*LAX", "LDY", "LDA", "LDX", "*LAX", "CLV", "LDA", "TSX", "???", "LDY", "LDA", "LDX", "*LAX",
		"CPY", "CMP", "*NOP", "*DCP", "CPY", "CMP", "DEC", "*DCP", "INY", "CMP", "DEX", "*AXS", "CPY", "CMP", "DEC", "*DCP",
		"BNE", "CMP", "*HLT", "*DCP", "*NOP", "CMP", "DEC", "*DCP", "CLD", "CMP", "*NOP", "*DCP", "*NOP", "CMP", "DEC", "*DCP",
		"CPX", "SBC", "*NOP", "*ISB", "CPX", "SBC", "INC", "*ISB", "INX", "SBC", "NOP", "*SBC", "CPX", "SBC", "INC", "*ISB",
		"BEQ", "SBC", "*HLT", "*ISB", "*NOP", "SBC", "INC", "*ISB", "SED", "SBC", "*NOP", "*ISB", "*NOP", "SBC", "INC", "*ISB"
	};

	const int InstructionFieldWidth = 48;
}

NintendulatorLog::NintendulatorLog()
{
}

NintendulatorLog::~NintendulatorLog()
{
	Disable();
}

void NintendulatorLog::SetReadByteCallback(ReadByteFn fn, void* opaque)
{
	read_byte = fn;
	read_opaque = opaque;
}

void NintendulatorLog::Enable()
{
	if (enabled)
		return;

	log_file = fopen("Nintendulator.log", "w");
	enabled = (log_file != nullptr);

	if (enabled)
	{
		LOG_BOARD(Breaknes::Cat_Events, "Nintendulator log enabled, writing to Nintendulator.log");
	}
	else
	{
		LOG_BOARD(Breaknes::Cat_Events, "Nintendulator log: cannot open Nintendulator.log!");
	}
}

void NintendulatorLog::Disable()
{
	if (log_file != nullptr)
	{
		fflush(log_file);
		fclose(log_file);
		log_file = nullptr;
	}

	if (enabled)
	{
		LOG_BOARD(Breaknes::Cat_Events, "Nintendulator log disabled");
	}

	enabled = false;
}

bool NintendulatorLog::Enabled() const
{
	return enabled;
}

uint8_t NintendulatorLog::ReadMem(uint16_t addr)
{
	if (read_byte != nullptr)
	{
		return read_byte(read_opaque, addr);
	}

	return 0;
}

void NintendulatorLog::LogInstruction(const Entry& entry)
{
	if (!enabled || log_file == nullptr)
		return;

	char line[160];
	FormatInstruction(line, sizeof(line), entry);

	fwrite(line, 1, strlen(line), log_file);
}

void NintendulatorLog::FormatInstruction(char* buf, size_t buf_size, const Entry& entry)
{
	const uint16_t addr = entry.pc;

	uint8_t opdata[3] = { ReadMem(addr), 0, 0 };
	AddrMode mode = TraceAddrMode[opdata[0]];

	if (AddrBytes[mode] >= 2)
		opdata[1] = ReadMem((uint16_t)(addr + 1));
	if (AddrBytes[mode] >= 3)
		opdata[2] = ReadMem((uint16_t)(addr + 2));

	uint16_t operand = 0;
	uint16_t mid_addr = 0;
	uint16_t effective_addr = 0;

	// Calculate the operand / effective address the same way Nintendulator does.
	switch (mode)
	{
		case IND:
			operand = (uint16_t)(opdata[1] | (opdata[2] << 8));
			mid_addr = (uint16_t)((operand & 0xFF00) | ((operand + 1) & 0xFF));	// JMP indirect does not handle page crossing correctly
			effective_addr = (uint16_t)(ReadMem(operand) | (ReadMem(mid_addr) << 8));
			break;

		case ADR:
			operand = (uint16_t)(opdata[1] | (opdata[2] << 8));
			break;

		case ABS:
			operand = (uint16_t)(opdata[1] | (opdata[2] << 8));
			effective_addr = operand;
			break;

		case ABX:
			operand = (uint16_t)(opdata[1] | (opdata[2] << 8));
			effective_addr = (uint16_t)(operand + entry.x);
			break;

		case ABY:
			operand = (uint16_t)(opdata[1] | (opdata[2] << 8));
			effective_addr = (uint16_t)(operand + entry.y);
			break;

		case IMM:
			operand = opdata[1];
			break;

		case ZPG:
			operand = opdata[1];
			effective_addr = operand;
			break;

		case ZPX:
			operand = opdata[1];
			effective_addr = (uint16_t)((operand + entry.x) & 0xFF);
			break;

		case ZPY:
			operand = opdata[1];
			effective_addr = (uint16_t)((operand + entry.y) & 0xFF);
			break;

		case INX:
			operand = opdata[1];
			mid_addr = (uint16_t)((operand + entry.x) & 0xFF);
			effective_addr = (uint16_t)(ReadMem(mid_addr) | (ReadMem((uint16_t)((mid_addr + 1) & 0xFF)) << 8));
			break;

		case INY:
			operand = opdata[1];
			mid_addr = (uint16_t)(ReadMem(operand) | (ReadMem((uint16_t)((operand + 1) & 0xFF)) << 8));
			effective_addr = (uint16_t)(mid_addr + entry.y);
			break;

		case REL:
			operand = (uint16_t)(addr + 2 + (int8_t)opdata[1]);
			break;

		default:
			break;
	}

	char text[InstructionFieldWidth + 64];
	int len = 0;

	// Address
	len += sprintf(text + len, "%04X  ", (int)addr);

	// Instruction bytes
	if (AddrBytes[mode] == 1)
	{
		len += sprintf(text + len, "%02X", (int)opdata[0]);
	}
	else if (AddrBytes[mode] == 2)
	{
		len += sprintf(text + len, "%02X %02X", (int)opdata[0], (int)opdata[1]);
	}
	else
	{
		len += sprintf(text + len, "%02X %02X %02X", (int)opdata[0], (int)opdata[1], (int)opdata[2]);
	}

	// Pad to the mnemonic column
	while (len < 16)
		text[len++] = ' ';

	// Mnemonic
	len += sprintf(text + len, "%s", TraceNames[opdata[0]]);

	// Operand and memory access hints
	switch (mode)
	{
		case IMP:
		case ERR:
			break;

		case ACC:
			len += sprintf(text + len, " A");
			break;

		case IMM:
			len += sprintf(text + len, " #$%02X", (int)opdata[1]);
			break;

		case REL:
			len += sprintf(text + len, " $%04X", (int)operand);
			break;

		case ZPG:
			len += sprintf(text + len, " $%02X = %02X", (int)operand, (int)ReadMem(operand));
			break;

		case ZPX:
			len += sprintf(text + len, " $%02X,X @ %02X = %02X", (int)operand, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		case ZPY:
			len += sprintf(text + len, " $%02X,Y @ %02X = %02X", (int)operand, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		case INX:
			len += sprintf(text + len, " ($%02X,X) @ %02X = %04X = %02X", (int)operand, (int)mid_addr, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		case INY:
			len += sprintf(text + len, " ($%02X),Y = %04X @ %04X = %02X", (int)operand, (int)mid_addr, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		case ADR:
			len += sprintf(text + len, " $%04X", (int)operand);
			break;

		case ABS:
			len += sprintf(text + len, " $%04X = %02X", (int)operand, (int)ReadMem(operand));
			break;

		case IND:
			len += sprintf(text + len, " ($%04X) = %04X", (int)operand, (int)effective_addr);
			break;

		case ABX:
			len += sprintf(text + len, " $%04X,X @ %04X = %02X", (int)operand, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		case ABY:
			len += sprintf(text + len, " $%04X,Y @ %04X = %02X", (int)operand, (int)effective_addr, (int)ReadMem(effective_addr));
			break;

		default:
			break;
	}

	// Pad the instruction field so the register part starts at a fixed column
	while (len < InstructionFieldWidth)
		text[len++] = ' ';
	text[len] = 0;

	snprintf(buf, buf_size, "%sA:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3zu,%3zu CYC:%zu\n",
		text,
		(int)entry.a, (int)entry.x, (int)entry.y, (int)entry.p, (int)entry.s,
		entry.h_counter, entry.v_counter, entry.phi_counter);
}
