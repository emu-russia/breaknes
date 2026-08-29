// Adapter between CartPcb and the chip simulators (Chips component).

#include "ChipsAdapter.h"

#include "../Chips/MMC1/MMC1.h"

namespace CartPcb
{
	namespace
	{
		class MMC1ChipInstance : public ChipInstance
		{
			Chips::MMC1 mmc;
			ChipDesc desc;

		public:
			MMC1ChipInstance()
			{
				desc.type = "MMC1";

				for (size_t n = 0; n < (size_t)Chips::MMC1_Input::Max; n++)
				{
					ChipPin pin;
					pin.name = Chips::MMC1_InputPinNames[n];
					pin.output = false;
					desc.pins.push_back(pin);
				}

				for (size_t n = 0; n < (size_t)Chips::MMC1_Output::Max; n++)
				{
					ChipPin pin;
					pin.name = Chips::MMC1_OutputPinNames[n];
					pin.output = true;
					desc.pins.push_back(pin);
				}
			}

			void sim(BaseLogic::TriState* inputs, BaseLogic::TriState* outputs) override
			{
				mmc.sim(inputs, outputs);
			}

			const ChipDesc* GetDesc() const override
			{
				return &desc;
			}

			size_t Dbg_GetPRGAddress(size_t cpu_addr) override
			{
				return mmc.Dbg_GetPRGAddress(cpu_addr);
			}

			void SetLogMask(uint64_t mask) override
			{
				mmc.SetLogMask(mask);
			}
		};
	}

	ChipInstance* CreateChipInstance(const std::string& type)
	{
		if (type == "MMC1")
		{
			return new MMC1ChipInstance();
		}

		// Add new chips here.

		return nullptr;
	}
}
