// Definitions for APU/PPU register dump format.

#pragma once

#pragma pack(push, 1)
struct RegDumpEntry
{
	uint32_t	clkDelta;	// Delta of previous CLK counter (CPU Core clock cycles) value at the time of accessing to the register
	uint8_t 	reg; 		// Register index + Flag (msb - 0: write, 1: read)
	uint8_t 	value;		// Written value. Not used for reading.
	uint16_t	padding;	// Not used (yet?)
};
#pragma pack(pop)

class RegDumper
{
	uint64_t SavedPHICounter;
	FILE* regLogFile;
	char regdump_target[32];
	bool first_access = true;

public:
	RegDumper(const char* target, uint64_t phi_counter_now, char* filename);
	~RegDumper();

	void LogRegRead(uint64_t phi_counter_now, uint8_t regnum);
	void LogRegWrite(uint64_t phi_counter_now, uint8_t regnum, uint8_t val);
	void Flush();
};
