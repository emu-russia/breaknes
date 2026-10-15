#pragma once

#include "../PPUSim/ppu.h"

namespace PPUSimFast
{
	class PPU;
	using PPUSim::VideoOutSignal;
	using PPUSim::VideoSignalFeatures;
	using PPUSim::OAMDecayBehavior;
	using PPUSim::DacLevels;
	using PPUSim::Revision;
	using PPUSim::InputPad;
	using PPUSim::OutputPad;
	using PPUSim::LogCategory;
	using PPUSim::Cat_Regs;
	using PPUSim::Cat_Events;
}

#if defined(_MSC_VER)
#define PPUSIM_FAST_NOINLINE __declspec(noinline)
#else
#define PPUSIM_FAST_NOINLINE __attribute__((noinline))
#endif

// The pixel-edge body (v4 round 2): every module stage of the edge call is inlined into
// PPU::sim_Full so that the EdgeContext travels in registers; an out-of-line stage taking the
// context by reference would force it into memory. Link-time optimization already inlined most
// stages; this makes the remaining large ones follow. MSVC keeps its own inlining decisions
// (__forceinline on an out-of-class definition would change the linkage).
#if defined(_MSC_VER)
#define PPUSIM_FAST_EDGE_INLINE
#else
#define PPUSIM_FAST_EDGE_INLINE __attribute__((always_inline))
#endif

// An external class that has access to all internals. Use for unit testing.

namespace PPUSimUnitTest
{
	class UnitTest;
}

namespace PPUSimFast
{
	/// <summary>
	/// v4 round 2 (track PPU): the pixel-edge call's inter-module interface. Values a module
	/// produces and another module consumes within the SAME edge call travel in a local,
	/// register-resident EdgeContextT instead of through TriState byte wires in PPU::wire /
	/// PPU::fsm. The wire/fsm fields are still written by their producers (the debug accessors and
	/// the kernel tests read them); hot consumers read the context. Values a consumer needs from
	/// the PREVIOUS call (uroboros signals such as TH_MUX in the VRAM write decoder) keep reading
	/// the stored wire. CaptureEdge() rebuilds a context from the stored fields for the
	/// field-driven entry points the kernel tests call. EdgeValues holds the values themselves:
	/// plain TriState bytes, no padding.
	/// </summary>
	struct EdgeValues
	{
		// FSM commands (FSM::sim), same names as PPU::FsmCommands
		BaseLogic::TriState SEV, CLIP_O, CLIP_B, ZHPOS, n_EVAL, EEV, IOAM2, OBJ_READ, nVIS, nFNT;
		BaseLogic::TriState FTB, FTA, FAT, nFO, BPORCH, SCCNT, nHB, BURST, SYNC, n_PICTURE;
		BaseLogic::TriState RESCL, VSYNC, nVSET, VB, BLNK;

		// Delayed H (FSM::sim_DelayedH), same names/polarity as PPU::InternalWires
		BaseLogic::TriState H0_Dash, H0_Dash2, nH1_Dash, H1_Dash2, nH2_Dash, H2_Dash2, H3_Dash2, H4_Dash2, H5_Dash2;

		// Sprite fetch strobes (FIFO::sim_SpriteH)
		BaseLogic::TriState n_OBJ_RD_ATTR, n_OBJ_RD_X, n_OBJ_RD_A, n_OBJ_RD_B;

		// Clipping (ControlRegs::sim_CLP)
		BaseLogic::TriState n_CLPB, CLPO;

		// VRAM control strobes (VRAM_Control::sim)
		BaseLogic::TriState DB_PAR, TSTEP, PD_RB;

		// Counter control and the Even/Odd circuit (FSM::sim_EvenOdd / sim_CountersControl)
		BaseLogic::TriState EvenOddOut, HC, VC;

		// OAM fetch strobe (OAM::sim_OFETCH)
		BaseLogic::TriState OFETCH;

		// Sprite evaluation (ObjEval::sim)
		BaseLogic::TriState OAM8, OAMCTR2, SPR_OV, n_SPR0_EV, PD_FIFO;

		// Sprite FIFO (FIFO::sim)
		BaseLogic::TriState n_ZH, n_ZCOL0, n_ZCOL1, ZCOL2, ZCOL3, n_ZPRIO, n_SPR0HIT;

		// VRAM address mux select and read-buffer enable (VRAM_Control::sim_TH_MUX)
		BaseLogic::TriState TH_MUX, XRB;

		// Pixel luma (CRAM::sim), sampled by the video generator
		BaseLogic::TriState n_LL0, n_LL1;
	};

	/// <summary>
	/// The pixel clock phase of an edge call. For Phase 0/1 both lines are compile-time constants, so
	/// every latch enable, mux select and phase test that depends on them folds away in that
	/// instantiation of the body; Phase -1 carries the live values (field-driven test entry points).
	/// PPU::sim_PCLK always produces a binary PCLK, so the hot path only instantiates 0 and 1.
	/// </summary>
	template<int Phase> struct EdgePhase
	{
		static constexpr BaseLogic::TriState PCLK = Phase ? BaseLogic::TriState::One : BaseLogic::TriState::Zero;
		static constexpr BaseLogic::TriState n_PCLK = Phase ? BaseLogic::TriState::Zero : BaseLogic::TriState::One;
	};
	template<> struct EdgePhase<-1>
	{
		BaseLogic::TriState PCLK = BaseLogic::TriState::X, n_PCLK = BaseLogic::TriState::X;
	};
	template<int Phase> struct EdgeContextT : EdgePhase<Phase>, EdgeValues {};
	using EdgeContext = EdgeContextT<-1>;

	/// <summary>
	/// The video generator's per-call inputs (VideoOut::sim_T): the phase (a constant in the edge
	/// instantiations), the FSM commands it samples, the CRAM luma bits and whether this call
	/// re-evaluates the pixel context (input/output latches): every edge call, every call of a
	/// VideoGen-only instance and every RGB call.
	/// </summary>
	template<int Phase> struct VideoInputsT : EdgePhase<Phase>
	{
		BaseLogic::TriState SYNC, BURST, n_PICTURE, n_LL0, n_LL1;
		bool context;
	};

	// Rebuilds a live-phase context from the stored wire/fsm fields (the field-driven entry
	// points the kernel tests call, and the FASTSIM_SKIP_ORACLE check of the edge body).
	EdgeContext CaptureEdge(const PPU& ppu);
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
#include "par.h"
#include "tilecnt.h"
#include "pamux.h"
#include "regs.h"
#include "scroll_regs.h"
#include "dataread.h"
#include "obj_eval.h"
#include "video_out.h"
#include "vram_ctrl.h"


namespace PPUSimFast
{
	/// <summary>
	/// The category list of the PPU, used by BreaksCore to register the source
	/// for the user interface (definitions flow Component -> BreaksCore).
	/// </summary>
	const Log::LogCategoryDesc* GetLogCategories(size_t& count);

	// One entry of the pixel-clock divider table over (bank state, CLK, RES, PAL); see pclk.cpp.
	struct PixelClockStep { uint8_t state; BaseLogic::TriState clock; };

	class PPU
	{
		friend PPUSimUnitTest::UnitTest;
		friend EdgeContext CaptureEdge(const PPU& ppu);
		friend ControlRegs;
		friend HVCounterBit;
		friend HVCounter;
		friend HVDecoder;
		friend FSM;
		friend CRAM;
		friend VideoOut;
		friend Mux;
		friend ObjEval;
		friend OAM;
		friend FIFOLane;
		friend FIFO;
		friend DataReader;
		friend PAR;
		friend TileCnt;
		friend PAMUX;
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
			BaseLogic::TriState BLACK;			// Active when PPU rendering is disabled (see $2001[3] - $2001[4]).
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
			BaseLogic::TriState n_OBJ_RD_ATTR;		// Sprite H value bits. /OBJ_RD_ATTR also goes into V. Inversion.
			BaseLogic::TriState n_OBJ_RD_X;			// Sprite H value bits
			BaseLogic::TriState n_OBJ_RD_A;			// Sprite H value bits
			BaseLogic::TriState n_OBJ_RD_B;			// Sprite H value bits
			BaseLogic::TriState n_ZH;

			BaseLogic::TriState BGC[4];			// Background color
			BaseLogic::TriState FH[3];			// Fine H value
			BaseLogic::TriState FV[3];			// Fine V value
			BaseLogic::TriState NTV;
			BaseLogic::TriState NTVOut;
			BaseLogic::TriState NTH;
			BaseLogic::TriState NTHOut;
			BaseLogic::TriState TV[5];
			BaseLogic::TriState TH[5];
			BaseLogic::TriState THO[5];
			BaseLogic::TriState n_THO[5];
			BaseLogic::TriState TVO[5];
			BaseLogic::TriState n_TVO[5];
			BaseLogic::TriState FVO[3];
			BaseLogic::TriState n_FVO[3];

			BaseLogic::TriState n_CB_DB;		// 0: CB -> DB
			BaseLogic::TriState n_BW;			// The outputs of the 4 bit CB responsible for the chrominance are controlled by the /BW control signal.
			BaseLogic::TriState n_DB_CB;		// 0: DB -> CB
			BaseLogic::TriState CGA[5];			// Color RAM Address
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
			BaseLogic::TriState CLIP_O;			// "Clip Objects". 1: Do not show the left 8 screen points for sprites. Used to get the CLPO signal that goes into the Obj FIFO.
			BaseLogic::TriState CLIP_B;			// "Clip Background". 1: Do not show the left 8 points of the screen for the background. Used to get the /CLPB signal that goes into the Data Reader.
			BaseLogic::TriState ZHPOS;			// "Clear HPos". Clear the H counters in the sprite FIFO and start the FIFO
			BaseLogic::TriState n_EVAL;			// 0: "Sprite Evaluation in Progress"
			BaseLogic::TriState EEV;			// "End Sprite Evaluation"
			BaseLogic::TriState IOAM2;			// "Init OAM2". Initialize an additional (temp) OAM
			BaseLogic::TriState OBJ_READ;		// Common sprite fetch event, shared by many modules. Selecting a tile for an object (sprite)
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

		static const PixelClockStep* DividerTable();
		void sim_PCLK();

		BaseLogic::FF Reset_FF;

		uint8_t pclk_state = 0;
		size_t pclk_counter = 0;
		BaseLogic::TriState Prev_PCLK = BaseLogic::TriState::X;

		// Edge detection for logging (CPU I/F register accesses, reset).

		BaseLogic::TriState prev_n_WR = BaseLogic::TriState::X;
		BaseLogic::TriState prev_n_RD = BaseLogic::TriState::X;
		BaseLogic::TriState prev_RES = BaseLogic::TriState::X;

		ControlRegs* regs = nullptr;
		HVCounter* h = nullptr;
		HVCounter* v = nullptr;
		HVDecoder* hv_dec = nullptr;
		FSM* hv_fsm = nullptr;
		CRAM* cram = nullptr;
		VideoOut* vid_out = nullptr;
		Mux* mux = nullptr;
		ObjEval* eval = nullptr;
		OAM* oam = nullptr;
		FIFO* fifo = nullptr;
		VRAM_Control* vram_ctrl = nullptr;
		DataReader* data_reader = nullptr;

		// The internal PPU buses do not use the Bus Conflicts resolver because of the large Capacitance.

		uint8_t DB = 0;				// CPU I/F Data bus
		uint8_t PD = 0;				// Internal PPU Data bus

		void sim_BusInput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus);
		void sim_BusOutput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus);

		uint8_t extout_latch = 0;
		bool pixel_invoked = true;
		bool is_pal = false;			// RP2C07-0 / UMC UA6538 divider and video pipelines
		uint16_t divider_pal = 0;		// 256 for the PAL divider table half, else 0

		// v4 idle-call path (plan N3/N4, track PPU). `idle_armed` is set at the end of a full call
		// (edge calls included) when the next call can only advance the PCLK divider and the video
		// phase ring: RES and the register-clear FF are low, the CPU port is released and owes no
		// release call, and the video generator is not in RAW or noise mode and has no pending mode
		// change. Everything the idle
		// call publishes besides the sample is captured at arming time in `idle`; the live inputs
		// (no edge, port released, no reset) are re-checked per call. Every public mutator that can
		// change idle-class state calls Disarm(), so the next call takes the full path and re-arms.
		// SetDBBit/SetPDBit do not disarm: no idle-class stage reads DB (only the n_WR/n_RD/RC/W0/W1
		// strobes do), and PD is resampled from the AD bus at the start of every call.
		bool idle_armed = false;
		struct IdleContext
		{
			uint32_t outputs = 0;		// the four output pads (n_INT, ALE, n_RD, n_WR), settled wires
			uint32_t decode_raw = 0;	// RnW,RS0,RS1,RS2 input bytes the current register decode was made from
			uint8_t ext_mask = 0;		// 15: EXT_In follows the EXT pads (slave mode); 0: EXT_In is all Zero
			uint8_t ext_out = 0;		// EXT drive value (master mode)
			uint8_t ad = 0;				// AD drive value while RD is low
			uint8_t addr_hi = 0;		// PA8-13 pads
			bool drive_ext = false, drive_ad = false;
		} idle{};
		void ArmIdle(const BaseLogic::TriState inputs[], const BaseLogic::TriState outputs[], const VideoOutSignal& vout);
		void Disarm() { idle_armed = false; }
		// The idle call peeks at the live class conditions (port released, RES low, a binary clock
		// whose divider step leaves PCLK unchanged) and commits only when they hold; otherwise it
		// returns false without side effects and sim_Full() runs. Kept call-free so that the
		// dispatcher PPU::sim needs no frame on the idle path; the full path is out of line.
		bool sim_IdleCall(const BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout);
		PPUSIM_FAST_NOINLINE void sim_Full(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout);
		// The pixel-edge body, specialized on the PCLK polarity of the edge (see EdgePhase).
		template<int Phase> void sim_Edge(VideoOutSignal& vout);
#ifdef FASTSIM_SKIP_ORACLE
		void sim_IdleOracle(const BaseLogic::TriState inputs[], const BaseLogic::TriState outputs[], uint8_t ext_before, uint8_t ext_after, uint8_t data_after, uint8_t ad_before, uint8_t ad_after, uint8_t addr_hi_after, VideoOutSignal& vout);
		alignas(VideoOut) unsigned char oracle_video_before[sizeof(VideoOut)];
		uint8_t oracle_pclk_before = 0;
#endif
#ifdef FASTSIM_CALL_CLASSES
		bool stats_prev_edge = false;
#endif

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

		/// <summary>
		/// Get the VBlank status flag (bit 7 of $2002).
		/// </summary>
		BaseLogic::TriState Dbg_GetVBLFlag();

		/// <summary>
		/// Set the log category mask of the PPU (one bit per PPUSim::LogCategory).
		/// The mask is stored in the global Log manager, so it can be set at any time,
		/// even before the PPU instance exists.
		/// </summary>
		void SetLogMask(uint64_t mask);
	};

	inline EdgeContext CaptureEdge(const PPU& p)
	{
		EdgeContext c{};
		c.PCLK = p.wire.PCLK; c.n_PCLK = p.wire.n_PCLK;
		c.SEV = p.fsm.SEV; c.CLIP_O = p.fsm.CLIP_O; c.CLIP_B = p.fsm.CLIP_B; c.ZHPOS = p.fsm.ZHPOS; c.n_EVAL = p.fsm.n_EVAL;
		c.EEV = p.fsm.EEV; c.IOAM2 = p.fsm.IOAM2; c.OBJ_READ = p.fsm.OBJ_READ; c.nVIS = p.fsm.nVIS; c.nFNT = p.fsm.nFNT;
		c.FTB = p.fsm.FTB; c.FTA = p.fsm.FTA; c.FAT = p.fsm.FAT; c.nFO = p.fsm.nFO; c.BPORCH = p.fsm.BPORCH;
		c.SCCNT = p.fsm.SCCNT; c.nHB = p.fsm.nHB; c.BURST = p.fsm.BURST; c.SYNC = p.fsm.SYNC; c.n_PICTURE = p.fsm.n_PICTURE;
		c.RESCL = p.fsm.RESCL; c.VSYNC = p.fsm.VSYNC; c.nVSET = p.fsm.nVSET; c.VB = p.fsm.VB; c.BLNK = p.fsm.BLNK;
		c.H0_Dash = p.wire.H0_Dash; c.H0_Dash2 = p.wire.H0_Dash2; c.nH1_Dash = p.wire.nH1_Dash; c.H1_Dash2 = p.wire.H1_Dash2;
		c.nH2_Dash = p.wire.nH2_Dash; c.H2_Dash2 = p.wire.H2_Dash2; c.H3_Dash2 = p.wire.H3_Dash2; c.H4_Dash2 = p.wire.H4_Dash2; c.H5_Dash2 = p.wire.H5_Dash2;
		c.n_OBJ_RD_ATTR = p.wire.n_OBJ_RD_ATTR; c.n_OBJ_RD_X = p.wire.n_OBJ_RD_X; c.n_OBJ_RD_A = p.wire.n_OBJ_RD_A; c.n_OBJ_RD_B = p.wire.n_OBJ_RD_B;
		c.n_CLPB = p.wire.n_CLPB; c.CLPO = p.wire.CLPO;
		c.DB_PAR = p.wire.DB_PAR; c.TSTEP = p.wire.TSTEP; c.PD_RB = p.wire.PD_RB;
		c.EvenOddOut = p.wire.EvenOddOut; c.HC = p.wire.HC; c.VC = p.wire.VC;
		c.OFETCH = p.wire.OFETCH;
		c.OAM8 = p.wire.OAM8; c.OAMCTR2 = p.wire.OAMCTR2; c.SPR_OV = p.wire.SPR_OV; c.n_SPR0_EV = p.wire.n_SPR0_EV; c.PD_FIFO = p.wire.PD_FIFO;
		c.n_ZH = p.wire.n_ZH; c.n_ZCOL0 = p.wire.n_ZCOL0; c.n_ZCOL1 = p.wire.n_ZCOL1; c.ZCOL2 = p.wire.ZCOL2; c.ZCOL3 = p.wire.ZCOL3;
		c.n_ZPRIO = p.wire.n_ZPRIO; c.n_SPR0HIT = p.wire.n_SPR0HIT;
		c.TH_MUX = p.wire.TH_MUX; c.XRB = p.wire.XRB;
		c.n_LL0 = p.wire.n_LL[0]; c.n_LL1 = p.wire.n_LL[1];
		return c;
	}
}
