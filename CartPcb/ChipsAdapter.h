// Adapter between CartPcb and the chip simulators (Chips component).
//
// CartPcb only simulates PCB wiring; mapper chips (MMC1, ...) are implemented
// in Chips and are injected into a Pcb through the ChipInstance interface.

#pragma once

#include <string>

#include "Pcb.h"

namespace CartPcb
{
	/// <summary>
	/// Create a chip instance by its type name (e.g. "MMC1").
	/// Returns nullptr for unknown chip types.
	/// </summary>
	ChipInstance* CreateChipInstance(const std::string& type);
}
