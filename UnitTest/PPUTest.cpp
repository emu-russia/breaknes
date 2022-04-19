// Various PPU simulator tests. Doing it without stress, according to the mood.

#include "pch.h"

using namespace BaseLogic;
using namespace PPUSim;

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
		ppu->regs->Debug_ClippingAlwaysDisabled(true);

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

			ppu->GetDebugInfo_Wires(wires);
			ppu->GetDebugInfo_FSMStates(fsm);

			LogWires(&log, wires, ts);
			LogFSMState(&log, fsm, ts);

			ts++;
			CLK = NOT(CLK);
		}

		CloseWires(&log, ts);
		CloseFSMState(&log, ts);

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

		CloseWire(I2SEV, 3000);
		CloseWire(OFETCH, 3001);
		CloseWire(SPR_OV, 3002);
		CloseWire(OAMCTR2, 3003);
		CloseWire(OAM8, 3004);
		CloseWire(Z_FIFO, 3005);
	}

	void UnitTest::CloseFSMState(Debug::EventLog* log, size_t ts)
	{
		CloseFSM(S_EV, 100);
		CloseFSM(CLIP_O, 101);
		CloseFSM(CLIP_B, 102);
		CloseFSM(Z_HPOS, 103);
		CloseFSM(EVAL, 104);
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
		//CloseFSM(n_HB, 116);
		CloseFSM(BURST, 117);
		CloseFSM(HSYNC, 118);
		CloseFSM(PICTURE, 119);
		CloseFSM(RESCL, 120);
		CloseFSM(VSYNC, 121);
		CloseFSM(n_VSET, 122);
		CloseFSM(VB, 123);
		CloseFSM(BLNK, 124);
		CloseFSM(INT, 125);
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

		OpenDirectWire(I2SEV, 3000);
		OpenDirectWire(OFETCH, 3001);
		OpenDirectWire(SPR_OV, 3002);
		OpenDirectWire(OAMCTR2, 3003);
		OpenDirectWire(OAM8, 3004);
		OpenDirectWire(Z_FIFO, 3005);
	}

	void UnitTest::LogFSMState(Debug::EventLog* log, PPU_FSMStates& fsm, size_t ts)
	{
		OpenDirectFSM(S_EV, 100);
		OpenDirectFSM(CLIP_O, 101);
		OpenDirectFSM(CLIP_B, 102);
		OpenDirectFSM(Z_HPOS, 103);
		OpenInverseFSM(EVAL, 104);
		OpenDirectFSM(E_EV, 105);
		OpenDirectFSM(I_OAM2, 106);
		OpenDirectFSM(PAR_O, 107);
		OpenInverseFSM(n_VIS, 108);
		OpenInverseFSM(F_NT, 109);
		OpenDirectFSM(F_TB, 110);
		OpenDirectFSM(F_TA, 111);
		OpenDirectFSM(F_AT, 112);
		OpenDirectFSM(n_FO, 113);
		OpenInverseFSM(BPORCH, 114);
		OpenDirectFSM(SC_CNT, 115);
		//OpenInverseFSM(n_HB, 116);
		OpenDirectFSM(BURST, 117);
		OpenDirectFSM(HSYNC, 118);
		OpenDirectFSM(PICTURE, 119);
		OpenDirectFSM(RESCL, 120);
		OpenDirectFSM(VSYNC, 121);
		OpenInverseFSM(n_VSET, 122);
		OpenDirectFSM(VB, 123);
		OpenDirectFSM(BLNK, 124);
		OpenDirectFSM(INT, 125);
	}
}
