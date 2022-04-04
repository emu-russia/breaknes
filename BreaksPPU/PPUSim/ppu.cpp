// Main module and auxiliary logic.

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	PPU::PPU(Revision _rev)
	{
		rev = _rev;

		h = new HVCounter(this, 9);
		v = new HVCounter(this, 9);
		vid_out = new VideoOut(this);
		hv_fsm = new FSM(this);
		mux = new Mux(this);
		vram_ctrl = new VRAM_Control(this);
	}

	PPU::~PPU()
	{
		delete h;
		delete v;
		delete vid_out;
		delete hv_fsm;
		delete mux;
		delete vram_ctrl;
	}

	void PPU::sim(TriState inputs[], TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout)
	{
		// Input terminals and binding

		wire.RnW = inputs[(size_t)InputPad::RnW];
		wire.RS[0] = inputs[(size_t)InputPad::RS0];
		wire.RS[1] = inputs[(size_t)InputPad::RS1];
		wire.RS[2] = inputs[(size_t)InputPad::RS2];
		wire.n_DBE = inputs[(size_t)InputPad::n_DBE];

		wire.RES = NOT(inputs[(size_t)InputPad::n_RES]);

		wire.CLK = inputs[(size_t)InputPad::CLK];
		wire.n_CLK = NOT(wire.CLK);

		Reset_FF.set(NOR(wire.RES, NOR(Reset_FF.get(), hv_fsm->get_RESCL())));
		wire.RC = NOT(Reset_FF.get());

		sim_PCLK();

		// H/V Control logic

		hv_fsm->sim();

		// The other parts

		vid_out->sim(vout);

		// Output terminals

		outputs[(size_t)OutputPad::n_INT] = fsm.INT ? TriState::Zero : TriState::Z;
		outputs[(size_t)OutputPad::ALE] = NOT(wire.n_ALE);
		outputs[(size_t)OutputPad::n_RD] = NOT(wire.RD);
		outputs[(size_t)OutputPad::n_WR] = NOT(wire.WR);
	}

	void PPU::sim_PCLK()
	{
		TriState CLK = wire.CLK;
		TriState n_CLK = wire.n_CLK;

		// TBD: Add support for other PPU revisions when a critical mass of code is ready.

		if (rev == Revision::RP2C02G)
		{
			pclk_1.set(NOR(wire.RES, pclk_4.nget()), n_CLK);
			pclk_2.set(pclk_1.nget(), CLK);
			pclk_3.set(pclk_2.nget(), n_CLK);
			pclk_4.set(pclk_3.nget(), CLK);

			TriState pclk = pclk_4.nget();

			// The software PCLK counter is triggered by the falling edge.
			// This is purely a software design for convenience, and has nothing to do with PPU hardware circuitry.

			if (wire.PCLK /*prev*/ == TriState::One && pclk /*new*/ == TriState::Zero)
			{
				pclk_counter++;
			}

			wire.PCLK = pclk;
			wire.n_PCLK = NOT(wire.PCLK);
		}
	}

	size_t PPU::GetPCLKCounter()
	{
		return pclk_counter;
	}

	void PPU::ResetPCLKCounter()
	{
		pclk_counter = 0;
	}
}
