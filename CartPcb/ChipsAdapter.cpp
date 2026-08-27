// Adapter between CartPcb and the chip simulators (Chips component).

#include "ChipsAdapter.h"

// Mapper chip implementations (moved from Mappers to Chips, issue #509)
// #include "../../Chips/MMC1/MMC1.h"

namespace CartPcb
{
	ChipInstance* CreateChipInstance(const std::string& type)
	{
		// if (type == "MMC1") return new Chips::MMC1ChipInstance();
		// Add new chips here.

		(void)type;
		return nullptr;
	}
}
