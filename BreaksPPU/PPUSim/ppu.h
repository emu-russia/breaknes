#pragma once

namespace PPUSim
{
	class PPU;

#pragma pack(push, 1)

	/// <summary>
	/// A software descriptor of the current video sample. Due to the different PPU revisions the output value may differ in content, so union is used.
	/// </summary>
	union VideoOutSignal
	{
		/// <summary>
		/// Sample for "composite" PPUs
		/// </summary>
		float composite;	// For NTSC/PAL variations of PPU. A sample of the composite video signal.
		
		/// <summary>
		/// Sample for RGB PPU
		/// </summary>
		struct RGBOut
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t syncLevel;	// This field is reserved for the "SYNC" output of the RGB PPU (Sync Level).
		} RGB;

		/// <summary>
		/// Raw PPU color, which is obtained from the PPU circuits BEFORE the video signal generator.
		/// The user can switch the PPUSim video output to use "raw" color, instead of the original (Composite/RGB).
		/// </summary>
		union RAWOut
		{
			struct
			{
				unsigned CC0 : 1;	// Chroma (CB[0-3])
				unsigned CC1 : 1;
				unsigned CC2 : 1;
				unsigned CC3 : 1;
				unsigned LL0 : 1;	// Luma (CB[4-5])
				unsigned LL1 : 1;
				unsigned TR : 1;	// "Tint Red", $2001[5]
				unsigned TG : 1;	// "Tint Green", $2001[6]
				unsigned TB : 1;	// "Tint Blue", $2001[7]
			};
			uint16_t raw;
		} RAW;
	};

	/// <summary>
	/// The structure describes all the features of the signal and helps organize its rendering.
	/// </summary>
	struct VideoSignalFeatures
	{
		size_t SamplesPerPCLK;
		size_t PixelsPerScan;		// Excluding Dot Crawl
		size_t ScansPerField;
		size_t Composite;			// 1: Composite, 0: RGB
		float BlankLevel;		// IRE = 0
		float V_pk_pk;
	};

#pragma pack(pop)
}

// An external class that has access to all internals. Use for unit testing.

namespace PPUSimUnitTest
{
	class UnitTest;
}

#include "debug.h"
#include "bgcol.h"
#include "cram.h"
#include "fifo.h"
#include "fsm.h"
#include "hv.h"
#include "hv_decoder.h"
#include "mux.h"
#include "oam.h"
#include "patgen.h"
#include "par.h"
#include "regs.h"
#include "scroll_regs.h"
#include "dataread.h"
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
		friend ControlRegs;
		friend HVCounterBit;
		friend HVCounter;
		friend HVDecoder;
		friend FSM;
		friend CBBit;
		friend CRAM;
		friend VideoOutSRBit;
		friend VideoOut;
		friend Mux;
		friend OAMEval;
		friend OAMBufferBit;
		friend OAMCell;
		friend OAM;
		friend FIFOLane;
		friend FIFO;
		friend DataReader;
		friend PATGen;
		friend PAR;
		friend ScrollRegs;
		friend BGCol;
		friend RB_Bit;
		friend VRAM_Control;

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// See: https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/rails.md
		/// </summary>
		struct InternalWires
		{
			BaseLogic::TriState CLK;
			BaseLogic::TriState n_CLK;
			BaseLogic::TriState RES;
			BaseLogic::TriState RC;
			BaseLogic::TriState PCLK;
			BaseLogic::TriState n_PCLK;

			BaseLogic::TriState RnW;
			BaseLogic::TriState RS[3];
			BaseLogic::TriState n_DBE;
			BaseLogic::TriState n_ALE;
			BaseLogic::TriState n_RD;
			BaseLogic::TriState n_WR;
			BaseLogic::TriState n_W6_1;
			BaseLogic::TriState n_W6_2;
			BaseLogic::TriState n_W5_1;
			BaseLogic::TriState n_W5_2;
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

			BaseLogic::TriState OB[8];
			BaseLogic::TriState n_SPR0_EV;
			BaseLogic::TriState OFETCH;
			BaseLogic::TriState SPR_OV;
			BaseLogic::TriState OAMCTR2;
			BaseLogic::TriState n_OAM[8];
			BaseLogic::TriState OAM8;
			BaseLogic::TriState PD_FIFO;
			BaseLogic::TriState OV[8];
			BaseLogic::TriState n_WE;		// 0: OAM Write

			BaseLogic::TriState CLPB;
			BaseLogic::TriState CLPO;
			BaseLogic::TriState n_SH2;
			BaseLogic::TriState n_SH3;
			BaseLogic::TriState n_SH5;
			BaseLogic::TriState n_SH7;
			BaseLogic::TriState n_ZH;

			BaseLogic::TriState n_PA_Bot[8];
			BaseLogic::TriState n_PA_Top[6];
			BaseLogic::TriState BGC[4];
			BaseLogic::TriState FH[3];
			BaseLogic::TriState FV[3];
			BaseLogic::TriState NTV;
			BaseLogic::TriState NTH;
			BaseLogic::TriState TV[5];
			BaseLogic::TriState TH[5];
			BaseLogic::TriState THO[5];
			BaseLogic::TriState n_THO[5];
			BaseLogic::TriState TVO[5];
			BaseLogic::TriState n_TVO[5];
			BaseLogic::TriState FVO[3];
			BaseLogic::TriState n_FVO[3];
			BaseLogic::TriState PAD[13];

			BaseLogic::TriState n_CB_DB;
			BaseLogic::TriState n_BW;
			BaseLogic::TriState n_DB_CB;
			BaseLogic::TriState PAL[5];
			BaseLogic::TriState n_CC[4];
			BaseLogic::TriState n_LL[2];

			BaseLogic::TriState RD;
			BaseLogic::TriState WR;
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
			BaseLogic::TriState nFNT;
			BaseLogic::TriState FTB;
			BaseLogic::TriState FTA;
			BaseLogic::TriState FAT;
			BaseLogic::TriState nFO;
			BaseLogic::TriState BPORCH;
			BaseLogic::TriState SCCNT;
			BaseLogic::TriState nHB;
			BaseLogic::TriState BURST;
			BaseLogic::TriState HSYNC;
			BaseLogic::TriState n_PICTURE;
			BaseLogic::TriState RESCL;
			BaseLogic::TriState VSYNC;
			BaseLogic::TriState nVSET;
			BaseLogic::TriState VB;
			BaseLogic::TriState BLNK;
			BaseLogic::TriState INT;
		} fsm{};

		Revision rev = Revision::Unknown;

		void sim_PCLK();

		BaseLogic::FF Reset_FF;

		BaseLogic::DLatch pclk_1;
		BaseLogic::DLatch pclk_2;
		BaseLogic::DLatch pclk_3;
		BaseLogic::DLatch pclk_4;
		size_t pclk_counter = 0;
		BaseLogic::TriState Prev_PCLK = BaseLogic::TriState::X;

		ControlRegs* regs = nullptr;
		HVCounter* h = nullptr;
		HVCounter* v = nullptr;
		HVDecoder* hv_dec = nullptr;
		FSM* hv_fsm = nullptr;
		CRAM* cram = nullptr;
		VideoOut* vid_out = nullptr;
		Mux* mux = nullptr;
		OAMEval* eval = nullptr;
		OAM* oam = nullptr;
		FIFO* fifo = nullptr;
		VRAM_Control* vram_ctrl = nullptr;
		DataReader* data_reader = nullptr;

		// The internal PPU buses do not use the Bus Conflicts resolver because of the large Capacitance.

		uint8_t DB = 0;				// CPU I/F Data bus
		uint8_t PD = 0;				// Internal PPU Data bus

		void sim_BusInput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus);
		void sim_BusOutput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus);

		BaseLogic::DLatch extout_latch[4]{};

		bool HLEMode = false;

	public:
		PPU(Revision rev, bool HLEMode = false);
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

		BaseLogic::TriState GetDBBit(size_t n);

		void SetDBBit(size_t n, BaseLogic::TriState bit_val);

		BaseLogic::TriState GetPDBit(size_t n);

		void SetPDBit(size_t n, BaseLogic::TriState bit_val);

		void GetDebugInfo_Wires(PPU_Interconnects & wires);

		void GetDebugInfo_FSMStates(PPU_FSMStates & fsm_states);

		void GetDebugInfo_OAMEval(OAMEvalWires & wires);

		void GetDebugInfo_Regs(PPU_Registers& regs);

		size_t GetHCounter();
		size_t GetVCounter();

		/// <summary>
		/// Get the video signal properties of the current PPU revision.
		/// </summary>
		/// <param name="features"></param>
		void GetSignalFeatures(VideoSignalFeatures& features);

		/// <summary>
		/// Switch the video output to RAW Color mode.
		/// </summary>
		/// <param name="enable"></param>
		void SetRAWOutput(bool enable);

		/// <summary>
		/// Convert RAW Color to RGB. A video generator simulation circuit will be activated, which will return a sample corresponding to the current PPU revision.
		/// </summary>
		/// <param name="rawIn"></param>
		/// <param name="rgbOut"></param>
		void ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);

		uint8_t Dbg_OAMReadByte(size_t addr);
		uint8_t Dbg_TempOAMReadByte(size_t addr);
		void Dbg_OAMWriteByte(size_t addr, uint8_t val);
		void Dbg_TempOAMWriteByte(size_t addr, uint8_t val);
		uint8_t Dbg_CRAMReadByte(size_t addr);
		uint8_t Dbg_GetCRAMAddress();
		uint16_t Dbg_GetPPUAddress();
		void Dbg_RandomizePicture(bool enable);
		void Dbg_FixedPicture(bool enable);
		void Dbg_RenderAlwaysEnabled(bool enable);
	};
}
