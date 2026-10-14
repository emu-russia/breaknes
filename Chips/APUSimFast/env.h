// Envelope Unit
// Shared between the square channels and the noise generator.

#pragma once

namespace APUSimFast
{
	class EnvelopeUnit
	{
		friend APUSimUnitTest::UnitTest;
		APU* apu = nullptr;

		RegisterBit envdis_reg{};
		RegisterBit lc_reg{};
		uint8_t vol_reg = 0;
		PackedCounter<4, false> decay_cnt{};
		PackedCounter<4, false> env_cnt{};
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

		void Debug_Get(uint32_t& VolumeReg, uint32_t& DecayCounter, uint32_t& EnvCounter);

		uint32_t Debug_Get_VolumeReg();
		uint32_t Debug_Get_DecayCounter();
		uint32_t Debug_Get_EnvCounter();

		void Debug_Set_VolumeReg(uint32_t val);
		void Debug_Set_DecayCounter(uint32_t val);
		void Debug_Set_EnvCounter(uint32_t val);
	};
}
