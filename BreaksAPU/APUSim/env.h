// Envelope Unit
// Shared between the square channels and the noise generator.

#pragma once

namespace APUSim
{
	class EnvelopeUnit
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		RegisterBit envdis_reg{};
		RegisterBit lc_reg{};
		RegisterBit vol_reg[4]{};
		DownCounterBit decay_cnt[4]{};
		DownCounterBit env_cnt[4]{};
		BaseLogic::FF EnvReload{};
		BaseLogic::DLatch erld_latch{};
		BaseLogic::DLatch reload_latch{};
		BaseLogic::DLatch rco_latch{};
		BaseLogic::DLatch eco_latch{};

	public:
		EnvelopeUnit(APU* parent);
		~EnvelopeUnit();

		void sim(BaseLogic::TriState V[4], BaseLogic::TriState WR_Reg, BaseLogic::TriState WR_LC);
		BaseLogic::TriState get_LC();
	};
}
