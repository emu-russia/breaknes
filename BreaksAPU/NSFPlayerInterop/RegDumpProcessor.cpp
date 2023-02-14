#include "pch.h"

using namespace BaseLogic;

namespace NSFPlayer
{
	RegDumpProcessor::RegDumpProcessor()
	{
	}

	RegDumpProcessor::~RegDumpProcessor()
	{
		if (regdump != nullptr)
		{
			delete[] regdump;
		}
	}

	void RegDumpProcessor::sim(TriState PHI0, TriState& RnW, uint16_t* addr_bus, uint8_t* data_bus)
	{
		// Increase the cycle counter

		if (IsNegedge(PrevPHI0, PHI0))
		{
			phi_counter++;
		}

		// If regdump is loaded and the cycle counter has reached the value for the next RegOp - execute

		if (regdump != nullptr)
		{
			if (phi_counter >= next_phi && PHI0 == TriState::One)
			{
				APULogEntry* current = GetCurrentEntry();

				RnW = (current->reg & 0x80) ? TriState::One : TriState::Zero;
				if (RnW == TriState::Zero)
				{
					*data_bus = current->value;
				}
				*addr_bus = 0x4000 | (current->reg & 0x1f);

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
					APULogEntry* next = GetCurrentEntry();
					next_phi = phi_counter + next->phiDelta;
				}
			}
		}

		PrevPHI0 = PHI0;
	}

	void RegDumpProcessor::SetRegDump(void* ptr, size_t size)
	{
		if (size < sizeof(APULogEntry))
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
		regdump_entry_max = regdump_size / sizeof(APULogEntry);

		APULogEntry* first_entry = (APULogEntry*)ptr;
		next_phi = first_entry->phiDelta;

		phi_counter = 0;
		PrevPHI0 = TriState::X;
	}

	APULogEntry* RegDumpProcessor::GetCurrentEntry()
	{
		return (APULogEntry*)(regdump + regdump_entry * sizeof(APULogEntry));
	}
}
