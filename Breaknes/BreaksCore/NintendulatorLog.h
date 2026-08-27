// Nintendulator-compatible CPU instruction trace log.

#pragma once

#include <cstdio>
#include <cstdint>
#include <cstddef>

/// <summary>
/// Produces one log line per executed 6502 instruction in the format used by the
/// Nintendulator emulator, e.g.:
///
///		FFF2  A9 00     LDA #$00                        A:00 X:00 Y:00 P:24 SP:FD PPU:  0,  0 CYC:7
///		FFF4  8D B3 FF  STA $FFB3 = 00                  A:00 X:00 Y:00 P:26 SP:FD PPU:  6,  0 CYC:9
///
/// The log is written to the "Nintendulator.log" file in the working directory.
/// </summary>
class NintendulatorLog
{
public:
	/// <summary>
	/// Side-effect-free CPU memory read callback, used to disassemble the instruction
	/// bytes and to compute the memory access hints.
	/// </summary>
	typedef uint8_t(*ReadByteFn)(void* opaque, uint16_t addr);

	/// <summary>
	/// One instruction trace entry, captured at the opcode fetch cycle (before the
	/// instruction is executed).
	/// </summary>
	struct Entry
	{
		uint16_t pc;			// Instruction address
		uint8_t a;				// Accumulator
		uint8_t x;				// X register
		uint8_t y;				// Y register
		uint8_t s;				// Stack pointer
		uint8_t p;				// Processor status register (NV1BDIZC, the B bit is always 0, as in Nintendulator)
		size_t phi_counter;		// CPU (PHI2) cycle counter -> CYC
		size_t h_counter;		// PPU H counter
		size_t v_counter;		// PPU V counter
	};

	NintendulatorLog();
	~NintendulatorLog();

	void SetReadByteCallback(ReadByteFn fn, void* opaque);

	/// <summary>
	/// Open the log file ("Nintendulator.log", previous content is discarded).
	/// </summary>
	void Enable();

	/// <summary>
	/// Close the log file.
	/// </summary>
	void Disable();

	bool Enabled() const;

	/// <summary>
	/// Write one instruction trace line to the log file.
	/// </summary>
	void LogInstruction(const Entry& entry);

private:
	FILE* log_file = nullptr;
	bool enabled = false;
	ReadByteFn read_byte = nullptr;
	void* read_opaque = nullptr;

	uint8_t ReadMem(uint16_t addr);
	void FormatInstruction(char* buf, size_t buf_size, const Entry& entry);
};
