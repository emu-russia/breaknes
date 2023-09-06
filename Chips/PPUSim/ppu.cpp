// Main module and auxiliary logic.

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	/// <summary>
	/// Constructor. Creates all PPU modules.
	/// </summary>
	/// <param name="_rev">Revision of the PPU chip.</param>
	/// <param name="VideoGen">true: Create a special version of the PPU that contains only a video generator.</param>
	PPU::PPU(Revision _rev, bool VideoGen)
	{
		rev = _rev;

		if (!VideoGen)
		{
			regs = new ControlRegs(this);
			h = new HVCounter(this, 9);
			v = new HVCounter(this, 9);
			hv_dec = new HVDecoder(this);
			hv_fsm = new FSM(this);
			cram = new CRAM(this);
			mux = new Mux(this);
			eval = new OAMEval(this);
			oam = new OAM(this);
			fifo = new FIFO(this);
			data_reader = new DataReader(this);
			vram_ctrl = new VRAM_Control(this);
		}

		vid_out = new VideoOut(this);
	}

	PPU::~PPU()
	{
		if (regs)
			delete regs;
		if (h)
			delete h;
		if (v)
			delete v;
		if (hv_dec)
			delete hv_dec;
		if (hv_fsm)
			delete hv_fsm;
		if (cram)
			delete cram;
		if (vid_out)
			delete vid_out;
		if (mux)
			delete mux;
		if (eval)
			delete eval;
		if (oam)
			delete oam;
		if (fifo)
			delete fifo;
		if (data_reader)
			delete data_reader;
		if (vram_ctrl)
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

		regs->sim_RWDecoder();

		wire.RES = NOT(inputs[(size_t)InputPad::n_RES]);

		if (wire.RES == TriState::One)
		{
			ResetPCLKCounter();
		}

		wire.CLK = inputs[(size_t)InputPad::CLK];
		wire.n_CLK = NOT(wire.CLK);

		sim_PCLK();

		hv_fsm->sim_RESCL_early();

		Reset_FF.set(NOR(fsm.RESCL, NOR(wire.RES, Reset_FF.get())));
		wire.RC = NOT(Reset_FF.nget());

		sim_BusInput(ext, data_bus, ad_bus);

		// Regs

		regs->sim();

		if (Prev_PCLK != wire.PCLK)
		{
			// H/V Control logic

			TriState* HPLA;
			hv_dec->sim_HDecoder(hv_fsm->get_VB(), hv_fsm->get_BLNK(wire.BLACK), &HPLA);
			TriState* VPLA;
			hv_dec->sim_VDecoder(&VPLA);

			hv_fsm->sim(HPLA, VPLA);

			h->sim(TriState::One, wire.HC);
			TriState V_IN = HPLA[23];
			v->sim(V_IN, wire.VC);

			// The other parts

			fifo->sim_SpriteH();

			regs->sim_CLP();

			vram_ctrl->sim();

			oam->sim_OFETCH();

			eval->sim();

			oam->sim();

			data_reader->sim();

			fifo->sim();

			vram_ctrl->sim_TH_MUX();

			vram_ctrl->sim_ReadBuffer();

			mux->sim();

			cram->sim();

			Prev_PCLK = wire.PCLK;
		}

		vid_out->sim(vout);

		// Output terminals

		outputs[(size_t)OutputPad::n_INT] = fsm.INT ? TriState::Zero : TriState::Z;
		outputs[(size_t)OutputPad::ALE] = NOT(wire.n_ALE);
		outputs[(size_t)OutputPad::n_RD] = NOT(wire.RD);
		outputs[(size_t)OutputPad::n_WR] = NOT(wire.WR);

		sim_BusOutput(ext, data_bus, ad_bus, addrHi_bus);
	}

	void PPU::sim_BusInput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus)
	{
		switch (rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				TriState nSLAVE = regs->get_nSLAVE();

				for (size_t n = 0; n < 4; n++)
				{
					TriState extIn = (((*ext) >> n) & 1) ? TriState::One : TriState::Zero;
					wire.EXT_In[n] = NOR(NOT(extIn), nSLAVE);
				}
			}
			break;

			// TBD: The other RGB PPU's appear to be similar, but there are no pictures yet, we do not speculate.

			case Revision::RP2C04_0003:
			{
				for (size_t n = 0; n < 4; n++)
				{
					wire.EXT_In[n] = TriState::Zero;
				}
			}
			break;
		}

		if (wire.n_WR == TriState::Zero)
		{
			DB = *data_bus;
		}

		PD = *ad_bus;
	}

	void PPU::sim_BusOutput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus)
	{
		TriState n_PCLK = wire.n_PCLK;

		switch (rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				for (size_t n = 0; n < 4; n++)
				{
					extout_latch[n].set(wire.n_EXT_Out[n], n_PCLK);
				}

				if (wire.n_SLAVE == TriState::One)
				{
					*ext = 0;
					for (size_t n = 0; n < 4; n++)
					{
						TriState extOut = NOR(NOT(wire.n_SLAVE), extout_latch[n].get());
						(*ext) |= ((extOut == TriState::One) ? 1 : 0) << n;
					}
				}
			}
			break;

			case Revision::RP2C04_0003:
			{
				// There are no EXT terminals in the RGB PPUs.
			}
			break;
		}

		if (wire.n_RD == TriState::Zero)
		{
			*data_bus = DB;
		}

		if (wire.RD == TriState::Zero)
		{
			uint8_t PABot = 0;
			for (size_t n = 0; n < 8; n++)
			{
				PABot |= (wire.n_PA_Bot[n] == TriState::Zero ? 1 : 0) << n;
			}
			*ad_bus = PABot;
		}

		uint8_t PATop = 0;
		for (size_t n = 0; n < 6; n++)
		{
			PATop |= (wire.n_PA_Top[n] == TriState::Zero ? 1 : 0) << n;
		}
		*addrHi_bus = PATop;
	}

	size_t PPU::GetPCLKCounter()
	{
		return pclk_counter;
	}

	void PPU::ResetPCLKCounter()
	{
		pclk_counter = 0;
		Prev_PCLK = TriState::X;
	}

	const char* PPU::RevisionToStr(Revision rev)
	{
		switch (rev)
		{
			case Revision::RP2C02G: return "RP2C02G";
			case Revision::RP2C02H: return "RP2C02H";

			case Revision::RP2C03B: return "RP2C03B";
			case Revision::RP2C03C: return "RP2C03C";
			case Revision::RC2C03B: return "RC2C03B";
			case Revision::RC2C03C: return "RC2C03C";
			case Revision::RP2C04_0001: return "RP2C04-0001";
			case Revision::RP2C04_0002: return "RP2C04-0002";
			case Revision::RP2C04_0003: return "RP2C04-0003";
			case Revision::RP2C04_0004: return "RP2C04-0004";
			case Revision::RC2C05_01: return "RC2C05-01";
			case Revision::RC2C05_02: return "RC2C05-02";
			case Revision::RC2C05_03: return "RC2C05-03";
			case Revision::RC2C05_04: return "RC2C05-04";
			case Revision::RC2C05_99: return "RC2C05-99";
			
			case Revision::RP2C07_0: return "RP2C07-0";

			case Revision::UMC_UA6538: return "UMC UA6538";

			default:
				break;
		}
		return "Unknown";
	}

	TriState PPU::GetDBBit(size_t n)
	{
		TriState DBBit = (DB & (1 << n)) != 0 ? TriState::One : TriState::Zero;
		return DBBit;
	}

	void PPU::SetDBBit(size_t n, TriState bit_val)
	{
		if (bit_val != TriState::Z)
		{
			uint8_t out = DB & ~(1 << n);
			out |= (bit_val == BaseLogic::One ? 1 : 0) << n;
			DB = out;
		}
	}

	TriState PPU::GetPDBit(size_t n)
	{
		TriState PDBit = (PD & (1 << n)) != 0 ? TriState::One : TriState::Zero;
		return PDBit;
	}

	void PPU::SetPDBit(size_t n, TriState bit_val)
	{
		if (bit_val != TriState::Z)
		{
			uint8_t out = PD & ~(1 << n);
			out |= (bit_val == BaseLogic::One ? 1 : 0) << n;
			PD = out;
		}
	}

	size_t PPU::GetHCounter()
	{
		return h->get();
	}

	size_t PPU::GetVCounter()
	{
		return v->get();
	}

	void PPU::GetSignalFeatures(VideoSignalFeatures& features)
	{
		vid_out->GetSignalFeatures(features);
	}

	void PPU::SetRAWOutput(bool enable)
	{
		vid_out->SetRAWOutput(enable);
	}

	void PPU::ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		vid_out->ConvertRAWToRGB(rawIn, rgbOut);
	}

	void PPU::SetOamDecayBehavior(OAMDecayBehavior behavior)
	{
		if (oam != nullptr)
		{
			oam->SetOamDecayBehavior(behavior);
		}
	}

	bool PPU::IsComposite()
	{
		return vid_out->IsComposite();
	}

	void PPU::SetCompositeNoise(float volts)
	{
		vid_out->SetCompositeNoise(volts);
	}
}
