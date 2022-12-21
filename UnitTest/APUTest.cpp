#include "pch.h"

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace APUSimUnitTest
{
	UnitTest::UnitTest(APUSim::Revision rev)
	{
		core = new M6502Core::M6502(false, true);
		apu = new APUSim::APU(core, rev);
	}

	UnitTest::~UnitTest()
	{
		delete apu;
		delete core;
	}

	/// <summary>
	/// Run few full cycles and see what happens. The success of the test is checked by the M2 Duty Cycle.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDiv(bool trace)
	{
		char text[0x100]{};
		TriState CLK = TriState::Zero;
		size_t m2_duty = 0;
		size_t run_hcycles = 32LL * 2;
		float Expected_duty_cycle = 2.f / 3.f;		// approx 66%, but not exact

		apu->wire.RDY = TriState::Zero;

		for (size_t n = 0; n < run_hcycles; n++)
		{
			apu->wire.n_CLK = NOT(CLK);

			apu->core_int->sim();

			if (trace)
			{
				sprintf_s(text, sizeof(text), "CLK: %d, PHI0: %d, #M2: %d\n",
					BaseLogic::ToByte(CLK), BaseLogic::ToByte(apu->wire.PHI0), BaseLogic::ToByte(apu->wire.n_M2));
				Logger::WriteMessage(text);
			}

			if (apu->wire.n_M2 == TriState::Zero)
			{
				m2_duty++;
			}

			CLK = NOT(CLK);
		}

		float duty = (float)m2_duty / (float)run_hcycles;
		float duty_trunc = truncf(duty * 10) / 10;
		float Expected_duty_cycle_trunc = truncf(Expected_duty_cycle * 10) / 10;

		sprintf_s(text, sizeof(text), "M2 duty: %f (trunc: %f), expected: %f (trunc: %f)\n", 
			duty, duty_trunc, Expected_duty_cycle, Expected_duty_cycle_trunc);
		Logger::WriteMessage(text);

		return duty_trunc == Expected_duty_cycle_trunc;
	}

	bool UnitTest::TestAclk()
	{
		const size_t hcycles = 12 * 4;
		char clk_dump[hcycles + 1] = { 0 };
		char phi_dump[hcycles + 1] = { 0 };
		char aclk_dump[hcycles + 1] = { 0 };
		char naclk_dump[hcycles + 1] = { 0 };
		int aclk_integral = 0;
		int naclk_integral = 0;

		apu->wire.n_CLK = TriState::One;	// CLK = 0
		apu->wire.RES = TriState::Zero;

		for (size_t n = 0; n < hcycles; n++)
		{
			apu->core_int->sim();

			apu->clkgen->sim_ACLK();

			clk_dump[n] = NOT(apu->wire.n_CLK) == TriState::One ? '1' : '0';
			phi_dump[n] = (apu->wire.PHI0) == TriState::One ? '1' : '0';
			aclk_dump[n] = (apu->wire.ACLK) == TriState::One ? '1' : '0';
			naclk_dump[n] = (apu->wire.n_ACLK) == TriState::One ? '1' : '0';

			if (apu->wire.ACLK == TriState::One)
				aclk_integral++;
			else
				aclk_integral--;

			if (apu->wire.n_ACLK == TriState::One)
				naclk_integral++;
			else
				naclk_integral--;

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
		}

		Logger::WriteMessage(("CLK  :" + std::string(clk_dump) + "\n").c_str());
		Logger::WriteMessage(("PHI  :" + std::string(phi_dump) + "\n").c_str());
		Logger::WriteMessage(("ACLK :" + std::string(aclk_dump) + "\n").c_str());
		Logger::WriteMessage(("#ACLK:" + std::string(naclk_dump) + "\n").c_str());

		if (aclk_integral != (36 - 12))
			return false;

		if (naclk_integral != (-12 + 12 - 24))
			return false;

		return true;
	}

	bool UnitTest::TestLFO(bool mode)
	{
		char text[0x100]{};
		const size_t osc = 21477272;	// Hz
		const size_t hcycles = osc * 2;	// 1 sumulated second
		TriState prev_nLFO1 = TriState::X;
		TriState prev_nLFO2 = TriState::X;
		const float ns_in_second = 1000000000.f;
		const float hcycle_len = ns_in_second / (float)osc / 2.f;  // in nanoseconds
		float t = 0.f;
		size_t lfo1_counter = 0;
		size_t lfo2_counter = 0;
		bool trace = false;

		apu->wire.n_CLK = TriState::One;	// CLK = 0
		apu->wire.RES = TriState::Zero;
		apu->wire.DMCINT = TriState::Zero;

		apu->clkgen->reg_mode.sim(TriState::Zero, TriState::One, mode ? TriState::One : TriState::Zero);
		apu->clkgen->reg_mask.sim(TriState::Zero, TriState::One, TriState::Zero); // 0 - int enable

		sprintf_s(text, sizeof(text), "Test LFO in Mode: %d\n", mode ? 1 : 0);
		Logger::WriteMessage(text);

		for (size_t n = 0; n < hcycles; n++)
		{
			apu->core_int->sim();

			apu->clkgen->sim();

			// Detect the negedge for LFO 1/2

			if (apu->wire.n_LFO1 != prev_nLFO1)
			{
				if (prev_nLFO1 == TriState::One && apu->wire.n_LFO1 == TriState::Zero)
				{
					if (trace)
					{
						sprintf_s(text, sizeof(text), "LFO1: %f ns\n", t);
						Logger::WriteMessage(text);
					}

					lfo1_counter++;
				}

				prev_nLFO1 = apu->wire.n_LFO1;
			}

			if (apu->wire.n_LFO2 != prev_nLFO2)
			{
				if (prev_nLFO2 == TriState::One && apu->wire.n_LFO2 == TriState::Zero)
				{
					if (trace)
					{
						sprintf_s(text, sizeof(text), "LFO2: %f ns\n", t);
						Logger::WriteMessage(text);
					}

					lfo2_counter++;
				}

				prev_nLFO2 = apu->wire.n_LFO2;
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			t += hcycle_len;
		}

		sprintf_s(text, sizeof(text), "Executed %f ns. CLK: %f ns\n", t, hcycle_len * 2.f);
		Logger::WriteMessage(text);

		sprintf_s(text, sizeof(text), "LFO1 freq: %zd Hz\n", lfo1_counter);
		Logger::WriteMessage(text);

		sprintf_s(text, sizeof(text), "LFO2 freq: %zd Hz\n\n", lfo2_counter);
		Logger::WriteMessage(text);

		return true;
	}

	bool UnitTest::VerifyRegOpByAddress(uint16_t addr, bool read)
	{
		switch (addr)
		{
			case 0x4000:
				if ((apu->wire.W4000 != TriState::One && !read))
					return false;
				break;
			case 0x4001:
				if ((apu->wire.W4001 != TriState::One && !read))
					return false;
				break;
			case 0x4002:
				if ((apu->wire.W4002 != TriState::One && !read))
					return false;
				break;
			case 0x4003:
				if ((apu->wire.W4003 != TriState::One && !read))
					return false;
				break;
			case 0x4004:
				if ((apu->wire.W4004 != TriState::One && !read))
					return false;
				break;
			case 0x4005:
				if ((apu->wire.W4005 != TriState::One && !read))
					return false;
				break;
			case 0x4006:
				if ((apu->wire.W4006 != TriState::One && !read))
					return false;
				break;
			case 0x4007:
				if ((apu->wire.W4007 != TriState::One && !read))
					return false;
				break;
			case 0x4008:
				if ((apu->wire.W4008 != TriState::One && !read))
					return false;
				break;
			case 0x400a:
				if ((apu->wire.W400A != TriState::One && !read))
					return false;
				break;
			case 0x400b:
				if ((apu->wire.W400B != TriState::One && !read))
					return false;
				break;
			case 0x400c:
				if ((apu->wire.W400C != TriState::One && !read))
					return false;
				break;
			case 0x400e:
				if ((apu->wire.W400E != TriState::One && !read))
					return false;
				break;
			case 0x400f:
				if ((apu->wire.W400F != TriState::One && !read))
					return false;
				break;
			case 0x4010:
				if ((apu->wire.W4010 != TriState::One && !read))
					return false;
				break;
			case 0x4011:
				if ((apu->wire.W4011 != TriState::One && !read))
					return false;
				break;
			case 0x4012:
				if ((apu->wire.W4012 != TriState::One && !read))
					return false;
				break;
			case 0x4013:
				if ((apu->wire.W4013 != TriState::One && !read))
					return false;
				break;
			case 0x4014:
				if ((apu->wire.W4014 != TriState::One && !read))
					return false;
				break;
			case 0x4015:
				if ((apu->wire.W4015 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4015 != TriState::Zero && read))
					return false;
				break;
			case 0x4016:
				if ((apu->wire.W4016 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4016 != TriState::Zero && read))
					return false;
				break;
			case 0x4017:
				if ((apu->wire.W4017 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4017 != TriState::Zero && read))
					return false;
				break;
			case 0x4018:
				if ((apu->wire.n_R4018 != TriState::Zero && read))
					return false;
				break;
			case 0x4019:
				if ((apu->wire.n_R4019 != TriState::Zero && read))
					return false;
				break;
			case 0x401a:
				if ((apu->wire.W401A != TriState::One && !read))
					return false;
				if ((apu->wire.n_R401A != TriState::Zero && read))
					return false;
				break;

			default:
				break;
		}

		return true;
	}

	/// <summary>
	/// Check that register operations are as expected (mapped to the correct addresses).
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestRegOps()
	{
		apu->wire.DBG = TriState::One;
		apu->wire.PHI1 = TriState::Zero;
		
		// Read
		
		apu->Ax = 0x4000;
		apu->CPU_Addr = 0x4000;
		apu->wire.RnW = TriState::One;

		for (size_t n = 0; n < 0x20; n++)
		{
			apu->regs->sim();

			if (!VerifyRegOpByAddress(apu->Ax, true))
			{
				Logger::WriteMessage(("RegOp failed! Idx: " + std::to_string(n) + ", Read\n").c_str());
				return false;
			}

			apu->Ax++;
			apu->CPU_Addr++;
		}

		// Write

		apu->Ax = 0x4000;
		apu->CPU_Addr = 0x4000;
		apu->wire.RnW = TriState::Zero;

		for (size_t n = 0; n < 0x20; n++)
		{
			apu->regs->sim();

			if (!VerifyRegOpByAddress(apu->Ax, false))
			{
				Logger::WriteMessage(("RegOp failed! Idx: " + std::to_string(n) + ", Write\n").c_str());
				return false;
			}

			apu->Ax++;
			apu->CPU_Addr++;
		}

		return true;
	}
}
