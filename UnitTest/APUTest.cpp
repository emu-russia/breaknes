#include "pch.h"

// All tests are performed on the reference APU RP2A03G

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace APUSimUnitTest
{
	UnitTest::UnitTest(APUSim::Revision rev)
	{
		core = new M6502Core::M6502(true, true);
		apu = new APUSim::APU(core, rev);
	}

	UnitTest::~UnitTest()
	{
		delete apu;
		delete core;
	}

	/// <summary>
	/// Check the operation of the counters from common.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestCounters()
	{
		char text[0x100]{};
		TriState CLK = TriState::Zero;
		APUSim::CounterBit up_counter[8]{};
		uint8_t last_val;

		// In this test CLK=0 acts as the Keep phase

		for (size_t n = 0; n < 0x200; n++)
		{
			TriState carry = TriState::One;

			for (size_t cnt_bit = 0; cnt_bit < 8; cnt_bit++)
			{
				carry = up_counter[cnt_bit].sim(carry, TriState::Zero, TriState::Zero, CLK, NOT(CLK), TriState::Z);
			}

			uint8_t val = 0;
			for (size_t cnt_bit = 0; cnt_bit < 8; cnt_bit++)
			{
				val |= (up_counter[cnt_bit].get() == TriState::One ? 1 : 0) << cnt_bit;
			}

			if (CLK == TriState::Zero)
			{
				sprintf_s(text, sizeof(text), "%zd: 0x%02X %s\n", n / 2, val, carry == TriState::One ? "CARRY!" : "");
				Logger::WriteMessage(text);
				last_val = val;
			}

			// Check overflow

			if (CLK == TriState::One)
			{
				if (val == 0xff && carry != TriState::One)
					return false;
				if (val != 0xff && carry != TriState::Zero)
					return false;
			}

			CLK = NOT(CLK);
		}

		// The other counters are isomorphic and there is no point in testing them (rly?)

		return last_val == 0xff;
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

	/// <summary>
	/// Check the ACLK phase pattern.
	/// </summary>
	/// <returns></returns>
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

	/// <summary>
	/// Simulate one second and count how many times the LFO 1/2 signals are triggered.
	/// TODO: the number of nanoseconds executed is for some reason very different from 10^9 (shows 536870912 ns).
	/// </summary>
	/// <param name="mode">SoftCLK mode (0 - normal, 1 - extended)</param>
	/// <returns></returns>
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

		sprintf_s(text, sizeof(text), "LFO1 freq: ~%zd Hz\n", lfo1_counter);
		Logger::WriteMessage(text);

		sprintf_s(text, sizeof(text), "LFO2 freq: ~%zd Hz\n\n", lfo2_counter);
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

	/// <summary>
	/// Check the correctness of the length decoder with the reference values.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestLengthDecoder()
	{
		char text[0x100]{};
		size_t lc_out[32] = { 
			0x9, 0xfd, 0x13, 0x1, 0x27, 0x3, 0x4f, 0x5, 0x9f, 0x7, 0x3b, 0x9, 0xd, 0xb, 0x19, 0xd,
			0xb, 0xf, 0x17, 0x11, 0x2f, 0x13, 0x5f, 0x15, 0xbf, 0x17, 0x47, 0x19, 0xf, 0x1b, 0x1f, 0x1d,
		};

		APUSim::LengthCounter lc(apu);

		for (size_t n = 0; n < 32; n++)
		{
			apu->SetDBBit(3, (n >> 0) & 1 ? TriState::One : TriState::Zero);
			apu->SetDBBit(4, (n >> 1) & 1 ? TriState::One : TriState::Zero);
			apu->SetDBBit(5, (n >> 2) & 1 ? TriState::One : TriState::Zero);
			apu->SetDBBit(6, (n >> 3) & 1 ? TriState::One : TriState::Zero);
			apu->SetDBBit(7, (n >> 4) & 1 ? TriState::One : TriState::Zero);

			lc.sim_Decoder1();
			lc.sim_Decoder2();

			size_t dec_out = Pack(lc.LC);

			sprintf_s(text, sizeof(text), "dec %zd: 0x%02X\n", n, (uint8_t)dec_out);
			Logger::WriteMessage(text);

			if (dec_out != lc_out[n])
				return false;
		}

		return true;
	}

	/// <summary>
	/// The test repeats /HDL/Framework/Icarus/apu/length_conter.v
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestLengthCounter()
	{
		char text[0x100]{};
		TriState NoCount{};				// After the countdown is complete, the LC triggers this signal
		size_t hcycles = 512;
		bool trace_run = false;

		apu->wire.n_CLK = TriState::One;	// CLK = 0  (instead of the global CLK pad we use the output from it in the inverse polarity)
		apu->wire.RES = TriState::Zero;
		apu->wire.DMCINT = TriState::Zero;
		apu->wire.n_LFO2 = TriState::One;

		APUSim::LengthCounter lc(apu);

		// Load initial value

		apu->wire.W4015 = TriState::Zero;
		apu->wire.n_R4015 = TriState::One;
		apu->DB = 0b01001'001;		// The counter is set to 0x9, which becomes 0x7 after the decoder (the lsb is used to enable the LC).
		lc.sim(0, TriState::One, TriState::Zero, NoCount);

		sprintf_s(text, sizeof(text), "Initial LC Value: %d\n", lc.Debug_GetCnt());
		Logger::WriteMessage(text);

		if (lc.Debug_GetCnt() != 7)
			return false;

		// Enable LC and check that the current counter value is not broken

		apu->wire.W4015 = TriState::One;
		lc.sim(0, TriState::Zero, TriState::Zero, NoCount);
		apu->wire.W4015 = TriState::Zero;

		sprintf_s(text, sizeof(text), "LC Value after Enable: %d\n", lc.Debug_GetCnt());
		Logger::WriteMessage(text);

		if (lc.Debug_GetCnt() != 7)
			return false;

		// Start counting

		TriState prev_aclk = TriState::X;
		TriState prev_clk = TriState::X;

		for (size_t n = 0; n < hcycles; n++)
		{
			// Divider and ACLK simulation is required to follow the ACLK phase pattern

			apu->core_int->sim();

			apu->clkgen->sim();

			// We apply synthetic LFO2 control so that it triggers more often and the test finishes faster.

			if (IsNegedge(prev_aclk, apu->wire.ACLK))		// negedge ACLK
			{
				apu->wire.n_LFO2 = TriState::Zero;
			}
			if (IsPosedge(prev_clk, NOT(apu->wire.n_CLK)))	// posedge CLK
			{
				apu->wire.n_LFO2 = TriState::One;
			}

			lc.sim(0, TriState::Zero, TriState::One, NoCount);

			if (trace_run)
			{
				sprintf_s(text, sizeof(text), "LC Value: %d, NoCount: %d\n", lc.Debug_GetCnt(), ToByte(NoCount));
				Logger::WriteMessage(text);
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);

			prev_aclk = apu->wire.ACLK;
			prev_clk = NOT(apu->wire.n_CLK);
		}

		// Check after everything that the counter is 0xff and NoCount = 1

		if (lc.Debug_GetCnt() != 0xff)
			return false;

		if (NoCount != TriState::One)
			return false;

		return true;
	}

	/// <summary>
	/// Check the OAM DMA:
	/// - First check the DMA Unit in "quiet" mode - immediately after a reset. The OAM DMA Unit should be "silent" (NOSPR=1)
	/// - Then we simulate a write to register $4014 and check that the OAM DMA does its job (NOSPR=0; SPR_CPU and SPR_PPU are executed alternately).
	/// RUNDMC is always 0 during the test (DMC does not interfere with the process).
	/// WARNING: We are NOT simulating anything outside the OAM DMA unit (CPU, memory, etc.). The purpose of the test is to verify that the OAM DMA Unit generates adequate signals
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestOAM_DMA()
	{
		char text[0x100]{};

		// Pretend DMC is sleeping

		apu->wire.RUNDMC = TriState::Zero;
		apu->wire.DMCRDY = TriState::One;

		// Spare parts are needed to implement the ACLK phase pattern. OAM DMA does not really depend on anything else

		// Idle Mode during Reset
		// -----------------------

		Logger::WriteMessage("--- Idle Mode during Reset ---\n");

		size_t idle_hcycles = 256;			// Number of cycles for Idle mode

		apu->wire.n_CLK = TriState::One;	// CLK = 0  (instead of the global CLK pad we use the output from it in the inverse polarity)
		apu->wire.RES = TriState::One;			// Global reset
		apu->wire.W4014 = TriState::Zero;
		apu->wire.RnW = TriState::Zero;

		TriState prev_phi = TriState::X;
		TriState prev_aclk = TriState::X;

		size_t aclk_counter = 0;
		size_t phi_counter = 0;

		for (size_t n = 0; n < idle_hcycles; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			// Clock stats

			if (IsNegedge(prev_aclk, apu->wire.ACLK))
			{
				// TBD: For what?
				aclk_counter++;
			}
			if (IsPosedge(prev_phi, apu->wire.PHI0))
			{
				// CPU cycles are displayed so that you can understand at which cycle something went wrong

				sprintf_s(text, sizeof(text), "Core cycle: %zd\n", phi_counter);
				Logger::WriteMessage(text);
				phi_counter++;
			}

			apu->dma->sim();

			// Check
			// NOSPR should be equal to 1.
			// SPRS should be equal to 0 (always).
			// SPR_CPU and SPR_PPU signals should be "silent"
			// RDY should be 1

			if (apu->dma->NOSPR != TriState::One)
				return false;
			if (apu->dma->SPRS != TriState::Zero)
				return false;
			if (apu->wire.SPR_CPU != TriState::Zero)
				return false;
			if (apu->wire.SPR_PPU != TriState::Zero)
				return false;
			if (apu->wire.RDY != TriState::One)
				return false;

			// Tick

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			prev_aclk = apu->wire.ACLK;
			prev_phi = apu->wire.PHI0;
		}

		// OAM DMA Idle after reset
		// -----------------------

		Logger::WriteMessage("--- OAM DMA Idle after reset ---\n");

		apu->wire.RES = TriState::Zero;

		prev_phi = TriState::X;
		prev_aclk = TriState::X;

		aclk_counter = 0;
		phi_counter = 0;

		for (size_t n = 0; n < idle_hcycles; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			// Clock stats

			if (IsNegedge(prev_aclk, apu->wire.ACLK))
			{
				// TBD: For what?
				aclk_counter++;
			}
			if (IsPosedge(prev_phi, apu->wire.PHI0))
			{
				// CPU cycles are displayed so that you can understand at which cycle something went wrong

				sprintf_s(text, sizeof(text), "Core cycle: %zd\n", phi_counter);
				Logger::WriteMessage(text);
				phi_counter++;
			}

			apu->dma->sim();

			// Check
			// NOSPR should be equal to 1.
			// SPRS should be equal to 0 (always).
			// SPR_CPU and SPR_PPU signals should be "silent"
			// RDY should be 1

			if (apu->dma->NOSPR != TriState::One)
				return false;
			if (apu->dma->SPRS != TriState::Zero)
				return false;
			if (apu->wire.SPR_CPU != TriState::Zero)
				return false;
			if (apu->wire.SPR_PPU != TriState::Zero)
				return false;
			if (apu->wire.RDY != TriState::One)
				return false;

			// Tick

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			prev_aclk = apu->wire.ACLK;
			prev_phi = apu->wire.PHI0;
		}

		// OAM DMA Active
		// -----------------------

		Logger::WriteMessage("--- OAM DMA Active (some cycles) ---\n");

		size_t dma_hcycles = 256;			// Number of cycles to simulate the OAM DMA process (there is no point in doing a full OAM DMA, only the first few iterations are enough)

		apu->wire.RES = TriState::Zero;
		prev_phi = TriState::X;
		prev_aclk = TriState::X;

		size_t clk_counter = 0;			// CLK half-cycle counter inside a single CPU cycle

		int dma_step = -2;			// A special counter that counts the OAM DMA Unit step. Each step of the OAM DMA has its own special magic

		// Simulate writing to register $4014 and skip the various alignment tricks.
		// At the beginning of the work cycle OAM DMA must be in the "Read Memory" state

		bool Write_4014_notyet = true;
		bool Hold_4014_Write = false;		// Hold W4014 until the end of the CPU cycle

		for (size_t n = 0; n < dma_hcycles; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			// Clock stats

			if (IsNegedge(prev_aclk, apu->wire.ACLK))
			{
				Hold_4014_Write = false;
				aclk_counter++;
			}
			if (IsNegedge(prev_phi, apu->wire.PHI0))
			{
				Logger::WriteMessage("\nNew CPU Cycle!\n");
				phi_counter++;
				dma_step++;
				if (dma_step >= 2)
				{
					dma_step = 0;
				}
				clk_counter = 0;
				Hold_4014_Write = false;
			}

			// For simplicity, we will simulate an aligned write in $4014 (ACLK=1 at the start of the write cycle in $4014)

			if ((Hold_4014_Write && apu->wire.PHI2 == TriState::One) || (Write_4014_notyet && apu->wire.PHI2 == TriState::One && prev_aclk == TriState::One))
			{
				apu->wire.W4014 = TriState::One;
				apu->wire.RnW = TriState::Zero;
				Write_4014_notyet = false;
				Hold_4014_Write = true;
			}
			else
			{
				apu->wire.W4014 = TriState::Zero;
				apu->wire.RnW = TriState::One;
			}

			apu->dma->sim();

			// Dump OAM DMA State

			sprintf_s(text, sizeof(text), 
				"ACLK: %d, PHI1: %d, W4014: %d, SPRS: %d, SPRE: %d, NOSPR: %d, DOSPR: %d, StartDMA: %d, StopDMA: %d, SPR_CPU: %d, SPR_PPU: %d, OAM Addr: 0x%x\n",
				FromByte(apu->wire.ACLK),
				FromByte(apu->wire.PHI1),
				FromByte(apu->wire.W4014),
				FromByte(apu->dma->SPRS),
				FromByte(apu->dma->SPRE),
				FromByte(apu->dma->NOSPR),
				FromByte(apu->dma->DOSPR),
				FromByte(apu->dma->StartDMA.get()),
				FromByte(apu->dma->StopDMA.get()),
				FromByte(apu->wire.SPR_CPU),
				FromByte(apu->wire.SPR_PPU),
				apu->dma->Get_DMAAddress());
			Logger::WriteMessage(text);

			// Check DMA in progress

			if (!(Hold_4014_Write && apu->wire.PHI2 == TriState::One) && !Write_4014_notyet)
			{
				switch (dma_step)
				{
					// OAM DMA "Think" cycle:
					case -1:
						if (apu->dma->SPRS != TriState::Zero) {
							Logger::WriteMessage("1\n");
							return false;
						}
						if (apu->dma->SPRE != TriState::Zero) {
							Logger::WriteMessage("2\n");
							return false;
						}
						if (apu->dma->DOSPR != TriState::Zero && apu->wire.PHI1 == TriState::One) {
							Logger::WriteMessage("3\n");
							return false;
						}
						if (apu->dma->DOSPR != TriState::One && apu->wire.PHI1 == TriState::Zero && clk_counter != 0) {
							Logger::WriteMessage("4\n");
							return false;
						}
						if (apu->dma->NOSPR != TriState::One) {
							Logger::WriteMessage("5\n");
							return false;
						}
						if (apu->dma->StartDMA.get() != TriState::One) {
							Logger::WriteMessage("6\n");
							return false;
						}
						if (apu->dma->StopDMA.get() != TriState::Zero && apu->wire.PHI1 == TriState::One) {
							Logger::WriteMessage("7\n");
							return false;
						}
						if (apu->dma->StopDMA.get() != TriState::One && apu->wire.PHI1 == TriState::Zero && clk_counter != 0) {
							Logger::WriteMessage("8\n");
							return false;
						}
						if (apu->wire.SPR_CPU != TriState::Zero) {
							Logger::WriteMessage("9\n");
							return false;
						}
						if (apu->wire.SPR_PPU != TriState::Zero) {
							Logger::WriteMessage("10\n");
							return false;
						}
						break;
					
					// Read CPU mem
					case 0:
						if (apu->dma->SPRS != TriState::Zero) {
							Logger::WriteMessage("21\n");
							return false;
						}
						if (apu->dma->SPRE != TriState::Zero) {
							Logger::WriteMessage("22\n");
							return false;
						}
						if (apu->dma->NOSPR != TriState::Zero && apu->wire.PHI1 == TriState::Zero) {		// OAM DMA in progress
							Logger::WriteMessage("23\n");
							return false;
						}
						if (apu->dma->StartDMA.get() != TriState::Zero && clk_counter != 0) {
							Logger::WriteMessage("24\n");
							return false;
						}
						if (apu->dma->StopDMA.get() != TriState::One) {
							Logger::WriteMessage("25\n");
							return false;
						}
						if (apu->wire.SPR_CPU != TriState::One && clk_counter != 0) {		// Read CPU Mem
							Logger::WriteMessage("26\n");
							return false;
						}
						if (apu->wire.SPR_PPU != TriState::Zero) {
							Logger::WriteMessage("27\n");
							return false;
						}
						break;

					// Write $2004
					case 1:
						if (apu->dma->SPRS != TriState::One && apu->wire.PHI1 == TriState::One) {			// OAM DMA addr++
							Logger::WriteMessage("31\n");
							return false;
						}
						if (apu->dma->SPRS != TriState::Zero && apu->wire.PHI1 == TriState::Zero) {
							Logger::WriteMessage("32\n");
							return false;
						}
						if (apu->dma->SPRE != TriState::Zero) {
							Logger::WriteMessage("33\n");
							return false;
						}
						if (apu->dma->NOSPR != TriState::Zero) {			// OAM DMA in progress
							Logger::WriteMessage("34\n");
							return false;
						}
						if (apu->dma->StartDMA.get() != TriState::Zero) {
							Logger::WriteMessage("35\n");
							return false;
						}
						if (apu->dma->StopDMA.get() != TriState::One) {
							Logger::WriteMessage("36\n");
							return false;
						}
						if (apu->wire.SPR_CPU != TriState::Zero) {
							Logger::WriteMessage("37\n");
							return false;
						}
						if (apu->wire.SPR_PPU != TriState::One) {				// Write $2004
							Logger::WriteMessage("38\n");
							return false;
						}
						break;
				}
			}

			// Tick

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			clk_counter++;
			prev_aclk = apu->wire.ACLK;
			prev_phi = apu->wire.PHI0;
		}

		return true;
	}

	bool UnitTest::MegaCyclesTest(size_t desired_clk)
	{
		char text[0x100]{};
		TriState CLK = TriState::Zero;

		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;
		APUSim::AudioOutSignal aux{};

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;

		// Reset APU

		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::Zero;

		for (size_t n = 0; n < 256; n++)
		{
			data_bus = 0;
			inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
			apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
			CLK = NOT(CLK);

			data_bus = 0;
			inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
			apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
			CLK = NOT(CLK);
		}

		// Continue

		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;

		auto stamp1 = GetTickCount64();

		for (size_t n = 0; n < desired_clk; n++)
		{
			data_bus = 0;
			inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
			apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
			CLK = NOT(CLK);

			data_bus = 0;
			inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
			apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
			CLK = NOT(CLK);
		}

		auto stamp2 = GetTickCount64();
		int delta = (int)(stamp2 - stamp1);

		sprintf_s(text, sizeof(text), "APU+Core executed %zd cycles in real %d msec\n", desired_clk, delta);
		Logger::WriteMessage(text);

		if (delta > 1000)
		{
			sprintf_s(text, sizeof(text), "You're %.2f times slower :(\n", (float)delta / 1000.f);
			Logger::WriteMessage(text);
		}

		return delta <= 1000;
	}

}
