#include "pch.h"

RegDumper::RegDumper(uint64_t phi_counter_now, char* filename)
{
	regLogFile = fopen(filename, "wb");
	SavedPHICounter = phi_counter_now;
}

RegDumper::~RegDumper()
{
	fflush(regLogFile);
	fclose(regLogFile);
}

void RegDumper::LogRegRead(uint64_t phi_counter_now, uint8_t regnum)
{
	if (regLogFile != nullptr)
	{
		RegDumpEntry entry{};

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

		SavedPHICounter = phi_counter_now;

		fwrite(&entry, 1, sizeof(entry), regLogFile);
	}
}

void RegDumper::LogRegWrite(uint64_t phi_counter_now, uint8_t regnum, uint8_t val)
{
	if (regLogFile != nullptr)
	{
		RegDumpEntry entry{};

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
