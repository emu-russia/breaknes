#pragma once

namespace APUSim
{
#pragma pack(push,1)

	/// <summary>
	/// A software descriptor of the current audio sample.
	/// </summary>
	union AudioOutSignal
	{
		struct AnalogOut
		{
			float a;		// Analog output AUX_A in mV
			float b;		// Analog output AUX_B in mV
		} AUX;

		struct NormalizedOut
		{
			float a;	// Analog output AUX_A, normalized to the interval [0.0; 1.0]
			float b;	// Analog output AUX_B, normalized to the interval [0.0; 1.0]
		} normalized;

		/// <summary>
		/// "Raw" output from the sound generators.
		/// </summary>
		struct RAWOut
		{
			uint8_t sqa;	// Square0 channel digital output 0..15 (4 bit)
			uint8_t sqb;	// Square1 channel digital output 0..15 (4 bit)
			uint8_t tri;	// Triangle channel digital output 0..15 (4 bit)
			uint8_t rnd;	// Noise channel digital output 0..15 (4 bit)
			uint8_t dmc;	// DeltaMod channel digital output 0..127 (7 bit)
		} RAW;
	};

	/// <summary>
	/// The structure describes all the features of the signal and helps organize its rendering.
	/// </summary>
	struct AudioSignalFeatures
	{
		int32_t SampleRate;		// The sampling frequency of the audio signal (samples per "virtual" second). The audio is actually sampled every PHI (core clock) cycle.
		float AUXA_LowLevel;		// Lower signal level for AUX_A (mV)
		float AUXA_HighLevel;		// Upper signal level for AUX_A (mV)
		float AUXB_LowLevel;		// Lower signal level for AUX_B (mV)
		float AUXB_HighLevel;		// Upper signal level for AUX_B (mV)
	};

#pragma pack(pop)

	/// <summary>
	/// All known revisions of official APUs and compatible clones.
	/// </summary>
	enum class Revision
	{
		Unknown = 0,
		RP2A03G,
		RP2A03H,
		RP2A07,
		UA6527P,
		TA03NP1,
		Max,
	};

	enum class APU_Input
	{
		CLK = 0,
		n_NMI,
		n_IRQ,
		n_RES,
		DBG,
		Max,
	};

	enum class APU_Output
	{
		n_IN0 = 0,
		n_IN1,
		OUT_0,
		OUT_1,
		OUT_2,
		M2,
		RnW,
		Max,
	};
}

namespace FastAPU
{
	class FastAPU;
}

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

namespace FastAPU
{
	class FastAPU
	{
		//friend CoreBinding;
		//friend CLKGen;
		//friend RegsDecoder;
		//friend LengthCounter;
		//friend EnvelopeUnit;
		//friend DpcmChan;
		//friend NoiseChan;
		//friend SquareChan;
		//friend TriangleChan;
		//friend DMA;
		//friend Pads;
		//friend DAC;

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// </summary>
		struct InternalWires
		{
			int n_CLK;
			int PHI0;
			int PHI1;
			int PHI2;
			int RDY;			// To core
			int RDY2;			// Default 1 (2A03)
			int nACLK2;
			int ACLK1;
			int RES;
			int n_M2;
			int n_NMI;
			int n_IRQ;
			int INT;
			int n_LFO1;
			int n_LFO2;
			int RnW;			// From core
			int SPR_CPU;
			int SPR_PPU;
			int RW;				// To pad
			int RD;				// To DataBus pads
			int WR;				// To DataBus pads
			int SYNC;			// From core

			int n_DMC_AB;
			int RUNDMC;
			int DMCINT;
			int DMCRDY;

			union
			{
				struct
				{
					unsigned n_R4015 : 1;
					unsigned n_R4016 : 1;
					unsigned n_R4017 : 1;
					unsigned n_R4018 : 1;
					unsigned n_R4019 : 1;
					unsigned n_R401A : 1;
					unsigned W4000 : 1;
					unsigned W4001 : 1;
					unsigned W4002 : 1;
					unsigned W4003 : 1;
					unsigned W4004 : 1;
					unsigned W4005 : 1;
					unsigned W4006 : 1;
					unsigned W4007 : 1;
					unsigned W4008 : 1;
					unsigned W400A : 1;
					unsigned W400B : 1;
					unsigned W400C : 1;
					unsigned W400E : 1;
					unsigned W400F : 1;
					unsigned W4010 : 1;
					unsigned W4011 : 1;
					unsigned W4012 : 1;
					unsigned W4013 : 1;
					unsigned W4014 : 1;
					unsigned W4015 : 1;
					unsigned W4016 : 1;
					unsigned W4017 : 1;
					unsigned W401A : 1;
				};
				uint32_t raw_bits;
			} RegOps{};

			int SQA_LC;
			int SQB_LC;
			int TRI_LC;
			int RND_LC;
			int NOSQA;
			int NOSQB;
			int NOTRI;
			int NORND;

			// Auxiliary signals associated with the Test mode, which seems to be present only on 2A03.
			int DBG;			// from pad
			int n_DBGRD;		// from regs decoder
			int LOCK;
		} wire{};

		APUSim::Revision rev = APUSim::Revision::Unknown;

		// Instances of internal APU modules, including the core

		M6502Core::M6502* core = nullptr;
		//CoreBinding* core_int = nullptr;
		//CLKGen* clkgen = nullptr;
		//RegsDecoder* regs = nullptr;
		//LengthCounter* lc[4]{};
		//DpcmChan* dpcm = nullptr;
		//NoiseChan* noise = nullptr;
		//SquareChan* square[2]{};
		//TriangleChan* tri = nullptr;
		//DMA* dma = nullptr;
		//Pads* pads = nullptr;
		//DAC* dac = nullptr;

		// Internal buses.

		uint8_t DB = 0;

		uint16_t DMC_Addr{};
		uint16_t SPR_Addr{};
		uint16_t CPU_Addr{};		// Core to mux & regs predecoder
		uint16_t Ax{};				// Mux to pads & regs decoder

		// DAC Inputs

		uint8_t SQA_Out{};
		uint8_t SQB_Out{};
		uint8_t TRI_Out{};
		uint8_t RND_Out{};
		uint8_t DMC_Out{};

		int GetDBBit(size_t n);
		void SetDBBit(size_t n, int bit_val);

		size_t aclk_counter = 0;
		size_t phi_counter = 0;

		int PrevPHI_Core = -1;	// to optimize
		int PrevPHI_SoundGen = -1;	// to optimize

		void sim_CoreIntegration();
		void sim_SoundGenerators();

	public:
		FastAPU(M6502Core::M6502* core, APUSim::Revision rev);
		~FastAPU();

		/// <summary>
		/// Simulate one half cycle
		/// </summary>
		void sim(int inputs[], int outputs[], uint8_t* data, uint16_t* addr, APUSim::AudioOutSignal& AUX);

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
		void GetSignalFeatures(APUSim::AudioSignalFeatures& features);
	};
}
