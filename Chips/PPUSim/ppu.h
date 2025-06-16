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
			uint8_t RED;
			uint8_t GREEN;
			uint8_t BLUE;
			uint8_t nSYNC;	// This field is reserved for the "/SYNC" output of the RGB PPU (Sync Level).
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
				unsigned Sync : 1;	// 1: Sync level
			};
			uint16_t raw;
		} RAW;
	};

	/// <summary>
	/// The structure describes all the features of the signal and helps organize its rendering.
	/// </summary>
	struct VideoSignalFeatures
	{
		int32_t SamplesPerPCLK;
		int32_t PixelsPerScan;		// Excluding Dot Crawl
		int32_t ScansPerField;
		int32_t BackPorchSize;		// BackPorch size in pixels.
		int32_t Composite;			// 1: Composite, 0: RGB
		float BlackLevel;			// IRE = 0
		float WhiteLevel;			// IRE = 110
		float SyncLevel;			// SYNC low level
		int32_t PhaseAlteration;	// 1: PAL
	};

	/// <summary>
	/// How to handle the OAM Decay effect.
	/// </summary>
	enum class OAMDecayBehavior
	{
		Keep = 0,
		Evaporate,
		ToZero,
		ToOne,
		Randomize,
	};

	/// <summary>
	/// Externally set composite video signal levels (#481). All values are in volts (V)
	/// </summary>
	struct DacLevels
	{
		float SyncLevel[2];
		float BurstLevel[2];
		float LumaLevel[4][2];
		float EmphasizedLumaLevel[4][2];
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

		// NTSC PPUs.

		RP2C02G,		// It is known that there are two different revisions of `G` (old and new), which differ at least in the PCLK Distribution circuit.
		RP2C02H,

		// RGB PPUs.
		// http://johnsarcade.com/nintendo_vs_ppu_info.php

		RP2C03B,
		RP2C03C,
		RC2C03B,
		RC2C03C,
		RP2C04_0001,
		RP2C04_0002,
		RP2C04_0003,
		RP2C04_0004,
		RC2C05_01,
		RC2C05_02,
		RC2C05_03,
		RC2C05_04,
		RC2C05_99,

		// PAL PPUs.
		// It is unknown how many PAL PPU revisions there were, so far it looks like only one.

		RP2C07_0,

		// TBD: Clone PPUs. Here you can expect a variety of zoo, but in principle they should not differ in circuitry.

		UMC_UA6538,

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
		friend CBBit_RGB;
		friend CRAM;
		friend VideoOut;
		friend Mux;
		friend OAMEval;
		friend OAMBufferBit;
		friend OAMBufferBit_RGB;
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
			BaseLogic::TriState n_CLK;			// First half of the PPU cycle
			BaseLogic::TriState CLK;			// Second half of the PPU cycle
			BaseLogic::TriState RES;			// 1: Global reset
			BaseLogic::TriState RC;				// "Registers Clear"
			BaseLogic::TriState n_PCLK;			// The PPU is in the PCLK=0 state
			BaseLogic::TriState PCLK;			// The PPU is in the PCLK=1 state

			BaseLogic::TriState RnW;			// CPU interface operating mode (read/write)
			BaseLogic::TriState RS[3];			// Selecting a register for the CPU interface
			BaseLogic::TriState n_DBE;			// 0: "Data Bus Enable", enable CPU interface
			BaseLogic::TriState n_RD;
			BaseLogic::TriState n_WR;
			BaseLogic::TriState n_W6_1;			// 0: First write to $2006
			BaseLogic::TriState n_W6_2;			// 0: Second write to $2006
			BaseLogic::TriState n_W5_1;			// 0: First write to $2005
			BaseLogic::TriState n_W5_2;			// 0: Second write to $2005
			BaseLogic::TriState n_R7;			// 0: Read $2007
			BaseLogic::TriState n_W7;			// 0: Write $2007
			BaseLogic::TriState n_W4;			// 0: Write $2004
			BaseLogic::TriState n_W3;			// 0: Write $2003
			BaseLogic::TriState n_R2;			// 0: Read $2002
			BaseLogic::TriState n_W1;			// 0: Write $2001
			BaseLogic::TriState n_W0;			// 0: Write $2000
			BaseLogic::TriState n_R4;			// 0: Read $2004

			BaseLogic::TriState I1_32;			// Increment PPU address 1/32.
			BaseLogic::TriState OBSEL;			// Selecting Pattern Table for sprites
			BaseLogic::TriState BGSEL;			// Selecting Pattern Table for background
			BaseLogic::TriState O8_16;			// Object lines 8/16 (sprite size).
			BaseLogic::TriState n_SLAVE;
			BaseLogic::TriState VBL;			// Used in the VBlank interrupt handling circuitry
			BaseLogic::TriState BnW;			// Disable Color Burst, to generate a monochrome picture
			BaseLogic::TriState n_BGCLIP;		// To generate the CLIP_B control signal
			BaseLogic::TriState n_OBCLIP;		// To generate the CLIP_O control signal
			BaseLogic::TriState BGE;
			BaseLogic::TriState BLACK;			// Active when PPU rendering is disabled (see $2001[3] è $2001[4]).
			BaseLogic::TriState OBE;
			BaseLogic::TriState n_TR;			// "Tint Red". Modifying value for Emphasis
			BaseLogic::TriState n_TG;			// "Tint Green". Modifying value for Emphasis
			BaseLogic::TriState n_TB;			// "Tint Blue". Modifying value for Emphasis

			BaseLogic::TriState H0_Dash;		// H0 signal delayed by one DLatch
			BaseLogic::TriState H0_Dash2;		// H0 signal delayed by two DLatch
			BaseLogic::TriState nH1_Dash;		// H1 signal delayed by one DLatch (in inverse logic)
			BaseLogic::TriState H1_Dash2;		// H1 signal delayed by two DLatch
			BaseLogic::TriState nH2_Dash;		// H2 signal delayed by one DLatch (in inverse logic)
			BaseLogic::TriState H2_Dash2;		// H2 signal delayed by two DLatch
			BaseLogic::TriState H3_Dash2;		// H3 signal delayed by two DLatch
			BaseLogic::TriState H4_Dash2;		// H4 signal delayed by two DLatch
			BaseLogic::TriState H5_Dash2;		// H5 signal delayed by two DLatch
			BaseLogic::TriState EvenOddOut;		// Intermediate signal for the HCounter control circuit.
			BaseLogic::TriState HC;				// "HCounter Clear"
			BaseLogic::TriState VC;				// "VCounter Clear"

			BaseLogic::TriState n_ZCOL0;		// Sprite color. The lower 2 bits are in inverse logic, the higher 2 bits are in direct logic.
			BaseLogic::TriState n_ZCOL1;
			BaseLogic::TriState ZCOL2;
			BaseLogic::TriState ZCOL3;
			BaseLogic::TriState n_ZPRIO;		// 0: Priority of sprite over background
			BaseLogic::TriState n_SPR0HIT;		// To detect a Sprite 0 Hit event
			BaseLogic::TriState EXT_In[4];		// Input color from EXT terminals
			BaseLogic::TriState n_EXT_Out[4];	// The output color for EXT terminals (inverse logic)

			BaseLogic::TriState OB[8];
			BaseLogic::TriState n_SPR0_EV;		// 0: Sprite "0" is found on the current line. To define a Sprite 0 Hit event
			BaseLogic::TriState OFETCH;
			BaseLogic::TriState SPR_OV;			// Sprites on the current line are more than 8 or the main OAM counter is full, copying is stopped
			BaseLogic::TriState OAMCTR2;
			BaseLogic::TriState n_OAM[8];		// OAM Address
			BaseLogic::TriState OAM8;			// Selects an additional (temp) OAM for addressing
			BaseLogic::TriState PD_FIFO;		// To zero the output of the H. Inv circuit
			BaseLogic::TriState OV[8];			// Bit 0-7 of the V sprite value
			BaseLogic::TriState n_WE;			// 0: OAM Write

			BaseLogic::TriState n_CLPB;			// 0: To enable background clipping
			BaseLogic::TriState CLPO;			// To enable sprite clipping
			BaseLogic::TriState n_SH2;			// Sprite H value bits. /SH2 also goes into V. Inversion.
			BaseLogic::TriState n_SH3;			// Sprite H value bits
			BaseLogic::TriState n_SH5;			// Sprite H value bits
			BaseLogic::TriState n_SH7;			// Sprite H value bits
			BaseLogic::TriState n_ZH;

			BaseLogic::TriState n_PA_Bot[8];
			BaseLogic::TriState n_PA_Top[6];
			BaseLogic::TriState BGC[4];			// Background color
			BaseLogic::TriState FH[3];			// Fine H value
			BaseLogic::TriState FV[3];			// Fine V value
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
			BaseLogic::TriState PAD[13];		// Pattern address (patgen)

			BaseLogic::TriState n_CB_DB;		// 0: CB -> DB
			BaseLogic::TriState n_BW;			// The outputs of the 4 bit CB responsible for the chrominance are controlled by the /BW control signal.
			BaseLogic::TriState n_DB_CB;		// 0: DB -> CB
			BaseLogic::TriState PAL[5];			// Palette RAM Address
			BaseLogic::TriState n_CC[4];		// 4 bits of the chrominance of the current "pixel" (inverted value)
			BaseLogic::TriState n_LL[2];		// 2 bits of the luminance of the current "pixel" (inverted value)

			BaseLogic::TriState RD;				// Output value for /RD pin
			BaseLogic::TriState WR;				// Output value for /WR pin
			BaseLogic::TriState n_ALE;			// Output value for ALE pin
			BaseLogic::TriState TSTEP;			// For PAR Counters control logic
			BaseLogic::TriState DB_PAR;			// DB -> PAR
			BaseLogic::TriState PD_RB;			// PD -> Read Buffer
			BaseLogic::TriState TH_MUX;			// Send the TH Counter value to the MUX input, which will cause the value to go into the palette as Direct Color.
			BaseLogic::TriState XRB;			// 0: Enable RB Output
		} wire{};

		/// <summary>
		/// The most important control signals of the H/V FSM.
		/// </summary>
		struct FsmCommands
		{
			BaseLogic::TriState SEV;			// "Start Sprite Evaluation"
			BaseLogic::TriState CLIP_O;			// "Clip Objects". 1: Do not show the left 8 screen points for sprites. Used to get the CLPO signal that goes into the OAM FIFO.
			BaseLogic::TriState CLIP_B;			// "Clip Background". 1: Do not show the left 8 points of the screen for the background. Used to get the /CLPB signal that goes into the Data Reader.
			BaseLogic::TriState ZHPOS;			// "Clear HPos". Clear the H counters in the sprite FIFO and start the FIFO
			BaseLogic::TriState n_EVAL;			// 0: "Sprite Evaluation in Progress"
			BaseLogic::TriState EEV;			// "End Sprite Evaluation"
			BaseLogic::TriState IOAM2;			// "Init OAM2". Initialize an additional (temp) OAM
			BaseLogic::TriState PARO;			// "PAR for Object". Selecting a tile for an object (sprite)
			BaseLogic::TriState nVIS;			// "Not Visible". The invisible part of the signal (used in sprite logic)
			BaseLogic::TriState nFNT;			// 0: "Fetch Name Table"
			BaseLogic::TriState FTB;			// "Fetch Tile B"
			BaseLogic::TriState FTA;			// "Fetch Tile A"
			BaseLogic::TriState FAT;			// "Fetch Attribute Table"
			BaseLogic::TriState nFO;			// 0: "Fetch Output Enable"
			BaseLogic::TriState BPORCH;			// "Back Porch"
			BaseLogic::TriState SCCNT;			// "Scroll Counters Control". Update the scrolling registers.
			BaseLogic::TriState nHB;			// "HBlank"
			BaseLogic::TriState BURST;			// Color Burst
			BaseLogic::TriState SYNC;			// Sync pulse
			BaseLogic::TriState n_PICTURE;		// 0: Visible part of the scan-lines
			BaseLogic::TriState RESCL;			// "Reset FF Clear" / "VBlank Clear". VBlank period end event.
			BaseLogic::TriState VSYNC;			// Vertical sync pulse
			BaseLogic::TriState nVSET;			// "VBlank Set". VBlank period start event.
			BaseLogic::TriState VB;				// Active when the invisible part of the video signal is output (used only in H Decoder)
			BaseLogic::TriState BLNK;			// Active when PPU rendering is disabled (by BLACK signal) or during VBlank
			BaseLogic::TriState INT;			// "Interrupt". PPU Interrupt
		} fsm{};

		Revision rev = Revision::Unknown;

		void sim_PCLK();

		BaseLogic::FF Reset_FF;

		BaseLogic::DLatch pclk_1;
		BaseLogic::DLatch pclk_2;
		BaseLogic::DLatch pclk_3;
		BaseLogic::DLatch pclk_4;
		BaseLogic::DLatch pclk_5;	// PAL PPU
		BaseLogic::DLatch pclk_6;	// PAL PPU
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

	public:
		PPU(Revision rev, bool VideoGen = false);
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

		/// <summary>
		/// Set one of the ways to decay OAM cells.
		/// </summary>
		void SetOamDecayBehavior(OAMDecayBehavior behavior);

		/// <summary>
		/// Returns the nature of the PPU - composite or RGB.
		/// </summary>
		bool IsComposite();

		/// <summary>
		/// Noise value (V).
		/// </summary>
		/// <param name="volts">Noise +/- value. 0 to disable.</param>
		void SetCompositeNoise(float volts);

		/// <summary>
		/// Use externally set DAC signal levels.
		/// </summary>
		/// <param name="use">true: Use externally set table, false: Use internal values (see video_out.cpp class constructor)</param>
		/// <param name="tab">signal value table</param>
		void UseExternalDacLevels(bool use, DacLevels& tab);

		uint8_t Dbg_OAMReadByte(size_t addr);
		uint8_t Dbg_TempOAMReadByte(size_t addr);
		void Dbg_OAMWriteByte(size_t addr, uint8_t val);
		void Dbg_TempOAMWriteByte(size_t addr, uint8_t val);
		uint8_t Dbg_CRAMReadByte(size_t addr);
		void Dbg_CRAMWriteByte(size_t addr, uint8_t val);
		uint8_t Dbg_GetCRAMAddress();
		uint16_t Dbg_GetPPUAddress();
		void Dbg_RenderAlwaysEnabled(bool enable);

		uint32_t Dbg_ReadRegister(int ofs);
		void Dbg_WriteRegister(int ofs, uint32_t val);
	};
}
