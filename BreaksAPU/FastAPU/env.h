// Envelope Unit
// Shared between the square channels and the noise generator.

#pragma once

namespace FastAPU
{
	class EnvelopeUnit
	{
		FastAPU* apu = nullptr;

		int envdis_reg{};
		int lc_reg{};
		uint8_t vol_reg{};
		uint8_t decay_cnt{};
		uint8_t env_cnt{};
		BaseLogic::FF EnvReload{};
		BaseLogic::DLatch erld_latch{};
		BaseLogic::DLatch reload_latch{};
		BaseLogic::DLatch rco_latch{};
		BaseLogic::DLatch eco_latch{};

	public:
		EnvelopeUnit(FastAPU* parent);
		~EnvelopeUnit();

		void sim(BaseLogic::TriState V[4], BaseLogic::TriState WR_Reg, BaseLogic::TriState WR_LC);
		BaseLogic::TriState get_LC();
	};
}
