#pragma once

// Public values and debug layouts are shared with the unchanged reference simulator.
#include "../APUSim/apu.h"

namespace APUSimFast
{
    class APU;
    using APUSim::AudioOutSignal;
    using APUSim::AudioSignalFeatures;
    using APUSim::Revision;
    using APUSim::APU_Input;
    using APUSim::APU_Output;
    using APUSim::ChipFeature;
    using APUSim::ByRevision;
    using APUSim::QuartzPal;
    using APUSim::QuartzNtsc;
    using APUSim::QuartzCustom;
    using APUSim::DividerPal;
    using APUSim::DividerNtsc;
    using APUSim::DividerCustom;
    using APUSim::MixedSquareDuty;
    using APUSim::ClickingTriangle;
    using APUSim::NoiseMalfunction;
    using APUSim::BCDPresent;
    using APUSim::DebugRegistersInsteadHalt;
    using APUSim::DebugHaltInsteadRegisters;
}

// An external class that has access to all internals. Use for unit testing.

namespace APUSimUnitTest
{
	class UnitTest;
}

#include "common.h"
#include "debug.h"
#include "clkgen.h"
#include "core.h"
#include "dma.h"
#include "dpcm.h"
#include "length.h"
#include "env.h"
#include "noise.h"
#include "square.h"
#include "triangle.h"
#include "regs.h"
#include "pads.h"
#include "dac.h"

namespace APUSimFast
{
	/// <summary>
	/// Log categories of the APU (one bit per category, owned by this component).
	/// </summary>
	using APUSim::LogCategory;
	using APUSim::Cat_Regs;
	using APUSim::Cat_DMA;
	using APUSim::Cat_Events;

	/// <summary>
	/// The category list of the APU, used by BreaksCore to register the source
	/// for the user interface (definitions flow Component -> BreaksCore).
	/// </summary>
	const Log::LogCategoryDesc* GetLogCategories(size_t& count);

	class APU
	{
		friend APUSimUnitTest::UnitTest;
		friend CoreBinding;
		friend CLKGen;
		friend RegsDecoder;
		friend LengthCounter;
		friend EnvelopeUnit;
		friend DpcmChan;
		friend NoiseChan;
		friend SquareChan;
		friend TriangleChan;
		friend DMA;
		friend Pads;
		friend DAC;

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// </summary>
		struct InternalWires
		{
			BaseLogic::TriState n_CLK;
			BaseLogic::TriState PHI0;
			BaseLogic::TriState PHI1;
			BaseLogic::TriState PHI2;
			BaseLogic::TriState RDY;			// To core
			BaseLogic::TriState RDY2;			// Default 1 (2A03)
			BaseLogic::TriState nACLK2;
			BaseLogic::TriState ACLK1;
			BaseLogic::TriState RES;
			BaseLogic::TriState n_M2;
			BaseLogic::TriState n_NMI;
			BaseLogic::TriState n_IRQ;
			BaseLogic::TriState INT;
			BaseLogic::TriState n_LFO1;
			BaseLogic::TriState n_LFO2;
			BaseLogic::TriState RnW;			// From core
			BaseLogic::TriState SPR_CPU;
			BaseLogic::TriState SPR_PPU;
			BaseLogic::TriState RW;				// To pad
			BaseLogic::TriState RD;				// To DataBus pads
			BaseLogic::TriState WR;				// To DataBus pads
			BaseLogic::TriState SYNC;			// From core
			
			BaseLogic::TriState n_DMC_AB;
			BaseLogic::TriState RUNDMC;
			BaseLogic::TriState DMCINT;
			BaseLogic::TriState DMCRDY;
			
			// RegOps
			BaseLogic::TriState n_R4015;
			BaseLogic::TriState n_R4016;
			BaseLogic::TriState n_R4017;
			BaseLogic::TriState n_R4018;
			BaseLogic::TriState n_R4019;
			BaseLogic::TriState n_R401A;
			BaseLogic::TriState W4000;
			BaseLogic::TriState W4001;
			BaseLogic::TriState W4002;
			BaseLogic::TriState W4003;
			BaseLogic::TriState W4004;
			BaseLogic::TriState W4005;
			BaseLogic::TriState W4006;
			BaseLogic::TriState W4007;
			BaseLogic::TriState W4008;
			BaseLogic::TriState W400A;
			BaseLogic::TriState W400B;
			BaseLogic::TriState W400C;
			BaseLogic::TriState W400E;
			BaseLogic::TriState W400F;
			BaseLogic::TriState W4010;
			BaseLogic::TriState W4011;
			BaseLogic::TriState W4012;
			BaseLogic::TriState W4013;
			BaseLogic::TriState W4014;
			BaseLogic::TriState W4015;
			BaseLogic::TriState W4016;
			BaseLogic::TriState W4017;
			BaseLogic::TriState W401A;

			BaseLogic::TriState SQA_LC;
			BaseLogic::TriState SQB_LC;
			BaseLogic::TriState TRI_LC;
			BaseLogic::TriState RND_LC;
			BaseLogic::TriState NOSQA;
			BaseLogic::TriState NOSQB;
			BaseLogic::TriState NOTRI;
			BaseLogic::TriState NORND;

			// Auxiliary signals associated with the Test mode, which seems to be present only on 2A03.
			BaseLogic::TriState DBG;			// from pad
			BaseLogic::TriState n_DBGRD;		// from regs decoder
			BaseLogic::TriState LOCK;
		} wire{};

		Revision rev = Revision::Unknown;
		ChipFeature fx = ChipFeature::ByRevision;

		// Instances of internal APU modules, including the core

		M6502Core::M6502* core = nullptr;
		CoreBinding* core_int = nullptr;
		CLKGen* clkgen = nullptr;
		RegsDecoder* regs = nullptr;
		LengthCounter* lc[4]{};
		DpcmChan* dpcm = nullptr;
		NoiseChan* noise = nullptr;
		SquareChan* square[2]{};
		TriangleChan* tri = nullptr;
		DMA* dma = nullptr;
		Pads* pads = nullptr;
		DAC* dac = nullptr;

		// Internal buses.

		uint32_t audio_word = 0; // SQA4, SQB4, TRI4, noise4, DMC7
		uint64_t mutation_epoch = 0;
		uint8_t DB = 0;
		bool DB_Dirty = false;

		uint16_t DMC_Addr{};
		uint16_t SPR_Addr{};
		uint16_t CPU_Addr{};		// Core to mux & regs predecoder
		uint16_t Ax{};				// Mux to pads & regs decoder

		// DAC Inputs

		BaseLogic::TriState SQA_Out[4]{};
		BaseLogic::TriState SQB_Out[4]{};
		BaseLogic::TriState TRI_Out[4]{};
		BaseLogic::TriState RND_Out[4]{};
		BaseLogic::TriState DMC_Out[8]{};		// msb is not used. This is done for the convenience of packing the value in byte.

		BaseLogic::TriState GetDBBit(size_t n);
		void SetDBBit(size_t n, BaseLogic::TriState bit_val);

		size_t aclk_counter = 0;
		size_t phi_counter = 0;

		BaseLogic::TriState PrevPHI_Core = BaseLogic::TriState::X;	// to optimize
		BaseLogic::TriState PrevPHI_SoundGen = BaseLogic::TriState::X;	// to optimize

		void sim_CoreIntegration();
		void sim_SoundGenerators();

	public:
		APU(M6502Core::M6502 *core, Revision rev, ChipFeature features=ChipFeature::ByRevision);
		~APU();

		/// <summary>
		/// Simulate one half cycle
		/// </summary>
		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t *data, uint16_t* addr, AudioOutSignal& AUX);

		/// <summary>
		/// Get the values of internal connections for debugging.
		/// </summary>
		/// <param name="wires"></param>
		void GetDebugInfo_Wires(APU_Interconnects& wires);

		/// <summary>
		/// Get values of APU registers and counters
		/// </summary>
		/// <param name="regs"></param>
		void GetDebugInfo_Regs(APU_Registers& regs);

		/// <summary>
		/// Get the APU internal signal value
		/// </summary>
		uint8_t GetDebugInfo_Wire(int ofs);
		
		/// <summary>
		/// Set the internal APU signal value
		/// </summary>
		void SetDebugInfo_Wire(int ofs, uint8_t val);

		/// <summary>
		/// Get APU register/counter value
		/// </summary>
		/// <param name="ofs"></param>
		/// <returns></returns>
		uint32_t GetDebugInfo_Reg(int ofs);
		
		/// <summary>
		/// Set APU register/counter value
		/// </summary>
		/// <param name="ofs"></param>
		/// <param name="val"></param>
		void SetDebugInfo_Reg(int ofs, uint32_t val);

		/// <summary>
		/// Turn on the digital output, instead of the analog DAC levels.
		/// </summary>
		/// <param name="enable"></param>
		void SetRAWOutput(bool enable);

		/// <summary>
		/// Turn on the normalized analog level output from the DAC.
		/// </summary>
		/// <param name="enable"></param>
		void SetNormalizedOutput(bool enable);

		/// <summary>
		/// Get the value of the ACLK cycle counter (PHI/2)
		/// </summary>
		/// <returns></returns>
		size_t GetACLKCounter();

		/// <summary>
		/// Reset the ACLK cycle counter
		/// </summary>
		void ResetACLKCounter();

		/// <summary>
		/// Get the value of the 6502 core cycle counter.
		/// </summary>
		/// <returns></returns>
		size_t GetPHICounter();

		/// <summary>
		/// Reset the 6502 core cycle counter.
		/// </summary>
		void ResetPHICounter();

		/// <summary>
		/// Get the audio signal properties of the current APU revision.
		/// </summary>
		/// <param name="features"></param>
		void GetSignalFeatures(AudioSignalFeatures& features);

		BaseLogic::TriState GetPHI2();

		/// <summary>
		/// Get the current value of the core PHI0 signal.
		/// </summary>
		BaseLogic::TriState GetPHI0();

		/// <summary>
		/// Get the current value of the core SYNC signal (1 during the opcode fetch cycle).
		/// </summary>
		BaseLogic::TriState GetSYNC();

		/// <summary>
		/// Get the value of the CPU-visible APU status register ($4015).
		/// </summary>
		uint8_t Dbg_GetStatus();

		/// <summary>
		/// Set the value of the CPU-visible APU status register ($4015). For debugging.
		/// </summary>
		void Dbg_SetStatus(uint8_t val);

		/// <summary>
		/// Set the log category mask of the APU (one bit per APUSim::LogCategory).
		/// The mask is stored in the global Log manager, so it can be set at any time,
		/// even before the APU instance exists.
		/// </summary>
		void SetLogMask(uint64_t mask);
	};
}
