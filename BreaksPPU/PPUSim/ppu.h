#pragma once

namespace PPUSim
{
	class PPU;

	/// <summary>
	/// A software descriptor of the current video sample. Due to the different PPU revisions the output value may differ in content, so union is used.
	/// </summary>
	union VideoOutSignal
	{
		float composite;	// For NTSC/PAL variations of PPU. A sample of the composite video signal.
	};
}

// An external class that has access to all internals. Use for unit testing.

namespace PPUSimUnitTest
{
	class UnitTest;
}

#include "bgcol.h"
#include "cram.h"
#include "dataread.h"
#include "fifo.h"
#include "fsm.h"
#include "hv.h"
#include "hv_decoder.h"
#include "mux.h"
#include "oam.h"
#include "pargen.h"
#include "regs.h"
#include "scroll_regs.h"
#include "sprite_eval.h"
#include "video_out.h"
#include "vram_ctrl.h"

namespace PPUSim
{

	/// <summary>
	/// Version of the PPU chip, including all known official and pirate variants.
	/// TODO: The full list of official revisions is not yet very clear, since the area has not been researched on our wiki.
	/// See also here: https://www.nesdev.org/wiki/User:Lidnariq/Known_PPU_revisions
	/// </summary>
	enum class Revision
	{
		Unknown = 0,

		RP2C02G,		// It is known that there are two different revisions of `G` (old and new), which differ at least in the PCLK Distribution circuit.
		RP2C02H,
		RP2C03,			// With RGB PPU at all is not clear, because few people have at home arcade machines with these PPU and few want to give chips for decap and analysis.
		RP2C03B,
		RP2C04_0001,
		RP2C04_0002,
		RP2C04_0003,
		RP2C04_0004,
		RP2C05,
		RP2C07_0,		// It is unknown how many PAL PPU revisions there were, so far it looks like only one.

		// TBD: Clone PPUs. Here you can expect a variety of zoo, but in principle they should not differ in circuitry.

		Max,
	};

	enum class InputPad
	{
		RnW = 0,
		RS0,
		RS1,
		RS2,
		n_DBE,
		CLK,
		n_RES,
		Max,
	};

	enum class OutputPad
	{
		n_INT = 0,
		ALE,
		n_RD,
		n_WR,
		Max,
	};

	class PPU
	{
		friend PPUSimUnitTest::UnitTest;
		friend HVCounterBit;
		friend HVCounter;
		friend HVDecoder;
		friend FSM;
		friend VideoOutSRBit;
		friend VideoOut;
		friend Mux;
		friend VRAM_Control;

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// See: https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/rails.md
		/// </summary>
		struct InternalWires
		{
			BaseLogic::TriState RnW;
			BaseLogic::TriState RS[3];
			BaseLogic::TriState n_DBE;
			BaseLogic::TriState n_ALE;
			BaseLogic::TriState RD;
			BaseLogic::TriState WR;
			BaseLogic::TriState CLK;
			BaseLogic::TriState n_CLK;
			BaseLogic::TriState RES;
			BaseLogic::TriState RC;
			BaseLogic::TriState PCLK;
			BaseLogic::TriState n_PCLK;
			BaseLogic::TriState n_CC[4];
			BaseLogic::TriState n_LL[2];
			
			BaseLogic::TriState n_R7;
			BaseLogic::TriState n_W7;
			BaseLogic::TriState n_W4;
			BaseLogic::TriState n_W3;
			BaseLogic::TriState n_R2;
			BaseLogic::TriState n_W1;
			BaseLogic::TriState n_W0;
			BaseLogic::TriState n_R4;

			BaseLogic::TriState I1_32;
			BaseLogic::TriState OBSEL;
			BaseLogic::TriState BGSEL;
			BaseLogic::TriState O8_16;
			BaseLogic::TriState n_SLAVE;
			BaseLogic::TriState VBL;
			BaseLogic::TriState BnW;
			BaseLogic::TriState n_BGCLIP;
			BaseLogic::TriState n_OBCLIP;
			BaseLogic::TriState BGE;
			BaseLogic::TriState BLACK;
			BaseLogic::TriState OBE;
			BaseLogic::TriState n_TR;
			BaseLogic::TriState n_TG;
			BaseLogic::TriState n_TB;

			BaseLogic::TriState H0_Dash;
			BaseLogic::TriState H0_Dash2;
			BaseLogic::TriState nH1_Dash;
			BaseLogic::TriState H1_Dash2;
			BaseLogic::TriState nH2_Dash;
			BaseLogic::TriState H2_Dash2;
			BaseLogic::TriState H3_Dash2;
			BaseLogic::TriState H4_Dash2;
			BaseLogic::TriState H5_Dash2;
			BaseLogic::TriState EvenOddOut;
			BaseLogic::TriState V_IN;
			BaseLogic::TriState HC;
			BaseLogic::TriState VC;

			BaseLogic::TriState n_ZCOL0;
			BaseLogic::TriState n_ZCOL1;
			BaseLogic::TriState ZCOL2;
			BaseLogic::TriState ZCOL3;
			BaseLogic::TriState n_ZPRIO;
			BaseLogic::TriState n_SPR0HIT;
			BaseLogic::TriState EXT_In[4];
			BaseLogic::TriState n_EXT_Out[4];

			BaseLogic::TriState I2SEV;

			BaseLogic::TriState n_PA_Top[6];
			BaseLogic::TriState BGC[4];
			BaseLogic::TriState THO[5];

			BaseLogic::TriState PAL[5];

			BaseLogic::TriState TSTEP;
			BaseLogic::TriState DB_PAR;
			BaseLogic::TriState PD_RB;
			BaseLogic::TriState TH_MUX;
			BaseLogic::TriState XRB;
		} wire{};

		/// <summary>
		/// The most important control signals of the H/V FSM.
		/// </summary>
		struct FsmCommands
		{
			BaseLogic::TriState SEV;
			BaseLogic::TriState CLIP_O;
			BaseLogic::TriState CLIP_B;
			BaseLogic::TriState ZHPOS;
			BaseLogic::TriState EVAL;
			BaseLogic::TriState EEV;
			BaseLogic::TriState IOAM2;
			BaseLogic::TriState PARO;
			BaseLogic::TriState nVIS;
			BaseLogic::TriState FNT;
			BaseLogic::TriState FTB;
			BaseLogic::TriState FTA;
			BaseLogic::TriState FAT;
			BaseLogic::TriState nFO;
			BaseLogic::TriState BPORCH;
			BaseLogic::TriState SCCNT;
			BaseLogic::TriState nHB;
			BaseLogic::TriState BURST;
			BaseLogic::TriState HSYNC;

			BaseLogic::TriState PICTURE;
			BaseLogic::TriState RESCL;
			BaseLogic::TriState VSYNC;
			BaseLogic::TriState nVSET;
			BaseLogic::TriState VB;
			BaseLogic::TriState BLNK;

			BaseLogic::TriState INT;
		} fsm{};

		Revision rev;

		void sim_PCLK();

		BaseLogic::FF Reset_FF;

		BaseLogic::DLatch pclk_1;
		BaseLogic::DLatch pclk_2;
		BaseLogic::DLatch pclk_3;
		BaseLogic::DLatch pclk_4;
		size_t pclk_counter = 0;

		HVCounter* h = nullptr;
		HVCounter* v = nullptr;
		HVDecoder* hv_dec = nullptr;
		FSM* hv_fsm = nullptr;
		VideoOut* vid_out = nullptr;
		Mux* mux = nullptr;
		VRAM_Control* vram_ctrl = nullptr;

		uint8_t DB;
		bool DB_Dirty = false;

	public:
		PPU(Revision rev);
		~PPU();

		/// <summary>
		/// Simulate one half cycle (state) of the PPU.
		/// </summary>
		/// <param name="inputs">Inputs (see `InputPad`)</param>
		/// <param name="outputs">Outputs (see `OutputPad`)</param>
		/// <param name="ext">Bidirectional EXT color bus</param>
		/// <param name="data_bus">Bidirectional CPU-PPU data bus</param>
		/// <param name="ad_bus">Bidirectional PPU-VRAM data/address bus</param>
		/// <param name="addrHi_bus">This bus carries the rest of the address lines (output)</param>
		/// <param name="vout">The output video signal.</param>
		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout);

		size_t GetPCLKCounter();

		void ResetPCLKCounter();

		const char* RevisionToStr(Revision rev);

		void SetDBBit(size_t n, BaseLogic::TriState bit_val);
	};
}
