// Control Registers

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	ControlRegs::ControlRegs(PPU* parent)
	{
		ppu = parent;
	}

	ControlRegs::~ControlRegs()
	{
	}

	void ControlRegs::sim()
	{
		sim_RegularRegOps();
		sim_W56RegOps();
		sim_FirstSecond_SCCX_Write();

		sim_RegFFs();

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				sim_PalBLACK();
				break;
		}
	}

	void ControlRegs::sim_RWDecoder()
	{
		const bool enabled = ppu->wire.n_DBE == TriState::Zero;
		const bool read = ppu->wire.RnW == TriState::One;
		ppu->wire.n_RD = enabled && read ? TriState::Zero : TriState::One;
		ppu->wire.n_WR = enabled && !read ? TriState::Zero : TriState::One;
	}

	void ControlRegs::sim_RegularRegOps()
	{
		const uint8_t code = static_cast<uint8_t>(Pack3(ppu->wire.RS) | (ppu->wire.RnW == TriState::One ? 8 : 0));
		if (code == previous_decode) return;
		previous_decode = code;
		ppu->wire.n_R7 = code == 15 ? TriState::Zero : TriState::One;
		ppu->wire.n_W7 = code == 7 ? TriState::Zero : TriState::One;
		ppu->wire.n_W4 = code == 4 ? TriState::Zero : TriState::One;
		ppu->wire.n_W3 = code == 3 ? TriState::Zero : TriState::One;
		ppu->wire.n_R2 = code == 10 ? TriState::Zero : TriState::One;
		ppu->wire.n_W1 = code == 1 ? TriState::Zero : TriState::One;
		ppu->wire.n_W0 = code == 0 ? TriState::Zero : TriState::One;
		if (ppu->rev != Revision::RP2C04_0003) ppu->wire.n_R4 = code == 12 ? TriState::Zero : TriState::One;
	}

	void ControlRegs::sim_W56RegOps()
	{
		const uint8_t code = static_cast<uint8_t>(previous_decode | (SCCX_FF1.get() == TriState::One ? 16 : 0));
		if (code == previous_w56) return;
		previous_w56 = code;
		ppu->wire.n_W6_1 = code == 6 ? TriState::Zero : TriState::One;
		ppu->wire.n_W6_2 = code == 22 ? TriState::Zero : TriState::One;
		ppu->wire.n_W5_1 = code == 5 ? TriState::Zero : TriState::One;
		ppu->wire.n_W5_2 = code == 21 ? TriState::Zero : TriState::One;
		n_W56 = previous_decode == 5 || previous_decode == 6 ? TriState::Zero : TriState::One;
	}

	void ControlRegs::sim_FirstSecond_SCCX_Write()
	{
		if (ppu->wire.RC == TriState::One || (ppu->wire.n_R2 == TriState::Zero && ppu->wire.n_DBE == TriState::Zero))
		{ SCCX_FF1.set(TriState::Zero); SCCX_FF2.set(TriState::Zero); }
		else if (n_W56 == TriState::Zero && ppu->wire.n_DBE == TriState::Zero)
			SCCX_FF2.set(NOT(SCCX_FF1.get()));
		else SCCX_FF1.set(SCCX_FF2.get());
	}

	void ControlRegs::sim_RegFFs()
	{
		const bool write0 = ppu->wire.n_W0 == TriState::Zero && ppu->wire.n_DBE == TriState::Zero;
		const bool write1 = ppu->wire.n_W1 == TriState::Zero && ppu->wire.n_DBE == TriState::Zero;
		if (ppu->wire.RC == TriState::One) { ctrl0 = 0; ctrl1 = 0; }
		else
		{
			if (write0) ctrl0 = ppu->DB & 0xfc;
			if (write1) ctrl1 = ppu->DB;
		}
		if (!write0) latched0 = ctrl0;
		if (!write1) latched1 = ctrl1;
		const uint64_t key = ctrl0 | (static_cast<uint64_t>(ctrl1) << 8) |
			(static_cast<uint64_t>(latched0) << 16) | (static_cast<uint64_t>(latched1) << 24) |
			(static_cast<uint64_t>(RenderAlwaysEnabled) << 32) | (static_cast<uint64_t>(ClippingAlwaysDisabled) << 33);
		if (key == previous_exports) { ppu->wire.BLACK = base_black; return; }
		previous_exports = key;
		ppu->wire.I1_32 = static_cast<TriState>((latched0 >> 2) & 1);
		ppu->wire.OBSEL = static_cast<TriState>(((latched0 >> 3) & 1) ^ 1);
		ppu->wire.BGSEL = static_cast<TriState>(((latched0 >> 4) & 1) ^ 1);
		ppu->wire.O8_16 = static_cast<TriState>((latched0 >> 5) & 1);
		ppu->wire.n_SLAVE = static_cast<TriState>((ctrl0 >> 6) & 1);
		const bool pal = ppu->rev == Revision::RP2C07_0 || ppu->rev == Revision::UMC_UA6538;
		ppu->wire.VBL = static_cast<TriState>(((pal ? latched0 : ctrl0) >> 7) & 1);
		ppu->wire.BnW = static_cast<TriState>(ctrl1 & 1);
		ppu->wire.n_BGCLIP = ClippingAlwaysDisabled ? TriState::One : static_cast<TriState>((latched1 >> 1) & 1);
		ppu->wire.n_OBCLIP = ClippingAlwaysDisabled ? TriState::One : static_cast<TriState>((latched1 >> 2) & 1);
		ppu->wire.BGE = RenderAlwaysEnabled ? TriState::One : static_cast<TriState>((latched1 >> 3) & 1);
		ppu->wire.OBE = RenderAlwaysEnabled ? TriState::One : static_cast<TriState>((latched1 >> 4) & 1);
		base_black = ppu->wire.BGE == TriState::Zero && ppu->wire.OBE == TriState::Zero ? TriState::One : TriState::Zero;
		ppu->wire.BLACK = base_black;
		ppu->wire.n_TR = static_cast<TriState>(((latched1 >> 5) & 1) ^ 1);
		ppu->wire.n_TG = static_cast<TriState>(((latched1 >> 6) & 1) ^ 1);
		ppu->wire.n_TB = static_cast<TriState>(((ctrl1 >> 7) & 1) ^ 1);
	}

	TriState ControlRegs::get_Frst()
	{
		return SCCX_FF1.nget();
	}

	TriState ControlRegs::get_Scnd()
	{
		return SCCX_FF1.get();
	}

	/// <summary>
	/// The CLPB/CLPO signal acquisition simulation should be done after the FSM.
	/// </summary>
	void ControlRegs::sim_CLP()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState CLIP_B = ppu->fsm.CLIP_B;
		TriState CLIP_O = ppu->fsm.CLIP_O;
		TriState BGE = ppu->wire.BGE;
		TriState OBE = ppu->wire.OBE;

		nvis_latch.set(n_VIS, n_PCLK);
		clipb_latch.set(CLIP_B, n_PCLK);
		clipo_latch.set(NOR3(nvis_latch.get(), CLIP_O, NOT(OBE)), n_PCLK);

		ppu->wire.n_CLPB = NOR3(nvis_latch.get(), clipb_latch.get(), NOT(BGE));
		ppu->wire.CLPO = clipo_latch.nget();
	}

	void ControlRegs::Debug_RenderAlwaysEnabled(bool enable)
	{
		RenderAlwaysEnabled = enable;
	}

	void ControlRegs::Debug_ClippingAlwaysDisabled(bool enable)
	{
		ClippingAlwaysDisabled = enable;
	}

	uint8_t ControlRegs::Debug_GetCTRL0()
	{
		return static_cast<uint8_t>(ctrl0 | (ppu->data_reader->sccx->get_NTH() == TriState::One ? 1 : 0) |
			(ppu->data_reader->sccx->get_NTV() == TriState::One ? 2 : 0));
	}
	uint8_t ControlRegs::Debug_GetCTRL1() { return ctrl1; }
	void ControlRegs::Debug_SetCTRL0(uint8_t value)
	{
		ppu->data_reader->sccx->set_NTH(static_cast<TriState>(value & 1));
		ppu->data_reader->sccx->set_NTV(static_cast<TriState>((value >> 1) & 1));
		ctrl0 = value & 0xfc;
	}
	void ControlRegs::Debug_SetCTRL1(uint8_t value) { ctrl1 = value; }

	/// <summary>
	/// The `/SLAVE` signal is used for EXT input terminals.
	/// </summary>
	/// <returns></returns>
	TriState ControlRegs::get_nSLAVE()
	{
		return static_cast<TriState>((ctrl0 >> 6) & 1);
	}

	/// <summary>
	/// Special BLACK signal processing for PAL-Like PPUs.
	/// </summary>
	void ControlRegs::sim_PalBLACK()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		BLACK_FF1.set(MUX(PCLK, NOT(NOT(BLACK_FF1.get())), ppu->wire.BLACK));
		BLACK_FF2.set(MUX(n_PCLK, NOT(NOT(BLACK_FF2.get())), NOT(NOT(BLACK_FF1.get()))));
		black_latch1.set(NOT(NOT(BLACK_FF2.get())), PCLK);
		black_latch2.set(black_latch1.nget(), n_PCLK);
		ppu->wire.BLACK = black_latch2.nget();
	}
}
