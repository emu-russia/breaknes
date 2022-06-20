// Various PPU simulator tests. Doing it without stress, according to the mood.

#include "pch.h"

using namespace BaseLogic;
using namespace PPUSim;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PPUSimUnitTest
{
	UnitTest::UnitTest(Revision rev)
	{
		ppu = new PPU(rev);
	}

	UnitTest::~UnitTest()
	{
		delete ppu;
	}

	void UnitTest::pclk()
	{
		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
	}

	void UnitTest::n_pclk()
	{
		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
	}

	void UnitTest::hv_count()
	{
		ppu->h->sim(TriState::One, TriState::Zero);
		ppu->v->sim(TriState::One, TriState::Zero);
	}

	/// <summary>
	/// Execute several PCLK cycles with RES = 1 and check that the counter value is 0.
	/// Make several cycles of counting beforehand.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::HVCounterTestRES()
	{
		size_t numPclkCycles = 3;

		// Count

		ppu->wire.RES = TriState::Zero;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		if (! (ppu->h->get() == numPclkCycles && ppu->v->get() == numPclkCycles) )
		{
			return false;
		}

		// Count while Reset

		ppu->wire.RES = TriState::One;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		return ppu->h->get() == 0 && ppu->v->get() == 0;
	}

	/// <summary>
	/// Run several PCLK cycles and make sure that the counters count.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::HVCounterTest()
	{
		size_t numPclkCycles = 7;

		// Count

		ppu->wire.RES = TriState::Zero;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		return ppu->h->get() == numPclkCycles && ppu->v->get() == numPclkCycles;
	}

	bool UnitTest::RunSingleHalfCLK(TriState CLK, TriState n_RES)
	{
		TriState inputs[(size_t)InputPad::Max]{};
		TriState outputs[(size_t)OutputPad::Max]{};
		uint8_t ext = 0;
		uint8_t data_bus = 0;
		uint8_t ad_bus = 0;
		uint8_t addrHi_bus = 0;
		VideoOutSignal vout{};

		inputs[(size_t)InputPad::RnW] = TriState::Zero;
		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::Zero;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::n_DBE] = TriState::One;

		inputs[(size_t)InputPad::n_RES] = n_RES;

		inputs[(size_t)InputPad::CLK] = CLK;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

		return true;
	}

	/// <summary>
	/// Run one PCLK and check that nothing is broken.
	/// The safest combinations of inputs are used.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::RunSinglePCLK()
	{
		TriState inputs[(size_t)InputPad::Max]{};
		TriState outputs[(size_t)OutputPad::Max]{};
		uint8_t ext = 0;
		uint8_t data_bus = 0;
		uint8_t ad_bus = 0;
		uint8_t addrHi_bus = 0;
		VideoOutSignal vout{};

		inputs[(size_t)InputPad::RnW] = TriState::Zero;
		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::Zero;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::n_DBE] = TriState::One;

		inputs[(size_t)InputPad::n_RES] = TriState::One;

		// Iterate over CLK until the internal PCLK counter changes value.

		auto prevPclk = ppu->GetPCLKCounter();

		while (prevPclk == ppu->GetPCLKCounter())
		{
			inputs[(size_t)InputPad::CLK] = TriState::Zero;

			ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

			inputs[(size_t)InputPad::CLK] = TriState::One;

			ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
		}

		return true;
	}

	/// <summary>
	/// Run the specified number of full scans.
	/// </summary>
	bool UnitTest::RunLines(size_t n)
	{
		size_t pclkInLine = 341;

		while (n--)
		{
			for (size_t pclk = 0; pclk < pclkInLine; pclk++)
			{
				RunSinglePCLK();
			}
		}

		return true;
	}

	/// <summary>
	/// Run the specified number of half-cycles (CLK) and output JSON with signals, in the manner of ChipScope utilities.
	/// </summary>
	bool UnitTest::RunHalfCyclesWithChipScope(size_t half_cycles, const char* filename)
	{
		size_t ts = 0;
		TriState CLK = TriState::Zero;
		Debug::EventLog log;

		ppu->regs->Debug_RenderAlwaysEnabled(true);
		//ppu->regs->Debug_ClippingAlwaysDisabled(true);

		// Reset

		for (size_t cnt = 0; cnt < 32; cnt++)
		{
			RunSingleHalfCLK(CLK, TriState::Zero);
			CLK = NOT(CLK);
		}

		// Continue

		for (size_t cnt = 0; cnt < half_cycles; cnt++)
		{
			RunSingleHalfCLK(CLK, TriState::One);

			PPU_Interconnects wires{};
			PPU_FSMStates fsm{};
			OAMEvalWires eval_wires{};

			ppu->GetDebugInfo_Wires(wires);
			ppu->GetDebugInfo_FSMStates(fsm);
			ppu->GetDebugInfo_OAMEval(eval_wires);

			LogWires(&log, wires, ts);
			LogFSMState(&log, fsm, ts);
			LogOAMEval(&log, eval_wires, ts);

			ts++;
			CLK = NOT(CLK);
		}

		CloseWires(&log, ts);
		CloseFSMState(&log, ts);
		CloseOAMEval(&log, ts);

		FILE* f = nullptr;
		fopen_s(&f, filename, "wt");
		if (f != nullptr)
		{
			std::string text;
			log.ToString(text);
			fwrite(text.c_str(), 1, text.size(), f);
			fclose(f);
		}

		return true;
	}

#define CloseWire(n, chan) {\
		if (wires_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
		} \
	}

#define CloseFSM(n, chan) { \
		if (fsm_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
		} \
	}

#define CloseEval(n, chan) { \
		if (eval_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
		} \
	}

	void UnitTest::CloseWires(Debug::EventLog* log, size_t ts)
	{
		CloseWire(CLK, 1000);
		CloseWire(n_CLK, 1001);
		CloseWire(PCLK, 1002);
		CloseWire(n_PCLK, 1003);

		CloseWire(H0_Dash, 2000);
		CloseWire(H0_Dash2, 2001);
		CloseWire(nH1_Dash, 2002);
		CloseWire(H1_Dash2, 2003);
		CloseWire(nH2_Dash, 2004);
		CloseWire(H2_Dash2, 2005);
		CloseWire(H3_Dash2, 2006);
		CloseWire(H4_Dash2, 2007);
		CloseWire(H5_Dash2, 2008);

		CloseWire(n_SPR0_EV, 3000);
		CloseWire(OFETCH, 3001);
		CloseWire(SPR_OV, 3002);
		CloseWire(OAMCTR2, 3003);
		CloseWire(OAM8, 3004);
		CloseWire(PD_FIFO, 3005);
	}

	void UnitTest::CloseFSMState(Debug::EventLog* log, size_t ts)
	{
		CloseFSM(S_EV, 100);
		CloseFSM(CLIP_O, 101);
		CloseFSM(CLIP_B, 102);
		CloseFSM(Z_HPOS, 103);
		CloseFSM(n_EVAL, 104);
		CloseFSM(E_EV, 105);
		CloseFSM(I_OAM2, 106);
		CloseFSM(PAR_O, 107);
		CloseFSM(n_VIS, 108);
		CloseFSM(F_NT, 109);
		CloseFSM(F_TB, 110);
		CloseFSM(F_TA, 111);
		CloseFSM(F_AT, 112);
		CloseFSM(n_FO, 113);
		CloseFSM(BPORCH, 114);
		CloseFSM(SC_CNT, 115);
		CloseFSM(BURST, 117);
		CloseFSM(SYNC, 118);
		CloseFSM(n_PICTURE, 119);
		CloseFSM(RESCL, 120);
		CloseFSM(BLNK, 124);
		CloseFSM(INT, 125);
	}

	void UnitTest::CloseOAMEval(Debug::EventLog* log, size_t ts)
	{
		CloseEval(OMFG, 300);
		CloseEval(OMSTEP, 301);
		CloseEval(OMOUT, 302);
		CloseEval(OMV, 303);
		CloseEval(OSTEP, 304);
		CloseEval(ORES, 305);
		CloseEval(TMV, 306);
		CloseEval(OAP, 307);
		CloseEval(COPY_STEP, 308);
		CloseEval(DO_COPY, 309);
		CloseEval(COPY_OVF, 310);
		CloseEval(OVZ, 311);
	}

#define OpenDirectWire(n, chan) { \
		if (wires.n == 1 && !wires_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			wires_open.n = 1; \
		} \
		else if (wires.n == 0 && wires_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			wires_open.n = 0; \
		} \
	}

#define OpenInverseWire(n, chan) { \
		if (wires.n == 0 && !wires_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			wires_open.n = 1; \
		} \
		else if (wires.n == 1 && wires_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			wires_open.n = 0; \
		} \
	}

#define OpenDirectFSM(n, chan) { \
		if (fsm.n == 1 && !fsm_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			fsm_open.n = 1; \
		} \
		else if (fsm.n == 0 && fsm_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			fsm_open.n = 0; \
		} \
	}

#define OpenInverseFSM(n, chan) { \
		if (fsm.n == 0 && !fsm_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			fsm_open.n = 1; \
		} \
		else if (fsm.n == 1 && fsm_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			fsm_open.n = 0; \
		} \
	}

#define OpenDirectEval(n, chan) { \
		if (eval_wires.n == 1 && !eval_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			eval_open.n = 1; \
		} \
		else if (eval_wires.n == 0 && eval_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			eval_open.n = 0; \
		} \
	}

#define OpenInverseEval(n, chan) { \
		if (eval_wires.n == 0 && !eval_open.n) \
		{ \
			log->TraceBegin(chan, #n, ts); \
			eval_open.n = 1; \
		} \
		else if (eval_wires.n == 1 && eval_open.n) \
		{ \
			log->TraceEnd(chan, ts); \
			eval_open.n = 0; \
		} \
	}

	void UnitTest::LogWires(Debug::EventLog* log, PPU_Interconnects& wires, size_t ts)
	{
		OpenDirectWire(CLK, 1000);
		OpenDirectWire(n_CLK, 1001);
		OpenDirectWire(PCLK, 1002);
		OpenDirectWire(n_PCLK, 1003);

		OpenDirectWire(H0_Dash, 2000);
		OpenDirectWire(H0_Dash2, 2001);
		OpenInverseWire(nH1_Dash, 2002);
		OpenDirectWire(H1_Dash2, 2003);
		OpenInverseWire(nH2_Dash, 2004);
		OpenDirectWire(H2_Dash2, 2005);
		OpenDirectWire(H3_Dash2, 2006);
		OpenDirectWire(H4_Dash2, 2007);
		OpenDirectWire(H5_Dash2, 2008);

		OpenDirectWire(n_SPR0_EV, 3000);
		OpenDirectWire(OFETCH, 3001);
		OpenDirectWire(SPR_OV, 3002);
		OpenDirectWire(OAMCTR2, 3003);
		OpenDirectWire(OAM8, 3004);
		OpenDirectWire(PD_FIFO, 3005);
	}

	void UnitTest::LogFSMState(Debug::EventLog* log, PPU_FSMStates& fsm, size_t ts)
	{
		OpenDirectFSM(S_EV, 100);
		OpenDirectFSM(CLIP_O, 101);
		OpenDirectFSM(CLIP_B, 102);
		OpenDirectFSM(Z_HPOS, 103);
		OpenInverseFSM(n_EVAL, 104);
		OpenDirectFSM(E_EV, 105);
		OpenDirectFSM(I_OAM2, 106);
		OpenDirectFSM(PAR_O, 107);
		OpenInverseFSM(n_VIS, 108);
		OpenDirectFSM(F_NT, 109);
		OpenDirectFSM(F_TB, 110);
		OpenDirectFSM(F_TA, 111);
		OpenDirectFSM(F_AT, 112);
		OpenDirectFSM(n_FO, 113);
		OpenDirectFSM(BPORCH, 114);
		OpenDirectFSM(SC_CNT, 115);
		OpenDirectFSM(BURST, 117);
		OpenDirectFSM(SYNC, 118);
		OpenInverseFSM(n_PICTURE, 119);
		OpenDirectFSM(RESCL, 120);
		OpenDirectFSM(BLNK, 124);
		OpenDirectFSM(INT, 125);
	}

	void UnitTest::LogOAMEval(Debug::EventLog* log, OAMEvalWires& eval_wires, size_t ts)
	{
		OpenDirectEval(OMFG, 300);
		OpenDirectEval(OMSTEP, 301);
		OpenDirectEval(OMOUT, 302);
		OpenDirectEval(OMV, 303);
		OpenDirectEval(OSTEP, 304);
		OpenDirectEval(ORES, 305);
		OpenDirectEval(TMV, 306);
		OpenDirectEval(OAP, 307);
		OpenDirectEval(COPY_STEP, 308);
		OpenDirectEval(DO_COPY, 309);
		OpenDirectEval(COPY_OVF, 310);
		OpenDirectEval(OVZ, 311);
	}

	uint8_t UnitTest::TestComparator(uint8_t a, uint8_t b)
	{
		// OV = V - OB

		ppu->v->set(a);
		TriState OB[8]{};
		Unpack(b, OB);
		for (size_t bit_num = 0; bit_num < 8; bit_num++)
		{
			ppu->oam->set_OB(bit_num, OB[bit_num]);
		}
		ppu->wire.PCLK = TriState::One;

		ppu->eval->sim_Comparator();

		return Pack(ppu->wire.OV);
	}

	bool UnitTest::TestBGC_SR8()
	{
		BGC_SR8 sr;

		for (size_t n=0; n<0x100; n++)
		{
			uint8_t sr_val = (uint8_t)n;
			TriState sr_val_unpacked[8]{};
			TriState sout[8]{};
			TriState unused[8]{};
			TriState srin = TriState::Zero;

			Unpack(sr_val, sr_val_unpacked);

			sr.sim(sr_val_unpacked, srin, TriState::One, TriState::Zero, TriState::One, sout);

			if (Pack(sout) != sr_val)
				return false;

			for (size_t i = 0; i < 8; i++)
			{
				sr.sim(unused, srin, TriState::Zero, TriState::One, TriState::Zero, sout);
				sr_val >>= 1;

				sr.sim(unused, srin, TriState::Zero, TriState::Zero, TriState::One, sout);

				if (Pack(sout) != sr_val)
					return false;
			}
		}

		return true;
	}

	/// <summary>
	/// Check the operation of the main OAM counter in different modes (Mode +1/+4).
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestOAMCounter()
	{
		ppu->wire.n_PCLK = TriState::One;

		ppu->eval->OMFG = TriState::Zero;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->wire.n_W4 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		// Reset

		ppu->fsm.PARO = TriState::One;
		ppu->eval->OMOUT = TriState::One;
		ppu->eval->sim_MainCounter();

		if (ppu->eval->Debug_GetMainCounter() != 0)
			return false;

		// Count in Mode1

		ppu->eval->OMFG = TriState::Zero;
		ppu->fsm.BLNK = TriState::One;
		ppu->fsm.PARO = TriState::Zero;

		for (size_t n = 0; n < 0x100; n++)
		{
			if (ppu->eval->Debug_GetMainCounter() != n)
				return false;

			ppu->eval->OMOUT = TriState::One;
			ppu->eval->OMSTEP = TriState::Zero;
			ppu->eval->sim_MainCounter();

			if (n == 255)
			{
				auto OMV = ppu->eval->omv_latch.get();
				if (OMV != TriState::One)
					return false;
			}

			ppu->eval->OMOUT = TriState::Zero;
			ppu->eval->OMSTEP = TriState::One;
			ppu->eval->sim_MainCounter();
		}

		if (ppu->eval->Debug_GetMainCounter() != 0)
			return false;

		// Reset

		ppu->fsm.PARO = TriState::One;
		ppu->eval->OMOUT = TriState::One;
		ppu->eval->sim_MainCounter();

		if (ppu->eval->Debug_GetMainCounter() != 0)
			return false;

		// Count in Mode4

		ppu->eval->OMFG = TriState::One;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->fsm.PARO = TriState::Zero;

		for (size_t n = 0; n < 0x100; n+=4)
		{
			if (ppu->eval->Debug_GetMainCounter() != n)
				return false;

			ppu->eval->OMOUT = TriState::One;
			ppu->eval->OMSTEP = TriState::Zero;
			ppu->eval->sim_MainCounter();

			if (n == (0x100 - 4))
			{
				auto OMV = ppu->eval->omv_latch.get();
				if (OMV != TriState::One)
					return false;
			}

			ppu->eval->OMOUT = TriState::Zero;
			ppu->eval->OMSTEP = TriState::One;
			ppu->eval->sim_MainCounter();
		}

		if (ppu->eval->Debug_GetMainCounter() != 0)
			return false;

		return true;
	}

	/// <summary>
	/// Check that the counter correctly counts down to 0 and correctly outputs the overflow signal.
	/// </summary>
	/// <param name="v"></param>
	/// <returns></returns>
	bool UnitTest::TestFIFODownCounter(size_t v)
	{
		FIFOLane lane(ppu);
		TriState carry_out;

		// Load value

		Unpack((uint8_t)v, ppu->wire.OB);
		
		lane.UPD = TriState::Zero;
		lane.LOAD = TriState::One;
		lane.STEP = TriState::Zero;
		carry_out = lane.sim_Counter();

		if (v == 0)
		{
			if (carry_out != TriState::One)
				return false;

			return true;
		}
		else
		{
			if (carry_out != TriState::Zero)
				return false;
		}

		lane.UPD = TriState::One;
		lane.LOAD = TriState::Zero;
		lane.STEP = TriState::Zero;
		carry_out = lane.sim_Counter();

		if (lane.get_Counter() != v)
			return false;

		// In the loop count down to 1 and check that there is an overflow signal when needed.

		while (v != 1)
		{
			lane.UPD = TriState::Zero;
			lane.LOAD = TriState::Zero;
			lane.STEP = TriState::One;
			carry_out = lane.sim_Counter();

			if (carry_out != TriState::Zero)
				return false;

			lane.UPD = TriState::One;
			lane.LOAD = TriState::Zero;
			lane.STEP = TriState::Zero;
			carry_out = lane.sim_Counter();

			v--;

			if (carry_out != TriState::Zero)
				return false;

			if (lane.get_Counter() != v)
				return false;
		}

		// The last tick to zero performs an overflow.

		lane.UPD = TriState::Zero;
		lane.LOAD = TriState::Zero;
		lane.STEP = TriState::One;
		carry_out = lane.sim_Counter();

		if (carry_out != TriState::One)
			return false;

		if (lane.get_Counter() != 0)
			return false;

		return true;
	}

	/// <summary>
	/// Check paired FIFO shift registers.
	/// </summary>
	bool UnitTest::TestFIFOPairedSR(uint8_t val)
	{
		FIFOLane lane(ppu);

		// Load a value on one register and an inverse value on the other register.

		TriState tx[8]{};
		TriState ntx[8]{};
		TriState unused[8]{};

		Unpack(val, tx);
		Unpack(~val, ntx);

		ppu->wire.n_PCLK = TriState::Zero;
		lane.T_SR0 = TriState::One;
		lane.T_SR1 = TriState::Zero;
		lane.SR_EN = TriState::Zero;
		lane.sim_PairedSR(tx);

		lane.T_SR0 = TriState::Zero;
		lane.T_SR1 = TriState::One;
		lane.sim_PairedSR(ntx);

		// Perform 8 shift iterations and check the output.

		lane.T_SR0 = TriState::Zero;
		lane.T_SR1 = TriState::Zero;

		for (size_t i = 0; i < 8; i++)
		{
			ppu->wire.n_PCLK = TriState::One;
			lane.SR_EN = TriState::Zero;
			lane.sim_PairedSR(unused);

			ppu->wire.n_PCLK = TriState::Zero;
			lane.SR_EN = TriState::One;
			lane.sim_PairedSR(unused);

			if ((lane.nZ_COL0 == TriState::One ? 1 : 0) != (val & 1))
				return false;

			if ((lane.nZ_COL1 == TriState::One ? 1 : 0) != (~val & 1))
				return false;

			val >>= 1;
		}

		// Perform another shift and make sure that the registers push out 1.

		ppu->wire.n_PCLK = TriState::One;
		lane.SR_EN = TriState::Zero;
		lane.sim_PairedSR(unused);

		ppu->wire.n_PCLK = TriState::Zero;
		lane.SR_EN = TriState::One;
		lane.sim_PairedSR(unused);

		if (lane.nZ_COL0 != TriState::One)
			return false;

		if (lane.nZ_COL1 != TriState::One)
			return false;

		return true;
	}

	/// <summary>
	/// Test the Chroma Decoder output numbers for the NTSC PPU VideoOut and their correspondence to CC0-3 / Burst.
	/// </summary>
	bool UnitTest::TestNtscPpuChromaDecoderOutputs()
	{
		VideoOut vout(ppu);

		size_t cc_map[_countof(vout.P)] = { 4, 6, 11, 3, 9, 1, 7, 12, 5, 10, 2, 8, 0 };
		size_t cb_output = 5;

		vout.cc_burst_latch.set(TriState::Zero, TriState::One);

		// Colors 13-15 have no phase
		
		for (size_t cc = 0; cc < 13; cc++)
		{
			vout.cc_latch2[0].set((cc & 1) ? TriState::One : TriState::Zero, TriState::One);
			vout.cc_latch2[1].set((cc & 2) ? TriState::One : TriState::Zero, TriState::One);
			vout.cc_latch2[2].set((cc & 4) ? TriState::One : TriState::Zero, TriState::One);
			vout.cc_latch2[3].set((cc & 8) ? TriState::One : TriState::Zero, TriState::One);

			vout.sim_ChromaDecoder();

			for (size_t n = 0; n < _countof(vout.P); n++)
			{
				if (n == cc_map[cc])
				{
					if (vout.P[n] != TriState::One)
						return false;
				}
				else
				{
					if (vout.P[n] != TriState::Zero)
						return false;
				}
			}
		}

		// Color Burst

		vout.cc_latch2[0].set(TriState::Zero, TriState::One);
		vout.cc_latch2[1].set(TriState::Zero, TriState::One);
		vout.cc_latch2[2].set(TriState::Zero, TriState::One);
		vout.cc_burst_latch.set(TriState::One, TriState::One);

		vout.sim_ChromaDecoder();

		for (size_t n = 0; n < _countof(vout.P); n++)
		{
			if (n == cb_output)
			{
				if (vout.P[n] != TriState::One)
					return false;
			}
			else
			{
				if (vout.P[n] != TriState::Zero)
					return false;
			}
		}

		return true;
	}

	/// <summary>
	/// Output the state of Phase Shifter outputs for a number of PCLK cycles.
	/// </summary>
	void UnitTest::DumpNtscPpuPhaseShifter(bool reset)
	{
		VideoOut vout(ppu);

		// Reset

		if (reset)
		{
			ppu->wire.RES = TriState::One;
			ppu->wire.CLK = TriState::Zero;
			ppu->wire.n_CLK = TriState::One;

			vout.sim_PhaseShifter(ppu->wire.n_CLK, ppu->wire.CLK, ppu->wire.RES);
			Logger::WriteMessage("After Reset:\n");
			DumpPhaseShifter(vout);
		}

		// Run single PCLK

		ppu->wire.RES = TriState::Zero;

		size_t HalfCycles = 128;
		TriState CLK = TriState::Zero;

		Logger::WriteMessage("Few cycles later:\n");

		for (size_t n = 0; n < HalfCycles; n++)
		{
			ppu->wire.CLK = CLK;
			ppu->wire.n_CLK = NOT(CLK);

			vout.sim_PhaseShifter(ppu->wire.n_CLK, ppu->wire.CLK, ppu->wire.RES);
			DumpPhaseShifter(vout);

			CLK = NOT(CLK);
		}
	}

	void UnitTest::DumpPhaseShifter(VideoOut& vout)
	{
		std::string text = "";

		size_t cc_map[_countof(vout.P)] = { 4, 6, 11, 3, 9, 1, 7, 12, 5, 10, 2, 8, 0 };
		char *int_to_hex[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };

		// Sorted by color from 1 to 12 (inclusive)

		for (size_t n = 1; n < 13; n++)
		{
			text += vout.PZ[cc_map[n]] == TriState::One ? int_to_hex[n] : ".";
		}

		text += "\n";

		Logger::WriteMessage(text.c_str());
	}

	/// <summary>
	/// Output all the colors that the NTSC PPU is capable of, in RGB format.
	/// </summary>
	void UnitTest::DumpNtscPpuColorSpace(bool emphasis)
	{
		for (size_t emphasis_bits = 0; emphasis_bits < (emphasis ? 8 : 1); emphasis_bits++)
		{
			Logger::WriteMessage(("Emphasis band: " + std::to_string(emphasis_bits) + "\n").c_str());

			for (size_t chroma_luma = 0; chroma_luma < 64; chroma_luma++)
			{
				uint16_t raw = (uint16_t)(chroma_luma | (emphasis_bits << 6));

				PPUSim::VideoOutSignal rawIn{}, rgbOut{};

				rawIn.RAW.raw = raw;
				ppu->ConvertRAWToRGB(rawIn, rgbOut);

				Logger::WriteMessage(("Color: " + std::to_string(chroma_luma) + ", RGB: " +
					std::to_string(rgbOut.RGB.RED) + " " +
					std::to_string(rgbOut.RGB.GREEN) + " " +
					std::to_string(rgbOut.RGB.BLUE) + "\n").c_str());
			}
		}
	}
}
