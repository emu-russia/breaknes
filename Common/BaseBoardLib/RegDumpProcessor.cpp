#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	RegDumpProcessor::RegDumpProcessor(uint16_t regs_base, uint8_t regs_mask)
	{
		regbase = regs_base;
		regmask = regs_mask;
	}

	RegDumpProcessor::~RegDumpProcessor()
	{
		if (regdump != nullptr)
		{
			delete[] regdump;
		}
	}

	void RegDumpProcessor::sim(TriState CLK, TriState n_RES, TriState& RnW, uint16_t* addr_bus, uint8_t* data_bus)
	{
		if (n_RES == TriState::Zero)
		{
			return;
		}

		// Increase the cycle counter

		if (IsNegedge(PrevCLK, CLK))
		{
			clk_counter++;
		}

		// If regdump is loaded and the cycle counter has reached the value for the next RegOp - execute

		if (regdump != nullptr)
		{
			if (clk_counter >= next_clk && CLK == TriState::One)
			{
				RegDumpEntry* current = GetCurrentEntry();

				RnW = (current->reg & 0x80) ? TriState::One : TriState::Zero;
				if (RnW == TriState::Zero)
				{
					*data_bus = current->value;
				}
				*addr_bus = regbase | (current->reg & regmask);

				// If the record is the last one - delete regdump

				if (regdump_entry >= regdump_entry_max)
				{
					delete[] regdump;
					regdump = nullptr;
				}

				// Otherwise, switch to the next record and wait until the cycle counter reaches the desired value

				else
				{
					regdump_entry++;
					RegDumpEntry* next = GetCurrentEntry();
					next_clk = clk_counter + next->clkDelta;
				}
			}
		}

		PrevCLK = CLK;
	}

	void RegDumpProcessor::SetRegDump(void* ptr, size_t size)
	{
		if (size < sizeof(RegDumpEntry))
			return;

		// Delete the old regdump

		if (regdump != nullptr)
		{
			delete[] regdump;
			regdump = nullptr;
		}

		// Copy regdump to a new buffer

		regdump = new uint8_t[size];
		memcpy(regdump, ptr, size);
		regdump_size = size;

		// Set the initial values of the state logic

		regdump_entry = 0;
		regdump_entry_max = regdump_size / sizeof(RegDumpEntry);

		RegDumpEntry* first_entry = (RegDumpEntry*)ptr;
		next_clk = first_entry->clkDelta;

		clk_counter = 0;
		PrevCLK = TriState::X;
	}

	RegDumpEntry* RegDumpProcessor::GetCurrentEntry()
	{
		return (RegDumpEntry*)(regdump + regdump_entry * sizeof(RegDumpEntry));
	}
}
