// Adapter between CartPcb and the chip simulators (Chips component and
// BaseBoardLib glue logic).

#include "ChipsAdapter.h"

#include "../Chips/MMC1/MMC1.h"
#include "../Common/BaseBoardLib/LS161.h"
#include "../Common/BaseBoardLib/LS32.h"

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

		// 74LS161 (BaseBoardLib): 4-bit binary counter used as the bank register
		// on UxROM/AxROM boards. Pin order of the ChipInstance interface follows
		// the sim() signature: inputs CLK, nRST, nLD, EN_T, EN_P, P0..P3;
		// outputs RCO, Q0..Q3 (Q0 = LSB).

		class LS161ChipInstance : public ChipInstance
		{
			BaseBoard::LS161 ls161;
			ChipDesc desc;

			enum In
			{
				In_CLK = 0,
				In_nRST,
				In_nLD,
				In_EN_T,
				In_EN_P,
				In_P0,
				In_P1,
				In_P2,
				In_P3,
				In_Max,
			};

			enum Out
			{
				Out_RCO = 0,
				Out_Q0,
				Out_Q1,
				Out_Q2,
				Out_Q3,
				Out_Max,
			};

		public:
			LS161ChipInstance()
			{
				desc.type = "LS161";

				static const char* const inputNames[] = { "CLK", "nRST", "nLD", "EN_T", "EN_P", "P0", "P1", "P2", "P3" };
				static const char* const outputNames[] = { "RCO", "Q0", "Q1", "Q2", "Q3" };

				for (size_t n = 0; n < In_Max; n++)
				{
					ChipPin pin;
					pin.name = inputNames[n];
					pin.output = false;
					desc.pins.push_back(pin);
				}

				for (size_t n = 0; n < Out_Max; n++)
				{
					ChipPin pin;
					pin.name = outputNames[n];
					pin.output = true;
					desc.pins.push_back(pin);
				}
			}

			void sim(BaseLogic::TriState* inputs, BaseLogic::TriState* outputs) override
			{
				BaseLogic::TriState P[4]
				{
					inputs[In_P0], inputs[In_P1], inputs[In_P2], inputs[In_P3]
				};

				BaseLogic::TriState Q[4]{};
				BaseLogic::TriState RCO = BaseLogic::TriState::Z;

				ls161.sim(
					inputs[In_CLK],
					inputs[In_nRST],
					inputs[In_nLD],
					inputs[In_EN_T],
					inputs[In_EN_P],
					P,
					RCO,
					Q);

				outputs[Out_RCO] = RCO;
				outputs[Out_Q0] = Q[0];
				outputs[Out_Q1] = Q[1];
				outputs[Out_Q2] = Q[2];
				outputs[Out_Q3] = Q[3];
			}

			const ChipDesc* GetDesc() const override
			{
				return &desc;
			}
		};

		// 74LS32 (BaseBoardLib): quad 2-input OR gate. On UNROM boards it is the
		// address multiplexer that combines the bank-register outputs with CPU
		// A14 (see Nescartdb/boards/unrom.json). Inputs A0..A3, B0..B3;
		// outputs Y0..Y3.

		class LS32ChipInstance : public ChipInstance
		{
			ChipDesc desc;

			enum In
			{
				In_A0 = 0,
				In_A1,
				In_A2,
				In_A3,
				In_B0,
				In_B1,
				In_B2,
				In_B3,
				In_Max,
			};

			enum Out
			{
				Out_Y0 = 0,
				Out_Y1,
				Out_Y2,
				Out_Y3,
				Out_Max,
			};

		public:
			LS32ChipInstance()
			{
				desc.type = "LS32";

				static const char* const inputNames[] = { "A0", "A1", "A2", "A3", "B0", "B1", "B2", "B3" };
				static const char* const outputNames[] = { "Y0", "Y1", "Y2", "Y3" };

				for (size_t n = 0; n < In_Max; n++)
				{
					ChipPin pin;
					pin.name = inputNames[n];
					pin.output = false;
					desc.pins.push_back(pin);
				}

				for (size_t n = 0; n < Out_Max; n++)
				{
					ChipPin pin;
					pin.name = outputNames[n];
					pin.output = true;
					desc.pins.push_back(pin);
				}
			}

			void sim(BaseLogic::TriState* inputs, BaseLogic::TriState* outputs) override
			{
				BaseLogic::TriState A[4]
				{
					inputs[In_A0], inputs[In_A1], inputs[In_A2], inputs[In_A3]
				};

				BaseLogic::TriState B[4]
				{
					inputs[In_B0], inputs[In_B1], inputs[In_B2], inputs[In_B3]
				};

				BaseLogic::TriState Y[4]{};

				BaseBoard::LS32::sim(A, B, Y);

				outputs[Out_Y0] = Y[0];
				outputs[Out_Y1] = Y[1];
				outputs[Out_Y2] = Y[2];
				outputs[Out_Y3] = Y[3];
			}

			const ChipDesc* GetDesc() const override
			{
				return &desc;
			}
		};
	}

	ChipInstance* CreateChipInstance(const std::string& type)
	{
		if (type == "MMC1")
		{
			return new MMC1ChipInstance();
		}

		if (type == "LS161")
		{
			return new LS161ChipInstance();
		}

		if (type == "LS32")
		{
			return new LS32ChipInstance();
		}

		// Add new chips here.

		return nullptr;
	}
}
