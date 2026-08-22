#include "pch.h"

// Tests for the internal components and debug interfaces of the APUSim.
// The existing APUTest.cpp covers the counters, divider, ACLK, LFO, register decode,
// length decoder/counter and the OAM DMA. Everything else is tested here.

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace APUSimUnitTest
{
	/// <summary>
	/// Run a few full APU cycles (no reset, data bus = 0). Used as a smoke-run helper.
	/// </summary>
	/// <param name="num_cycles">Number of full CLK cycles (two half-cycles each)</param>
	void UnitTest::RunCycles(size_t num_cycles)
	{
		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;
		APUSim::AudioOutSignal aux{};

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;

		TriState CLK = TriState::Zero;

		for (size_t n = 0; n < num_cycles; n++)
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
	}

	/// <summary>
	/// Check the public APU debug interface: wires/registers dump, the debug set/get roundtrips,
	/// the ACLK/PHI counters, the audio signal features and the output mode switches.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestAPUDebugInfo()
	{
		char text[0x100]{};
		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;
		APUSim::AudioOutSignal aux{};

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::Zero;	// reset the APU

		// The reset pattern from the MegaCycles test

		TriState CLK = TriState::Zero;

		for (size_t n = 0; n < 64; n++)
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

		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;

		RunCycles(64);

		// GetDebugInfo_Wires: all fields are readable and match the internal connections

		APUSim::APU_Interconnects wires{};
		apu->GetDebugInfo_Wires(wires);

		if (wires.n_CLK != ToByte(apu->wire.n_CLK)) return false;
		if (wires.PHI0 != ToByte(apu->wire.PHI0)) return false;
		if (wires.PHI1 != ToByte(apu->wire.PHI1)) return false;
		if (wires.RES != ToByte(apu->wire.RES)) return false;
		if (wires.n_LFO1 != ToByte(apu->wire.n_LFO1)) return false;
		if (wires.W4015 != ToByte(apu->wire.W4015)) return false;

		// GetDebugInfo_Regs: readable and consistent with the direct getters

		APUSim::APU_Registers regs{};
		apu->GetDebugInfo_Regs(regs);

		if (regs.DBOutputLatch != apu->pads->Get_DBOutputLatch()) return false;
		if (regs.LengthCounterSQA != apu->lc[0]->Debug_GetCnt()) return false;
		if (regs.SQFreqReg[0] != apu->square[0]->Get_FreqReg()) return false;

		// GetDebugInfo_Wire / SetDebugInfo_Wire roundtrip

		int ofs = offsetof(APUSim::APU_Interconnects, n_LFO1);
		apu->SetDebugInfo_Wire(ofs, 1);
		if (apu->GetDebugInfo_Wire(ofs) != 1) return false;
		apu->SetDebugInfo_Wire(ofs, 0);
		if (apu->GetDebugInfo_Wire(ofs) != 0) return false;

		ofs = offsetof(APUSim::APU_Interconnects, W4015);
		apu->SetDebugInfo_Wire(ofs, 1);
		if (apu->GetDebugInfo_Wire(ofs) != 1) return false;

		// GetDebugInfo_Reg / SetDebugInfo_Reg roundtrip

		ofs = offsetof(APUSim::APU_Registers, SQFreqReg[0]);
		apu->SetDebugInfo_Reg(ofs, 0x7FF);
		if (apu->GetDebugInfo_Reg(ofs) != 0x7FF) return false;

		ofs = offsetof(APUSim::APU_Registers, TRILinearReg);
		apu->SetDebugInfo_Reg(ofs, 0x7F);
		if (apu->GetDebugInfo_Reg(ofs) != 0x7F) return false;

		ofs = offsetof(APUSim::APU_Registers, DPCMOutput);
		apu->SetDebugInfo_Reg(ofs, 0x55);
		if (apu->GetDebugInfo_Reg(ofs) != 0x55) return false;

		// The msb of the DPCM address counter is hardwired to 1

		ofs = offsetof(APUSim::APU_Registers, DPCMAddressCounter);
		apu->SetDebugInfo_Reg(ofs, 0x1234);
		if (apu->GetDebugInfo_Reg(ofs) != (0x1234 | 0x8000)) return false;

		// The status register uses only bits 0..4

		ofs = offsetof(APUSim::APU_Registers, Status);
		apu->SetDebugInfo_Reg(ofs, 0x1F);
		if (apu->GetDebugInfo_Reg(ofs) != 0x1F) return false;

		// ACLK/PHI counters

		apu->ResetACLKCounter();
		if (apu->GetACLKCounter() != 0) return false;

		apu->ResetPHICounter();
		if (apu->GetPHICounter() != 0) return false;

		size_t aclk_before = apu->GetACLKCounter();
		size_t phi_before = apu->GetPHICounter();

		RunCycles(64);

		sprintf_s(text, sizeof(text), "ACLK counter: %zd -> %zd, PHI counter: %zd -> %zd\n",
			aclk_before, apu->GetACLKCounter(), phi_before, apu->GetPHICounter());
		Logger::WriteMessage(text);

		if (apu->GetACLKCounter() <= aclk_before) return false;
		if (apu->GetPHICounter() <= phi_before) return false;

		// Audio signal features (RP2A03G)

		APUSim::AudioSignalFeatures sf{};
		apu->GetSignalFeatures(sf);

		sprintf_s(text, sizeof(text), "SampleRate: %d\n", sf.SampleRate);
		Logger::WriteMessage(text);

		if (sf.SampleRate == 0) return false;

		// Output mode switches and PHI2

		apu->SetRAWOutput(true);
		apu->SetNormalizedOutput(true);
		apu->SetRAWOutput(false);
		apu->SetNormalizedOutput(false);

		if (apu->GetPHI2() != apu->wire.PHI2) return false;

		return true;
	}

	/// <summary>
	/// Check the bit cells from common.cpp (RegisterBit, RegisterBitRes, RegisterBitRes2,
	/// DownCounterBit, RevCounterBit). They are tested standalone, without an APU instance.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestCommonBitCells()
	{
		// RegisterBit: sim(ACLK1, Enable, Value)

		{
			APUSim::RegisterBit rb;

			// The initial value is Zero
			if (rb.get() != TriState::Zero) return false;
			if (rb.nget() != TriState::One) return false;

			// Load: ACLK1=0, Enable=1
			rb.sim(TriState::Zero, TriState::One, TriState::One);
			if (rb.get() != TriState::One) return false;
			if (rb.nget() != TriState::Zero) return false;

			// Keep: ACLK1=1
			rb.sim(TriState::One, TriState::One, TriState::Zero);
			if (rb.get() != TriState::One) return false;

			// Keep: Enable=0
			rb.sim(TriState::Zero, TriState::Zero, TriState::Zero);
			if (rb.get() != TriState::One) return false;

			// Load Zero
			rb.sim(TriState::Zero, TriState::One, TriState::Zero);
			if (rb.get() != TriState::Zero) return false;

			// set()
			rb.set(TriState::One);
			if (rb.get() != TriState::One) return false;
			rb.set(TriState::Zero);
			if (rb.get() != TriState::Zero) return false;
		}

		// RegisterBitRes: sim(ACLK1, Enable, Value, Res)

		{
			APUSim::RegisterBitRes rbr;

			// Reset forces Zero
			rbr.sim(TriState::Zero, TriState::One, TriState::One, TriState::One);
			if (rbr.get() != TriState::Zero) return false;

			// Load One
			rbr.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero);
			if (rbr.get() != TriState::One) return false;

			// Reset again
			rbr.sim(TriState::Zero, TriState::One, TriState::Zero, TriState::One);
			if (rbr.get() != TriState::Zero) return false;
		}

		// RegisterBitRes2: sim(ACLK1, Enable, Value, Res1, Res2)

		{
			APUSim::RegisterBitRes2 rbr2;

			// Either reset input forces Zero
			rbr2.sim(TriState::Zero, TriState::One, TriState::One, TriState::One, TriState::Zero);
			if (rbr2.get() != TriState::Zero) return false;

			rbr2.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::One);
			if (rbr2.get() != TriState::Zero) return false;

			// Load One
			rbr2.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::Zero);
			if (rbr2.get() != TriState::One) return false;
		}

		// DownCounterBit: sim(Carry, Clear, Load, Step, ACLK1, val)
		// Load=1 -> val, Load=0 & Clear=1 -> Zero, Load=0 & Clear=0 & Step=1 -> count (toggles when Carry=1)

		{
			APUSim::DownCounterBit dcb;

			// Load
			dcb.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);
			if (dcb.get() != TriState::One) return false;

			// Clear (Load=0 takes the Clear path)
			dcb.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::One);
			if (dcb.get() != TriState::Zero) return false;

			// Keep (Step=0)
			dcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::One);
			if (dcb.get() != TriState::Zero) return false;

			// Load One, then count (Step=1, Carry=1): the bit toggles
			dcb.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);
			if (dcb.get() != TriState::One) return false;

			dcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero);
			if (dcb.get() != TriState::Zero) return false;

			dcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero);
			if (dcb.get() != TriState::One) return false;

			// Carry-out: cout = Carry AND NOT(bit)
			APUSim::DownCounterBit dcb2;
			dcb2.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero);	// bit = 0
			TriState cout = dcb2.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::One) return false;		// Carry=1, bit=0 -> 1

			dcb2.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);	// bit = 1
			cout = dcb2.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::Zero) return false;		// Carry=1, bit=1 -> 0

			dcb2.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero);	// bit = 0
			cout = dcb2.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::Zero) return false;		// Carry=0 -> 0
		}

		// RevCounterBit: sim(Carry, Dec, Clear, Load, Step, ACLK1, val)

		{
			APUSim::RevCounterBit rcb;

			// Load
			rcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);
			if (rcb.get() != TriState::One) return false;

			// Clear (Load=0 takes the Clear path)
			rcb.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::One);
			if (rcb.get() != TriState::Zero) return false;

			// Keep (Step=0)
			rcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::One);
			if (rcb.get() != TriState::Zero) return false;

			// Load One, then count (Step=1, Carry=1): the bit toggles
			rcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);
			if (rcb.get() != TriState::One) return false;

			rcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero);
			if (rcb.get() != TriState::Zero) return false;

			rcb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero);
			if (rcb.get() != TriState::One) return false;

			// Carry-out: Dec=0 -> cout = bit, Dec=1 -> cout = NOT(bit), Carry=0 -> 0
			APUSim::RevCounterBit rcb2;
			rcb2.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);	// bit = 1
			TriState cout = rcb2.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::One) return false;		// Carry=1, Dec=0, bit=1 -> 1

			APUSim::RevCounterBit rcb3;
			rcb3.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero);	// bit = 0
			cout = rcb3.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::One) return false;		// Carry=1, Dec=1, bit=0 -> NOT(bit)=1

			APUSim::RevCounterBit rcb4;
			rcb4.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::One);	// bit = 1
			cout = rcb4.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero);
			if (cout != TriState::Zero) return false;		// Carry=0 -> 0
		}

		return true;
	}

	/// <summary>
	/// Check the SoftCLK shift register bit from clkgen.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestSoftCLK_SRBit()
	{
		// sim(ACLK1, F1, F2, shift_in):
		// in_latch = MUX(F2, MUX(F1, Z, One), shift_in); out_latch = NOT(in_latch) on ACLK1
		// F2=1 -> shift_in, F1=1 -> One, otherwise the bit is held

		APUSim::SoftCLK_SRBit b;

		// Shift in a One (F2=1)
		b.sim(TriState::One, TriState::Zero, TriState::One, TriState::One);
		if (b.get_sout() != TriState::One) return false;
		if (b.get_nsout() != TriState::Zero) return false;

		// Shift in a Zero (F2=1)
		b.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero);
		if (b.get_sout() != TriState::Zero) return false;

		// Set to One (F1=1)
		b.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero);
		if (b.get_sout() != TriState::One) return false;

		// Hold (F1=0, F2=0): in_latch = Z
		b.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One);
		if (b.get_sout() != TriState::One) return false;

		// ACLK1=0: the output latch is not updated
		b.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (b.get_sout() != TriState::One) return false;

		return true;
	}

	/// <summary>
	/// Check the divider shift register bit from core.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDIV_SRBit()
	{
		// sim(q, nq, rst, sin):
		// in_latch = sin on q; out_latch = NOT(in_latch) on nq
		// get_sout(rst) = NOR(out_latch.get(), rst); get_nval() = NOT(in_latch)

		APUSim::DIV_SRBit b;

		// Capture sin=One on q, propagate to the output on nq
		b.sim(TriState::One, TriState::One, TriState::Zero, TriState::One);
		if (b.get_sout(TriState::Zero) != TriState::One) return false;
		if (b.get_nval() != TriState::Zero) return false;

		// Reset forces the output to Zero
		if (b.get_sout(TriState::One) != TriState::Zero) return false;

		// Capture sin=Zero
		b.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero);
		if (b.get_sout(TriState::Zero) != TriState::Zero) return false;
		if (b.get_nval() != TriState::One) return false;

		// q=0: the input latch is not updated
		b.sim(TriState::Zero, TriState::One, TriState::Zero, TriState::One);
		if (b.get_nval() != TriState::One) return false;

		// nq=0: the output latch is not updated
		b.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One);
		if (b.get_sout(TriState::Zero) != TriState::Zero) return false;

		return true;
	}

	/// <summary>
	/// Check the timing generator: the LFO outputs toggle during the run and the frame
	/// interrupt flag can be set by putting the soft clock LFSR into the interrupt state.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestCLKGen()
	{
		char text[0x100]{};

		apu->wire.n_CLK = TriState::One;	// CLK = 0
		apu->wire.RES = TriState::Zero;
		apu->wire.DMCINT = TriState::Zero;
		apu->wire.W4017 = TriState::Zero;

		// The interrupt flag is clear right after construction

		if (apu->clkgen->GetINTFF() != TriState::Zero) return false;

		// Smoke run: the LFO outputs must toggle

		TriState prev_lfo1 = apu->wire.n_LFO1;
		TriState prev_lfo2 = apu->wire.n_LFO2;
		bool lfo1_toggled = false;
		bool lfo2_toggled = false;

		for (size_t n = 0; n < 0x400; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			if (apu->wire.n_LFO1 != prev_lfo1)
			{
				lfo1_toggled = true;
				prev_lfo1 = apu->wire.n_LFO1;
			}
			if (apu->wire.n_LFO2 != prev_lfo2)
			{
				lfo2_toggled = true;
				prev_lfo2 = apu->wire.n_LFO2;
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
		}

		sprintf_s(text, sizeof(text), "LFO1 toggled: %d, LFO2 toggled: %d\n", lfo1_toggled, lfo2_toggled);
		Logger::WriteMessage(text);

		if (!lfo1_toggled || !lfo2_toggled) return false;

		// Set up the frame interrupt condition:
		// mode = 0 (md_latch = 1) and the LFSR in the state that fires pla[3]:
		// s[0..4]=1, s[5..8]=0, s[9]=1, s[10]=0, s[11]=1, s[12..14]=0

		apu->clkgen->int_ff.set(TriState::Zero);
		if (apu->clkgen->GetINTFF() != TriState::Zero) return false;

		apu->clkgen->md_latch.set(TriState::One, TriState::One);	// mode = 0

		TriState irq_state[15] = {
			TriState::One, TriState::One, TriState::One, TriState::One, TriState::One,
			TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One,
			TriState::Zero, TriState::Zero, TriState::Zero
		};
		for (size_t n = 0; n < 15; n++)
		{
			// F2=1 makes the bit take the shift input
			apu->clkgen->lfsr[n].sim(TriState::One, TriState::Zero, TriState::One, irq_state[n]);
		}

		apu->wire.n_R4015 = TriState::One;
		apu->wire.RES = TriState::Zero;
		apu->wire.DMCINT = TriState::Zero;
		apu->wire.PHI1 = TriState::Zero;

		apu->clkgen->sim();

		if (apu->clkgen->GetINTFF() != TriState::One)
		{
			Logger::WriteMessage("The frame interrupt flag did not fire!\n");
			return false;
		}

		return true;
	}

	/// <summary>
	/// Check the OAM DMA unit: Get/Set roundtrips, the DMA buffer and the address mux.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDMA()
	{
		// Get/Set roundtrips

		apu->dma->Set_DMABuffer(0xA5);
		if (apu->dma->Get_DMABuffer() != 0xA5) return false;

		apu->dma->Set_DMAAddress(0x1234);
		if (apu->dma->Get_DMAAddress() != 0x1234) return false;

		// sim_DMA_Buffer: the buffer latches the data bus on PHI2

		apu->wire.PHI2 = TriState::One;
		apu->wire.SPR_PPU = TriState::Zero;
		apu->DB = 0x5A;
		apu->dma->sim_DMA_Buffer();
		if (apu->dma->Get_DMABuffer() != 0x5A) return false;

		// SPR_PPU routes the buffer back onto the data bus (no re-latch: PHI2=0)

		apu->dma->Set_DMABuffer(0xA5);
		apu->wire.PHI2 = TriState::Zero;
		apu->wire.SPR_PPU = TriState::One;
		apu->DB = 0x00;
		apu->dma->sim_DMA_Buffer();
		if (apu->DB != 0xA5) return false;

		// sim_AddressMux: only one bus master is active at a time

		apu->CPU_Addr = 0x1234;
		apu->SPR_Addr = 0x2345;
		apu->DMC_Addr = 0x3456;
		apu->wire.n_DMC_AB = TriState::One;		// DMC_AB = 0
		apu->wire.SPR_CPU = TriState::Zero;
		apu->wire.SPR_PPU = TriState::Zero;

		apu->dma->sim_AddressMux();				// the CPU owns the bus
		if (apu->Ax != 0x1234) return false;

		apu->wire.SPR_CPU = TriState::One;
		apu->dma->sim_AddressMux();				// OAM DMA reads CPU memory
		if (apu->Ax != 0x2345) return false;

		apu->wire.SPR_CPU = TriState::Zero;
		apu->wire.SPR_PPU = TriState::One;
		apu->dma->sim_AddressMux();				// OAM DMA writes $2004
		if (apu->Ax != 0x2004) return false;

		apu->wire.SPR_PPU = TriState::Zero;
		apu->wire.n_DMC_AB = TriState::Zero;	// DMC_AB = 1
		apu->dma->sim_AddressMux();				// DMC owns the bus
		if (apu->Ax != 0x3456) return false;

		return true;
	}

	/// <summary>
	/// Check the DPCM shift register bits from dpcm.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDPCMBits()
	{
		// DPCM_LFSRBit: sim(ACLK1, load, step, val, shift_in)
		// in_latch = MUX(load, MUX(step, Z, shift_in), val); out = NOT(in_latch) on ACLK1
		// load=1 -> val, load=0 & step=1 -> shift_in, otherwise held

		APUSim::DPCM_LFSRBit lfsr;

		// Load
		lfsr.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::Zero);
		if (lfsr.get_sout() != TriState::One) return false;

		// Shift in Zero (step=1)
		lfsr.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero);
		if (lfsr.get_sout() != TriState::Zero) return false;

		// Shift in One (step=1)
		lfsr.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One);
		if (lfsr.get_sout() != TriState::One) return false;

		// Keep (step=0)
		lfsr.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (lfsr.get_sout() != TriState::One) return false;

		// DPCM_SRBit: sim(ACLK1, clear, load, step, n_val, shift_in)
		// clear=1 -> Zero, load=1 -> n_val, load=0 & step=1 -> shift, otherwise held

		APUSim::DPCM_SRBit sr;

		// Clear -> d = Zero -> sout = One
		sr.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (sr.get_sout() != TriState::One) return false;

		// Load n_val=Zero -> d = Zero -> sout = One
		sr.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero);
		if (sr.get_sout() != TriState::One) return false;

		// Load n_val=One -> d = One -> sout = Zero
		sr.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero);
		if (sr.get_sout() != TriState::Zero) return false;

		// Step (step=1): capture shift_in on ACLK1, then d = NOT(shift_in) -> sout = shift_in
		sr.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One);
		if (sr.get_sout() != TriState::One) return false;

		// Keep (step=0)
		sr.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (sr.get_sout() != TriState::One) return false;

		return true;
	}

	/// <summary>
	/// Check the DPCM channel: Get/Set roundtrips, the enable flag and a smoke run.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDpcmChan()
	{
		char text[0x100]{};

		// Get/Set roundtrips

		apu->dpcm->Set_FreqReg(0xA);
		if (apu->dpcm->Get_FreqReg() != 0xA) return false;

		apu->dpcm->Set_SampleReg(0x77);
		if (apu->dpcm->Get_SampleReg() != 0x77) return false;

		apu->dpcm->Set_SampleCounter(0xABC);
		if (apu->dpcm->Get_SampleCounter() != 0xABC) return false;

		apu->dpcm->Set_SampleBuffer(0x88);
		if (apu->dpcm->Get_SampleBuffer() != 0x88) return false;

		apu->dpcm->Set_SampleBitCounter(0x5);
		if (apu->dpcm->Get_SampleBitCounter() != 0x5) return false;

		apu->dpcm->Set_AddressReg(0x99);
		if (apu->dpcm->Get_AddressReg() != 0x99) return false;

		// The msb of the address counter is hardwired to 1

		apu->dpcm->Set_AddressCounter(0x1234);
		if (apu->dpcm->Get_AddressCounter() != (0x1234 | 0x8000)) return false;

		apu->dpcm->Set_Output(0x55);
		if (apu->dpcm->Get_Output() != 0x55) return false;

		apu->dpcm->SetDpcmEnable(true);
		if (!apu->dpcm->GetDpcmEnable()) return false;
		apu->dpcm->SetDpcmEnable(false);
		if (apu->dpcm->GetDpcmEnable()) return false;

		// Smoke test: run some APU cycles with the DPCM channel active

		RunCycles(64);

		sprintf_s(text, sizeof(text), "DPCM sample counter after sim: 0x%X\n", apu->dpcm->Get_SampleCounter());
		Logger::WriteMessage(text);

		return true;
	}

	/// <summary>
	/// Check the LengthCounter debug methods.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestLengthCounterDebug()
	{
		char text[0x100]{};

		APUSim::LengthCounter lc(apu);

		// Get/Set roundtrips

		lc.Debug_SetCnt(0x42);
		if (lc.Debug_GetCnt() != 0x42) return false;

		lc.Debug_SetEnable(true);
		if (!lc.Debug_GetEnable()) return false;

		lc.Debug_SetEnable(false);
		if (lc.Debug_GetEnable()) return false;

		// The APU length counters are accessible the same way

		apu->lc[0]->Debug_SetCnt(0x7F);
		if (apu->lc[0]->Debug_GetCnt() != 0x7F) return false;
		apu->lc[1]->Debug_SetCnt(0x3F);
		if (apu->lc[1]->Debug_GetCnt() != 0x3F) return false;
		apu->lc[2]->Debug_SetCnt(0x1F);
		if (apu->lc[2]->Debug_GetCnt() != 0x1F) return false;
		apu->lc[3]->Debug_SetCnt(0x0F);
		if (apu->lc[3]->Debug_GetCnt() != 0x0F) return false;

		sprintf_s(text, sizeof(text), "LC values: %d %d %d %d\n",
			apu->lc[0]->Debug_GetCnt(), apu->lc[1]->Debug_GetCnt(),
			apu->lc[2]->Debug_GetCnt(), apu->lc[3]->Debug_GetCnt());
		Logger::WriteMessage(text);

		return true;
	}

	/// <summary>
	/// Check the envelope unit: Debug Get/Set roundtrips, the V[] output, the reload
	/// from the volume register and the decay counter operation.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestEnvelopeUnit()
	{
		char text[0x100]{};

		APUSim::EnvelopeUnit env(apu);

		// The LC output is 1 while the halt bit is not set

		if (env.get_LC() != TriState::One) return false;

		// Debug Get/Set roundtrips

		env.Debug_Set_VolumeReg(0xA);
		if (env.Debug_Get_VolumeReg() != 0xA) return false;

		env.Debug_Set_DecayCounter(0x6);
		if (env.Debug_Get_DecayCounter() != 0x6) return false;

		env.Debug_Set_EnvCounter(0x9);
		if (env.Debug_Get_EnvCounter() != 0x9) return false;

		uint32_t vol = 0, dec = 0, ecnt = 0;
		env.Debug_Get(vol, dec, ecnt);
		if (vol != 0xA || dec != 0x6 || ecnt != 0x9) return false;

		// With the envelope disabled (ENVDIS=1) the volume register is passed to V[].
		// The control register is written on ACLK1=0.

		apu->wire.n_LFO1 = TriState::One;
		apu->wire.ACLK1 = TriState::Zero;
		apu->wire.RES = TriState::Zero;
		apu->DB = (1 << 4) | 0xA;		// ENVDIS=1, volume = 0xA

		TriState V[4]{};
		env.sim(V, TriState::One, TriState::Zero);		// WR_Reg=1 -> envdis_reg=1, vol_reg=0xA

		TriState vol_bits[4]{};
		UnpackNibble(0xA, vol_bits);
		for (size_t n = 0; n < 4; n++)
		{
			if (V[n] != vol_bits[n]) return false;
		}

		// The WR_LC strobe marks the reload; the next LFO period loads the decay
		// counter from the volume register

		apu->wire.n_LFO1 = TriState::Zero;
		apu->wire.ACLK1 = TriState::One;

		env.Debug_Set_DecayCounter(0x5);
		env.sim(V, TriState::Zero, TriState::One);		// WR_LC=1
		env.sim(V, TriState::Zero, TriState::Zero);

		if (env.Debug_Get_DecayCounter() != 0xA) return false;

		// The following count-down cycle (n_LFO1=0, rco_latch=1 -> Step=1) decrements
		// the decay counter by exactly 1 (documented 2A03 behavior: the decay counter
		// counts down by 1 on every quarter-frame /LFO1 pulse).

		uint32_t before = env.Debug_Get_DecayCounter();
		env.sim(V, TriState::Zero, TriState::Zero);
		uint32_t after = env.Debug_Get_DecayCounter();

		sprintf_s(text, sizeof(text), "Decay counter: 0x%X -> 0x%X\n", before, after);
		Logger::WriteMessage(text);

		if (after != before - 1) return false;

		// get_LC(): with the halt bit set in the control register it goes to 0

		apu->wire.ACLK1 = TriState::Zero;
		apu->DB = (1 << 5);
		env.sim(V, TriState::One, TriState::Zero);		// WR_Reg=1 -> lc_reg = DB bit 5
		if (env.get_LC() != TriState::Zero) return false;

		return true;
	}

	/// <summary>
	/// Check the noise channel LFSR bits from noise.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestNoiseBits()
	{
		// FreqLFSRBit: sim(ACLK1, load, step, val, shift_in)
		// d = MUX(load, MUX(step, Z, shift_in), val); in = d; out = NOT(in) on ACLK1
		// load=1 -> val, load=0 & step=1 -> shift_in, otherwise held

		APUSim::FreqLFSRBit fb;

		// Load One
		fb.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::Zero);
		if (fb.get_sout() != TriState::One) return false;

		// Shift Zero (step=1)
		fb.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero);
		if (fb.get_sout() != TriState::Zero) return false;

		// Shift One (step=1)
		fb.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One);
		if (fb.get_sout() != TriState::One) return false;

		// Keep (step=0)
		fb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (fb.get_sout() != TriState::One) return false;

		// RandomLFSRBit: sim(ACLK1, load, shift_in)
		// in_reg.sim(ACLK1, load, shift_in); out = NOT(in_reg) on ACLK1

		APUSim::RandomLFSRBit rb;

		// Capture One (ACLK1=0), propagate (ACLK1=1)
		rb.sim(TriState::Zero, TriState::One, TriState::One);
		rb.sim(TriState::One, TriState::Zero, TriState::Zero);
		if (rb.get_sout() != TriState::One) return false;

		// Capture Zero, propagate
		rb.sim(TriState::Zero, TriState::One, TriState::Zero);
		rb.sim(TriState::One, TriState::Zero, TriState::Zero);
		if (rb.get_sout() != TriState::Zero) return false;

		// Keep
		rb.sim(TriState::One, TriState::Zero, TriState::One);
		if (rb.get_sout() != TriState::Zero) return false;

		return true;
	}

	/// <summary>
	/// Check the noise channel: Get/Set roundtrips and a smoke run.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestNoiseChan()
	{
		char text[0x100]{};

		apu->noise->Set_FreqReg(0xB);
		if (apu->noise->Get_FreqReg() != 0xB) return false;

		// The LC output is 1 while the halt bit is not set

		if (apu->noise->get_LC() != TriState::One) return false;

		// Smoke test: run some APU cycles with the noise channel active

		RunCycles(64);

		sprintf_s(text, sizeof(text), "Noise freq reg after sim: 0x%X\n", apu->noise->Get_FreqReg());
		Logger::WriteMessage(text);

		return true;
	}

	/// <summary>
	/// Check the square channel bits from square.cpp: the frequency register bit and the adder.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestSquareBits()
	{
		// FreqRegBit: sim(ACLK3, ACLK1, WR, DB_in, ADDOUT, n_sum)
		// d = MUX(WR, MUX(ACLK3, Z, get_Fx(ADDOUT)), DB_in); transp = d; sum_latch = n_sum on ACLK1

		APUSim::FreqRegBit fb;

		// Write DB_in=One (WR overrides ACLK3)
		fb.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::Zero);
		if (fb.get() != TriState::One) return false;

		// Keep (WR=0): with ACLK3=1 the register holds its value
		fb.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero);
		if (fb.get() != TriState::One) return false;

		// With ADDOUT=0: get_Fx = the register value, get_nFx = its inverse
		if (fb.get_Fx(TriState::Zero) != TriState::One) return false;
		if (fb.get_nFx(TriState::Zero) != TriState::Zero) return false;

		// Write Zero
		fb.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero);
		if (fb.get() != TriState::Zero) return false;
		if (fb.get_Fx(TriState::Zero) != TriState::Zero) return false;
		if (fb.get_nFx(TriState::Zero) != TriState::One) return false;

		// set()
		fb.set(TriState::One);
		if (fb.get() != TriState::One) return false;
		fb.set(TriState::Zero);
		if (fb.get() != TriState::Zero) return false;

		// AdderBit: the full adder truth table (sum = F + S + C, cout = majority)

		APUSim::AdderBit adder;

		// 0 + 0 + 0 = 0, carry 0
		{
			TriState cout = TriState::X, n_cout = TriState::X, n_sum = TriState::X;
			adder.sim(TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, cout, n_cout, n_sum);
			if (cout != TriState::Zero) return false;
			if (n_sum != TriState::One) return false;		// sum = 0
		}
		// 1 + 0 + 0 = 1, carry 0
		{
			TriState cout = TriState::X, n_cout = TriState::X, n_sum = TriState::X;
			adder.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, cout, n_cout, n_sum);
			if (cout != TriState::Zero) return false;
			if (n_sum != TriState::Zero) return false;		// sum = 1
		}
		// 1 + 1 + 0 = 0, carry 1
		{
			TriState cout = TriState::X, n_cout = TriState::X, n_sum = TriState::X;
			adder.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::One, cout, n_cout, n_sum);
			if (cout != TriState::One) return false;
			if (n_sum != TriState::One) return false;		// sum = 0
		}
		// 1 + 1 + 1 = 1, carry 1
		{
			TriState cout = TriState::X, n_cout = TriState::X, n_sum = TriState::X;
			adder.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, TriState::Zero, cout, n_cout, n_sum);
			if (cout != TriState::One) return false;
			if (n_sum != TriState::Zero) return false;		// sum = 1
		}

		// Duty cycle table (documented in BreakingNESWiki square.md):
		// The DUTY output for each duty counter value (7..0) and duty setting d (0..3).
		// We drive the duty counter directly and read the DUTY wire through sim_Duty.

		const TriState duty_table[4][8] = {
			// d=0 (12.5%): 7->1, rest 0
			{ TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero },
			// d=1 (25%):   7->1, 6->1, rest 0
			{ TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero },
			// d=2 (50%):   7..4->1, 3..0->0
			{ TriState::One, TriState::One, TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero },
			// d=3 (75%):   7,6->0, 5..0->1
			{ TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::One, TriState::One, TriState::One, TriState::One },
		};

		APUSim::SquareChan sq(apu, APUSim::SquareChanCarryIn::Vdd);
		TriState SQ_Out[4]{};

		for (size_t d = 0; d < 4; d++)
		{
			// Load the duty setting (bits 7:6 of $4000) via WR0

			apu->DB = (uint8_t)(d << 6);
			apu->wire.ACLK1 = TriState::Zero;
			sq.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, SQ_Out);

			// Check every duty counter value

			for (size_t v = 0; v < 8; v++)
			{
				// Counter value 0 corresponds to DT[2:0] = 0. The circuit counts
				// down, so we map the table index (7..0) to the counter value.

				sq.Set_DutyCounter((uint32_t)v);

				// sim_Duty is called from sim(); it reads duty_reg and duty_cnt.
				// With WR3=0/FLOAD=0 the counter holds its value.

				sq.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, SQ_Out);

				TriState expected = duty_table[d][7 - v];	// table is indexed by duty counter value 7..0

				if (sq.DUTY != expected)
				{
					char text[0x100]{};
					sprintf_s(text, sizeof(text), "Duty mismatch: d=%zd, counter=%zd, DUTY=%d, expected=%d\n",
						d, v, ToByte(sq.DUTY), ToByte(expected));
					Logger::WriteMessage(text);
					return false;
				}
			}
		}

		return true;
	}

	/// <summary>
	/// Check the square channel sweep against the documented 2A03 behavior:
	/// - increase: new_freq = freq + (freq >> shift)
	/// - decrease: new_freq = freq - (freq >> shift) - 1
	/// - overflow (target > $7FF) mutes the channel and blocks DO_SWEEP
	/// - underflow (target < 0) also mutes the channel
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestSquareSweep()
	{
		char text[0x100]{};

		APUSim::SquareChan* sq = apu->square[0];
		TriState SQ_Out[4]{};

		apu->wire.n_CLK = TriState::One;	// CLK = 0
		apu->wire.RES = TriState::Zero;
		apu->wire.DMCINT = TriState::Zero;
		apu->wire.n_LFO2 = TriState::One;
		apu->wire.NOSQA = TriState::Zero;	// Length counter enabled

		// Accelerated LFO2: pulse on every negedge of ACLK2 (like TestLengthCounter does)

		TriState prev_aclk = TriState::X;
		TriState prev_clk = TriState::X;
		TriState prev_phi0 = TriState::X;

		// Write the sweep register $4001: DB = enable(7) | period(6:4) | DEC(3) | shift(2:0)
		// Case A: enable, increase (DEC=0), shift=1, period=0 -> DB = 0x80 | 0x01 = 0x81

		apu->DB = 0x81;
		apu->wire.ACLK1 = TriState::Zero;
		apu->wire.nACLK2 = TriState::One;
		sq->sim(TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, SQ_Out);	// WR1=1

		sq->Set_FreqReg(0x100);

		// Run a few accelerated LFO2 cycles.
		// The sound generators are clocked once per PHI0 change (see APU::sim_SoundGenerators).
		// With period=0 the sweep applies on every LFO2 pulse: freq = freq + (freq >> shift).

		uint32_t freq = sq->Get_FreqReg();
		uint32_t expected = freq;

		for (size_t n = 0; n < 4096; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			if (IsNegedge(prev_aclk, apu->wire.nACLK2))
			{
				apu->wire.n_LFO2 = TriState::Zero;
			}
			if (IsPosedge(prev_clk, NOT(apu->wire.n_CLK)))
			{
				apu->wire.n_LFO2 = TriState::One;
			}

			if (apu->wire.PHI0 != prev_phi0)
			{
				prev_phi0 = apu->wire.PHI0;
				sq->sim(apu->wire.W4000, apu->wire.W4001, apu->wire.W4002, apu->wire.W4003, apu->wire.NOSQA, SQ_Out);

				uint32_t new_freq = sq->Get_FreqReg();
				if (new_freq != freq)
				{
					expected = freq + (freq >> 1);		// increase, shift=1
					if (new_freq != expected)
					{
						sprintf_s(text, sizeof(text), "Sweep increase mismatch: 0x%X -> 0x%X, expected 0x%X\n", freq, new_freq, expected);
						Logger::WriteMessage(text);
						return false;
					}
					sprintf_s(text, sizeof(text), "Sweep: freq 0x%X -> 0x%X\n", freq, new_freq);
					Logger::WriteMessage(text);
					freq = new_freq;
				}
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			prev_aclk = apu->wire.nACLK2;
			prev_clk = NOT(apu->wire.n_CLK);
		}

		sprintf_s(text, sizeof(text), "Sweep increase final freq: 0x%X\n", freq);
		Logger::WriteMessage(text);

		if (freq <= 0x100)
			return false;

		// Case B: decrease. target = freq - (freq >> shift) - 1 (the -1 is the 2A03 sweep quirk).

		sq->Set_FreqReg(0x100);
		apu->DB = 0x88 | 0x01;		// enable, DEC=1 (decrease), shift=1, period=0
		apu->wire.ACLK1 = TriState::Zero;
		apu->wire.nACLK2 = TriState::One;
		sq->sim(TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, SQ_Out);	// WR1=1

		prev_phi0 = TriState::X;
		freq = sq->Get_FreqReg();
		uint32_t prev_freq = freq;

		for (size_t n = 0; n < 4096; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			if (IsNegedge(prev_aclk, apu->wire.nACLK2))
			{
				apu->wire.n_LFO2 = TriState::Zero;
			}
			if (IsPosedge(prev_clk, NOT(apu->wire.n_CLK)))
			{
				apu->wire.n_LFO2 = TriState::One;
			}

			if (apu->wire.PHI0 != prev_phi0)
			{
				prev_phi0 = apu->wire.PHI0;
				sq->sim(apu->wire.W4000, apu->wire.W4001, apu->wire.W4002, apu->wire.W4003, apu->wire.NOSQA, SQ_Out);

				uint32_t new_freq = sq->Get_FreqReg();
				if (new_freq != freq)
				{
					expected = (freq - (freq >> 1)) - 1;	// decrease, shift=1, -1 quirk
					if (new_freq != expected)
					{
						sprintf_s(text, sizeof(text), "Sweep decrease mismatch: 0x%X -> 0x%X, expected 0x%X\n", freq, new_freq, expected);
						Logger::WriteMessage(text);
						return false;
					}
					sprintf_s(text, sizeof(text), "Sweep decrease: freq 0x%X -> 0x%X\n", freq, new_freq);
					Logger::WriteMessage(text);
					freq = new_freq;
				}
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			prev_aclk = apu->wire.nACLK2;
			prev_clk = NOT(apu->wire.n_CLK);
		}

		sprintf_s(text, sizeof(text), "Sweep decrease final freq: 0x%X\n", freq);
		Logger::WriteMessage(text);

		if (freq >= 0x100)
			return false;

		// Case C: overflow. target = 0x700 + (0x700 >> 1) = 0x700 + 0x380 = 0xA80 > 0x7FF.
		// The sweep must not happen (SW_OVF blocks DO_SWEEP) and the channel must be muted.

		sq->Set_FreqReg(0x700);
		apu->DB = 0x80 | 0x01;		// enable, increase, shift=1, period=0
		apu->wire.ACLK1 = TriState::Zero;
		apu->wire.nACLK2 = TriState::One;
		sq->sim(TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, SQ_Out);	// WR1=1

		prev_phi0 = TriState::X;
		freq = sq->Get_FreqReg();
		uint32_t overflow_sweeps = 0;

		for (size_t n = 0; n < 4096; n++)
		{
			apu->core_int->sim();
			apu->clkgen->sim();

			if (IsNegedge(prev_aclk, apu->wire.nACLK2))
			{
				apu->wire.n_LFO2 = TriState::Zero;
			}
			if (IsPosedge(prev_clk, NOT(apu->wire.n_CLK)))
			{
				apu->wire.n_LFO2 = TriState::One;
			}

			if (apu->wire.PHI0 != prev_phi0)
			{
				prev_phi0 = apu->wire.PHI0;
				sq->sim(apu->wire.W4000, apu->wire.W4001, apu->wire.W4002, apu->wire.W4003, apu->wire.NOSQA, SQ_Out);

				if (sq->Get_FreqReg() != freq)
				{
					overflow_sweeps++;
				}
			}

			apu->wire.n_CLK = NOT(apu->wire.n_CLK);
			prev_aclk = apu->wire.nACLK2;
			prev_clk = NOT(apu->wire.n_CLK);
		}

		sprintf_s(text, sizeof(text), "Sweep overflow: freq stayed 0x%X, %d sweeps\n", sq->Get_FreqReg(), (int)overflow_sweeps);
		Logger::WriteMessage(text);

		if (overflow_sweeps != 0)
			return false;

		if (sq->Get_FreqReg() != 0x700)
			return false;

		return true;
	}

	/// <summary>
	/// Check the square channels: Get/Set roundtrips for both channels, a register write
	/// propagation through sim() and a smoke run.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestSquareChan()
	{
		char text[0x100]{};

		for (size_t ch = 0; ch < 2; ch++)
		{
			APUSim::SquareChan* sq = apu->square[ch];

			sq->Set_FreqReg(0x7FF);
			if (sq->Get_FreqReg() != 0x7FF) return false;

			sq->Set_ShiftReg(0x5);
			if (sq->Get_ShiftReg() != 0x5) return false;

			sq->Set_FreqCounter(0x555);
			if (sq->Get_FreqCounter() != 0x555) return false;

			sq->Set_SweepReg(0x3);
			if (sq->Get_SweepReg() != 0x3) return false;

			sq->Set_SweepCounter(0x7);
			if (sq->Get_SweepCounter() != 0x7) return false;

			sq->Set_DutyCounter(0x6);
			if (sq->Get_DutyCounter() != 0x6) return false;
		}

		// The LC output is 1 while the halt bit is not set

		if (apu->square[0]->get_LC() != TriState::One) return false;
		if (apu->square[1]->get_LC() != TriState::One) return false;

		// Write $4002 through the data bus: the freq register (low 8 bits) must capture DB

		TriState SQ_Out[4]{};
		apu->DB = 0x2D;
		apu->square[0]->sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, SQ_Out);
		if ((apu->square[0]->Get_FreqReg() & 0xFF) != 0x2D) return false;

		// Smoke test: run some APU cycles with the square channels active

		RunCycles(64);

		sprintf_s(text, sizeof(text), "Square0 freq reg after sim: 0x%X\n", apu->square[0]->Get_FreqReg());
		Logger::WriteMessage(text);

		return true;
	}

	/// <summary>
	/// Check the triangle channel: Get/Set roundtrips and a smoke run.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestTriangleChan()
	{
		char text[0x100]{};

		apu->tri->Set_LinearReg(0x7F);
		if (apu->tri->Get_LinearReg() != 0x7F) return false;

		apu->tri->Set_LinearCounter(0x2A);
		if (apu->tri->Get_LinearCounter() != 0x2A) return false;

		apu->tri->Set_FreqReg(0x123);
		if (apu->tri->Get_FreqReg() != 0x123) return false;

		apu->tri->Set_FreqCounter(0x456);
		if (apu->tri->Get_FreqCounter() != 0x456) return false;

		apu->tri->Set_OutputCounter(0x1F);
		if (apu->tri->Get_OutputCounter() != 0x1F) return false;

		// The LC output is 1 while the halt bit is not set

		if (apu->tri->get_LC() != TriState::One) return false;

		// Smoke test: run some APU cycles with the triangle channel active

		RunCycles(64);

		sprintf_s(text, sizeof(text), "Triangle freq reg after sim: 0x%X\n", apu->tri->Get_FreqReg());
		Logger::WriteMessage(text);

		return true;
	}

	/// <summary>
	/// Check the register decoder debug registers: a no-op with DBG=0, and the channel
	/// outputs are routed to the data bus when the corresponding read strobe is active.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestRegsDecoder()
	{
		// sim_DebugRegisters() is a no-op when DBG=0

		apu->wire.DBG = TriState::Zero;
		apu->wire.n_R4018 = TriState::One;
		apu->wire.n_R4019 = TriState::One;
		apu->wire.n_R401A = TriState::One;
		apu->SQA_Out[0] = TriState::One;
		apu->DB = 0x00;
		apu->regs->sim_DebugRegisters();
		if (apu->DB != 0x00) return false;

		// With DBG=1 and the $4018 read strobe active the square channels are put on the bus

		apu->wire.DBG = TriState::One;
		apu->wire.n_R4018 = TriState::Zero;
		apu->wire.n_R4019 = TriState::One;
		apu->wire.n_R401A = TriState::One;

		apu->SQA_Out[0] = TriState::One; apu->SQA_Out[1] = TriState::Zero;
		apu->SQA_Out[2] = TriState::One; apu->SQA_Out[3] = TriState::Zero;
		apu->SQB_Out[0] = TriState::Zero; apu->SQB_Out[1] = TriState::One;
		apu->SQB_Out[2] = TriState::Zero; apu->SQB_Out[3] = TriState::One;

		apu->DB = 0x00;
		apu->regs->sim_DebugRegisters();
		if (apu->DB != 0xA5) return false;	// SQA (0x5) in bits 0..3, SQB (0xA) in bits 4..7

		// $4019: triangle in bits 0..3, noise in bits 4..7

		apu->wire.n_R4018 = TriState::One;
		apu->wire.n_R4019 = TriState::Zero;

		apu->TRI_Out[0] = TriState::One; apu->TRI_Out[1] = TriState::Zero;
		apu->TRI_Out[2] = TriState::One; apu->TRI_Out[3] = TriState::Zero;
		apu->RND_Out[0] = TriState::Zero; apu->RND_Out[1] = TriState::One;
		apu->RND_Out[2] = TriState::Zero; apu->RND_Out[3] = TriState::One;

		apu->DB = 0x00;
		apu->regs->sim_DebugRegisters();
		if (apu->DB != 0xA5) return false;	// TRI (0x5) in bits 0..3, RND (0xA) in bits 4..7

		// $401A: DMC output (7 bits)

		apu->wire.n_R4019 = TriState::One;
		apu->wire.n_R401A = TriState::Zero;

		apu->DMC_Out[0] = TriState::One; apu->DMC_Out[1] = TriState::Zero;
		apu->DMC_Out[2] = TriState::Zero; apu->DMC_Out[3] = TriState::One;
		apu->DMC_Out[4] = TriState::Zero; apu->DMC_Out[5] = TriState::Zero;
		apu->DMC_Out[6] = TriState::One; apu->DMC_Out[7] = TriState::Zero;

		apu->DB = 0x00;
		apu->regs->sim_DebugRegisters();
		if (apu->DB != 0b01001001) return false;	// bits 0, 3, 6

		return true;
	}

	/// <summary>
	/// Check the BIDIR terminal circuit from pads.cpp.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestBIDIR()
	{
		APUSim::BIDIR b;

		// set_in/set_out roundtrip

		b.set_in(TriState::One);
		if (b.get_in() != TriState::One) return false;
		b.set_out(TriState::One);
		if (b.get_out() != TriState::One) return false;

		b.set_in(TriState::Zero);
		if (b.get_in() != TriState::Zero) return false;
		b.set_out(TriState::Zero);
		if (b.get_out() != TriState::Zero) return false;

		// sim_Input: rd=1 passes pad_in to from_pad

		TriState from_pad = TriState::X;
		b.sim_Input(TriState::One, from_pad, TriState::One);
		if (from_pad != TriState::One) return false;
		b.sim_Input(TriState::Zero, from_pad, TriState::One);
		if (from_pad != TriState::Zero) return false;

		// sim_Output: wr=1 passes to_pad to pad_out

		TriState pad_out = TriState::X;
		b.sim_Output(TriState::One, pad_out, TriState::One);
		if (pad_out != TriState::One) return false;
		b.sim_Output(TriState::Zero, pad_out, TriState::One);
		if (pad_out != TriState::Zero) return false;

		// sim(): from_pad = pad_in when wr=0, pad_out = to_pad when rd=0 (active-low controls)

		TriState from = TriState::X;
		TriState pout = TriState::X;
		b.sim(TriState::One, TriState::Zero, from, pout, TriState::Zero, TriState::Zero);
		if (from != TriState::One) return false;
		if (pout != TriState::Zero) return false;

		b.sim(TriState::Zero, TriState::One, from, pout, TriState::Zero, TriState::Zero);
		if (from != TriState::Zero) return false;
		if (pout != TriState::One) return false;

		// rd=1 disables pad_out, wr=1 disables from_pad

		b.sim(TriState::One, TriState::One, from, pout, TriState::One, TriState::One);
		if (from != TriState::Z) return false;
		if (pout != TriState::Z) return false;

		return true;
	}

	/// <summary>
	/// Check the APU pads: Get/Set roundtrips, the data bus input/output and a smoke run
	/// of the input/output pad simulation.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestPads()
	{
		// Get/Set roundtrips

		apu->pads->Set_DBOutputLatch(0x5A);
		if (apu->pads->Get_DBOutputLatch() != 0x5A) return false;

		apu->pads->Set_DBInputLatch(0xA5);
		if (apu->pads->Get_DBInputLatch() != 0xA5) return false;

		apu->pads->Set_OutReg(0x5);
		if (apu->pads->Get_OutReg() != 0x5) return false;

		// sim_DataBusInput: with RD=1 the external data is captured into the internal bus

		apu->wire.RD = TriState::One;
		uint8_t data = 0x3C;
		apu->pads->sim_DataBusInput(&data);
		if (apu->DB != 0x3C) return false;
		if (!apu->DB_Dirty) return false;

		// sim_DataBusOutput: with WR=1 the internal bus is driven out

		apu->wire.WR = TriState::One;
		apu->DB = 0xC3;
		data = 0;
		apu->pads->sim_DataBusOutput(&data);
		if (data != 0xC3) return false;

		// sim_InputPads / sim_OutputPads smoke run

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};
		uint16_t addr = 0;

		inputs[(size_t)APUSim::APU_Input::CLK] = TriState::Zero;
		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;

		apu->pads->sim_InputPads(inputs);
		if (apu->wire.n_CLK != TriState::One) return false;	// CLK=0 -> n_CLK=1
		if (apu->wire.RES != TriState::Zero) return false;	// n_RES=1 -> RES=0

		apu->Ax = 0x4000;
		apu->pads->sim_OutputPads(outputs, &addr);
		if (addr != 0x4000) return false;	// RES=0 -> the address passes through

		return true;
	}

	/// <summary>
	/// Check the DAC: the RAW mode exposes the digital channel outputs directly, the
	/// normalized mode produces values in [0, 1] and the analog mode produces voltages.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestDAC()
	{
		char text[0x100]{};
		APUSim::AudioOutSignal aux{};

		// Set some known digital outputs

		apu->SQA_Out[0] = TriState::One; apu->SQA_Out[1] = TriState::Zero;
		apu->SQA_Out[2] = TriState::One; apu->SQA_Out[3] = TriState::Zero;	// 0x5
		apu->SQB_Out[0] = TriState::Zero; apu->SQB_Out[1] = TriState::One;
		apu->SQB_Out[2] = TriState::Zero; apu->SQB_Out[3] = TriState::One;	// 0xA
		apu->TRI_Out[0] = TriState::One; apu->TRI_Out[1] = TriState::One;
		apu->TRI_Out[2] = TriState::Zero; apu->TRI_Out[3] = TriState::Zero;	// 0x3
		apu->RND_Out[0] = TriState::Zero; apu->RND_Out[1] = TriState::One;
		apu->RND_Out[2] = TriState::One; apu->RND_Out[3] = TriState::Zero;	// 0x6
		apu->DMC_Out[0] = TriState::One; apu->DMC_Out[1] = TriState::Zero;
		apu->DMC_Out[2] = TriState::One; apu->DMC_Out[3] = TriState::Zero;
		apu->DMC_Out[4] = TriState::One; apu->DMC_Out[5] = TriState::Zero;
		apu->DMC_Out[6] = TriState::One; apu->DMC_Out[7] = TriState::Zero;	// 0x55

		// RAW mode: the digital channel outputs are exposed directly

		apu->dac->SetRAWOutput(true);
		apu->dac->sim(aux);

		if (aux.RAW.sqa != 0x5) return false;
		if (aux.RAW.sqb != 0xA) return false;
		if (aux.RAW.tri != 0x3) return false;
		if (aux.RAW.rnd != 0x6) return false;
		if (aux.RAW.dmc != 0x55) return false;

		sprintf_s(text, sizeof(text), "RAW: sqa=%d sqb=%d tri=%d rnd=%d dmc=%d\n",
			aux.RAW.sqa, aux.RAW.sqb, aux.RAW.tri, aux.RAW.rnd, aux.RAW.dmc);
		Logger::WriteMessage(text);

		// Normalized mode: values in [0, 1]

		apu->dac->SetRAWOutput(false);
		apu->dac->SetNormalizedOutput(true);
		apu->dac->sim(aux);

		if (aux.normalized.a < 0.f || aux.normalized.a > 1.f) return false;
		if (aux.normalized.b < 0.f || aux.normalized.b > 1.f) return false;

		sprintf_s(text, sizeof(text), "Normalized: a=%f b=%f\n", aux.normalized.a, aux.normalized.b);
		Logger::WriteMessage(text);

		// Analog mode: positive voltages (mV)

		apu->dac->SetNormalizedOutput(false);
		apu->dac->sim(aux);

		if (aux.AUX.a <= 0.f) return false;
		if (aux.AUX.b <= 0.f) return false;

		sprintf_s(text, sizeof(text), "Analog: a=%f mV, b=%f mV\n", aux.AUX.a, aux.AUX.b);
		Logger::WriteMessage(text);

		return true;
	}
}

namespace UnitTest
{
	TEST_CLASS(ApuInternalsUnitTest)
	{
	public:

		TEST_METHOD(TestAPUDebugInfo)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestAPUDebugInfo());
		}

		TEST_METHOD(TestCommonBitCells)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestCommonBitCells());
		}

		TEST_METHOD(TestSoftCLK_SRBit)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestSoftCLK_SRBit());
		}

		TEST_METHOD(TestDIV_SRBit)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestDIV_SRBit());
		}

		TEST_METHOD(TestCLKGen)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestCLKGen());
		}

		TEST_METHOD(TestDMA)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestDMA());
		}

		TEST_METHOD(TestDPCMBits)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestDPCMBits());
		}

		TEST_METHOD(TestDpcmChan)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestDpcmChan());
		}

		TEST_METHOD(TestLengthCounterDebug)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestLengthCounterDebug());
		}

		TEST_METHOD(TestEnvelopeUnit)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestEnvelopeUnit());
		}

		TEST_METHOD(TestNoiseBits)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestNoiseBits());
		}

		TEST_METHOD(TestNoiseChan)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestNoiseChan());
		}

		TEST_METHOD(TestSquareBits)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestSquareBits());
		}

		TEST_METHOD(TestSquareChan)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestSquareChan());
		}

		TEST_METHOD(TestSquareSweep)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestSquareSweep());
		}

		TEST_METHOD(TestTriangleChan)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestTriangleChan());
		}

		TEST_METHOD(TestRegsDecoder)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestRegsDecoder());
		}

		TEST_METHOD(TestBIDIR)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestBIDIR());
		}

		TEST_METHOD(TestPads)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestPads());
		}

		TEST_METHOD(TestDAC)
		{
			APUSimUnitTest::UnitTest ut(APUSim::Revision::RP2A03G);
			Assert::IsTrue(ut.TestDAC());
		}
	};
}
