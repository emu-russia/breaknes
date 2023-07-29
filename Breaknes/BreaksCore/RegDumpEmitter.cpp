#include "pch.h"

RegDumper::RegDumper(const char* target, uint64_t phi_counter_now, char* filename)
{
	regLogFile = fopen(filename, "wb");
	SavedPHICounter = phi_counter_now;
	strcpy(regdump_target, target);
}

RegDumper::~RegDumper()
{
	fflush(regLogFile);
	fclose(regLogFile);
}

void RegDumper::LogRegRead(uint64_t phi_counter_now, uint8_t regnum, size_t ppu_h, size_t ppu_v)
{
	if (regLogFile != nullptr)
	{
		RegDumpEntry entry{};

		if (first_access) {
			printf("First %s access, clk_counter: 0x%llx\n", regdump_target, phi_counter_now);
			first_access = false;
		}

		uint64_t delta = phi_counter_now - SavedPHICounter;
		if (delta > 0xffffffff)
		{
			printf("Clock Delta exceeds the allowable limit. Read skipped.\n");
			SavedPHICounter = phi_counter_now;
			return;
		}

		entry.clkDelta = (uint32_t)delta;
		entry.reg = regnum | 0x80;
		entry.value = 0;
		entry.padding = 0;
		entry.ppu_h = (uint16_t)ppu_h;
		entry.ppu_v = (uint16_t)ppu_v;
		entry.reserved_1 = 0;
		entry.reserved_2 = 0;

		SavedPHICounter = phi_counter_now;

		fwrite(&entry, 1, sizeof(entry), regLogFile);
	}
}

void RegDumper::LogRegWrite(uint64_t phi_counter_now, uint8_t regnum, uint8_t val, size_t ppu_h, size_t ppu_v)
{
	if (regLogFile != nullptr)
	{
		RegDumpEntry entry{};

		if (first_access) {
			printf("First %s access, clk_counter: 0x%llx\n", regdump_target, phi_counter_now);
			first_access = false;
		}

		uint64_t delta = phi_counter_now - SavedPHICounter;
		if (delta > 0xffffffff)
		{
			printf("Clock Delta exceeds the allowable limit. Write skipped.\n");
			SavedPHICounter = phi_counter_now;
			return;
		}

		entry.clkDelta = (uint32_t)delta;
		entry.reg = regnum;
		entry.value = val;
		entry.padding = 0;
		entry.ppu_h = (uint16_t)ppu_h;
		entry.ppu_v = (uint16_t)ppu_v;
		entry.reserved_1 = 0;
		entry.reserved_2 = 0;

		SavedPHICounter = phi_counter_now;

		fwrite(&entry, 1, sizeof(entry), regLogFile);
	}
}

void RegDumper::Flush()
{
	if (regLogFile != nullptr) {
		fflush(regLogFile);
	}
}
