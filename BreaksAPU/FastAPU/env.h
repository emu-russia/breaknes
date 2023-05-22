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
		int EnvReload{};
		int erld_latch{};
		int reload_latch{};
		int rco_latch{};
		int eco_latch{};

	public:
		EnvelopeUnit(FastAPU* parent);
		~EnvelopeUnit();

		void sim(uint8_t V, int WR_Reg, int WR_LC);
		int get_LC();
	};
}
