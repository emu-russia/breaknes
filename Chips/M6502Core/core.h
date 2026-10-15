#pragma once

#ifdef M6502_PASS2_STATS
#include <vector>
#endif

namespace M6502Core
{
	class M6502;
	class RandomLogic;
}

// An external class that has access to all core internals. Use for unit testing.

namespace M6502CoreUnitTest
{
	class UnitTest;
	class PackedProof;
}

#include "decoder.h"
#include "packed_decoder.h"
#include "ir.h"
#include "predecode.h"
#include "extra_counter.h"
#include "interrupts.h"
#include "flags.h"

#include "regs_control.h"
#include "alu_control.h"
#include "bus_control.h"
#include "pc_control.h"
#include "dispatch.h"
#include "flags_control.h"
#include "branch_logic.h"
#include "random_logic.h"

#include "address_bus.h"
#include "regs.h"
#include "alu.h"
#include "pc.h"
#include "data_bus.h"

#include "debug.h"

namespace M6502Core
{
	/// <summary>
	/// Compile-time phase of an ExactPacked kernel. 0 = generic: the phase is read from the wires at
	/// run time (the reference schedule; also used for a non-binary PHI0 pad). 1 = PHI0 is Zero
	/// (PHI1 high), 2 = PHI0 is One (PHI2 high). For a binary PHI0, PHI1 = NOT(PHI0) and PHI2 = PHI0
	/// are exactly {Zero, One}, so every latch enable and phase test below is a constant per phase;
	/// the generic instantiation keeps the original run-time expressions verbatim.
	/// </summary>
	template<int Phase> struct Phi
	{
		static constexpr bool Generic = Phase == 0;
		static constexpr bool Is1 = Phase == 1;
		static constexpr bool Is2 = Phase == 2;
		// The conditions below are constexpr, so every branch folds at compile time; plain `if` keeps
		// this public header usable by the existing C++14 consumers.
		// DLatch::set(value, PHI1) / DLatch::set(value, PHI2)
		static inline void Set1(BaseLogic::DLatch& latch, BaseLogic::TriState value, BaseLogic::TriState PHI1)
		{
			if (Generic) latch.set(value, PHI1);
			else if (Is1) latch.set(value, BaseLogic::TriState::One);
		}
		static inline void Set2(BaseLogic::DLatch& latch, BaseLogic::TriState value, BaseLogic::TriState PHI2)
		{
			if (Generic) latch.set(value, PHI2);
			else if (Is2) latch.set(value, BaseLogic::TriState::One);
		}
		// `PHI2 == One`, `PHI1 == One`, `if (PHI2)` (non-zero), `if (PHI1)` (non-zero)
		static inline bool Phi2One(BaseLogic::TriState PHI2) { return Generic ? PHI2 == BaseLogic::TriState::One : Is2; }
		static inline bool Phi1One(BaseLogic::TriState PHI1) { return Generic ? PHI1 == BaseLogic::TriState::One : Is1; }
		static inline bool Phi2(BaseLogic::TriState PHI2) { return Generic ? PHI2 != BaseLogic::TriState::Zero : Is2; }
		static inline bool Phi1(BaseLogic::TriState PHI1) { return Generic ? PHI1 != BaseLogic::TriState::Zero : Is1; }
		// NOR(a, PHI2) for a binary a: Zero in PHI2, NOT(a) in PHI1 (the generic form keeps the X/Z quirks)
		static inline BaseLogic::TriState NorPhi2(BaseLogic::TriState a, BaseLogic::TriState PHI2)
		{
			return Generic ? BaseLogic::NOR(a, PHI2) : (Is2 ? BaseLogic::TriState::Zero : BaseLogic::NOT(a));
		}
	};

	/// <summary>
	/// Log categories of the M6502 core (one bit per category, owned by this component).
	/// </summary>
	enum LogCategory : uint64_t
	{
		Cat_Events = 1ULL << 0,		// RES / NMI / IRQ edge events
		Cat_Bus = 1ULL << 1,		// external bus write cycles
	};

	/// <summary>
	/// The category list of the core, used by BreaksCore to register the source
	/// for the user interface (definitions flow Component -> BreaksCore).
	/// </summary>
	const Log::LogCategoryDesc* GetLogCategories(size_t& count);

	enum class ExecutionBackend { Reference, ExactPacked };

	enum class InputPad
	{
		n_NMI = 0,
		n_IRQ,
		n_RES,
		PHI0,
		RDY,
		SO,
		Max,
	};

	enum class OutputPad
	{
		PHI1 = 0,
		PHI2,
		RnW,
		SYNC,
		Max,
	};

	class M6502
	{
		friend M6502CoreUnitTest::UnitTest;
		friend M6502CoreUnitTest::PackedProof;
		friend IR;
		friend PreDecode;
		friend ExtraCounter;
		friend BRKProcessing;
		friend Dispatcher;
		friend RandomLogic;
		friend RegsControl;
		friend ALUControl;
		friend BusControl;
		friend PC_Control;
		friend FlagsControl;
		friend BranchLogic;
		friend AddressBus;
		friend Regs;
		friend Flags;
		friend ALU;
		friend ProgramCounter;
		friend DataBus;

		BaseLogic::FF nmip_ff;
		BaseLogic::FF irqp_ff;
		BaseLogic::FF resp_ff;
		BaseLogic::DLatch irqp_latch;
		BaseLogic::DLatch resp_latch;

		BaseLogic::DLatch prdy_latch1;
		BaseLogic::DLatch prdy_latch2;

		BaseLogic::DLatch rw_latch;

		uint8_t SB = 0;
		uint8_t DB = 0;
		uint8_t ADL = 0;
		uint8_t ADH = 0;

		bool SB_Dirty = false;
		bool DB_Dirty = false;
		bool ADL_Dirty = false;
		bool ADH_Dirty = false;

		ExecutionBackend execution_backend = ExecutionBackend::Reference;
		const PackedDecoderRow* packed_decoder = nullptr;
		Decoder* decoder = nullptr;
		PreDecode* predecode = nullptr;
		IR* ir = nullptr;
		ExtraCounter* ext = nullptr;
		BRKProcessing* brk = nullptr;
		Dispatcher* disp = nullptr;
		RandomLogic* random = nullptr;

		AddressBus* addr_bus = nullptr;
		Regs* regs = nullptr;
		ALU* alu = nullptr;
		ProgramCounter* pc = nullptr;
		DataBus* data_bus = nullptr;

		BaseLogic::TriState* decoder_out = nullptr;
		size_t TxBits = 0;		// Used to optimize table indexing

		void sim_Top(BaseLogic::TriState inputs[], uint8_t* data_bus);
		template<bool Packed, int Phase = 0> void sim_TopImpl(BaseLogic::TriState inputs[], uint8_t* data_bus, bool second_pass = false);

		void sim_Bottom(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
		template<bool Packed, int Phase = 0> void sim_BottomImpl(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
		// PHI1 only: the second Bottom traversal is a proven no-op when the controls Bottom reads
		// did not change in the second Top traversal and the command pattern has none of the four
		// PHI1 feedback paths (see sim()). Signature of the controls captured after a Top traversal.
		struct BottomControls { uint64_t cmd; BaseLogic::TriState WR, T1, BRK6E, B_OUT; };
		BottomControls CaptureBottomControls();
		static bool Phi1BottomIdempotent(uint64_t cmd);
		// PHI2 only: the second Top traversal recomputes only what the first traversal left unsettled
		// (see the proof at the definition); the generic phase and PHI1 run the full traversal.
		void sim_Phi2TopSettle(uint8_t* data_bus);
		// PHI1 only: the second Top traversal re-runs the blocks downstream of the re-selected decoder
		// row and settles the rest (see the proof at the definition).
		void sim_Phi1TopSettle(uint8_t* data_bus);
		// PHI2 only: the second Bottom traversal runs only its bus-driver steps (see the proof at the
		// definition).
		void sim_Phi2BottomSettle(uint8_t* data_bus);
#ifdef FASTSIM_SKIP_ORACLE
		void sim_Phi1BottomOracle(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
		void sim_Phi2TopOracle(BaseLogic::TriState inputs[], uint8_t* data_bus);
		void sim_Phi1TopOracle(BaseLogic::TriState inputs[], uint8_t* data_bus);
		void sim_Phi2BottomOracle(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
#endif

		BaseLogic::TriState nNMI_Cache = BaseLogic::TriState::Z;
		BaseLogic::TriState nIRQ_Cache = BaseLogic::TriState::Z;
		BaseLogic::TriState nRES_Cache = BaseLogic::TriState::Z;

		bool HLE_Mode = false;		// Acceleration mode for fast applications. In this case we are cheating a little bit.

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// </summary>
		struct InternalWires
		{
			BaseLogic::TriState n_NMI;
			BaseLogic::TriState n_IRQ;
			BaseLogic::TriState n_RES;
			BaseLogic::TriState PHI0;
			BaseLogic::TriState RDY;
			BaseLogic::TriState SO;
			BaseLogic::TriState PHI1;
			BaseLogic::TriState PHI2;
			BaseLogic::TriState n_PRDY;
			BaseLogic::TriState n_NMIP;
			BaseLogic::TriState n_IRQP;
			BaseLogic::TriState RESP;
			BaseLogic::TriState n_ready;		// Global internal processor readiness signal
			BaseLogic::TriState T0;				// Processor in the T0 instruction execution cycle
			BaseLogic::TriState FETCH;
			BaseLogic::TriState Z_IR;			// "injects" the BRK operation into the instruction stream
			BaseLogic::TriState ACRL1;
			BaseLogic::TriState ACRL2;
			BaseLogic::TriState n_T0;
			BaseLogic::TriState n_T1X;
			BaseLogic::TriState WR;
			BaseLogic::TriState RMW_T6;			// Processor in cycle RMW T6 (the name T5 is the old name of the signal)
			BaseLogic::TriState RMW_T7;			// Processor in cycle RMW T7 (the name T6 is the old name of the signal)
			BaseLogic::TriState n_1PC;			// 0: Increment the program counter
			BaseLogic::TriState ENDS;
			BaseLogic::TriState ENDX;
			BaseLogic::TriState TRES1;
			BaseLogic::TriState n_TRESX;
			BaseLogic::TriState n_T2;
			BaseLogic::TriState n_T3;
			BaseLogic::TriState n_T4;
			BaseLogic::TriState n_T5;
			BaseLogic::TriState n_IMPLIED;		// 0: An instruction of type IMPLIED which has no operands (takes 1 byte in memory)
			BaseLogic::TriState n_TWOCYCLE;		// 0: A short instruction which is executed in 2 clock cycles
			BaseLogic::TriState BRK6E;
			BaseLogic::TriState BRK7;
			BaseLogic::TriState DORES;
			BaseLogic::TriState n_DONMI;
			BaseLogic::TriState BRK5_RDY;
			BaseLogic::TriState B_OUT;
			BaseLogic::TriState BRFW;			// Branch forward (whenever taken)
			BaseLogic::TriState n_BRTAKEN;		// 0: Branch taken
			BaseLogic::TriState PC_DB;			// Auxiliary output signal for the RW Control circuit that is part of the dispatcher
			BaseLogic::TriState n_ADL_PCL;
			BaseLogic::TriState n_IR5;			// /IR5 goes additionally to flags and is used in set/clear flags instructions (SEI/CLI, SED/CLD, SEC/CLC)
		} wire{};

		/// <summary>
		/// Commands for bottom and flags.
		/// </summary>
		union RandomLogic_Output
		{
			struct {
				unsigned Y_SB : 1;			// Y => SB
				unsigned SB_Y : 1;			// SB => Y
				unsigned X_SB : 1;			// X => SB
				unsigned SB_X : 1;			// SB => X
				unsigned S_ADL : 1;			// S => ADL
				unsigned S_SB : 1;			// S => SB
				unsigned SB_S : 1;			// SB => S
				unsigned S_S : 1;			// The S/S command is active if the SB/S command is inactive. This command simply "refreshes" the current state of the S register.
				unsigned NDB_ADD : 1;		// ~DB => BI
				unsigned DB_ADD : 1;		// DB => BI
				unsigned Z_ADD : 1;			// 0 => AI
				unsigned SB_ADD : 1;		// SB => AI
				unsigned ADL_ADD : 1;		// ADL => BI
				unsigned n_ACIN : 1;		// ALU input carry. The ALU also returns the result of carry (ACR) and overflow (AVR)
				unsigned ANDS : 1;			// AI & BI
				unsigned EORS : 1;			// AI ^ BI
				unsigned ORS : 1;			// AI | BI
				unsigned SRS : 1;			// >>= 1
				unsigned SUMS : 1;			// AI + BI
				unsigned n_DAA : 1;			// 0: Perform BCD correction after addition
				unsigned n_DSA : 1;			// 0: Perform BCD correction after subtraction
				unsigned ADD_SB7 : 1;		// ADD[7] => SB[7]
				unsigned ADD_SB06 : 1;		// ADD[0-6] => SB[0-6]
				unsigned ADD_ADL : 1;		// ADD => ADL
				unsigned SB_AC : 1;			// Place the value from the SB bus/BCD correction circuit into the accumulator
				unsigned AC_SB : 1;			// Place the AC value on the SB bus
				unsigned AC_DB : 1;			// Place the AC value on the DB bus
				unsigned ADH_PCH : 1;		// ADH => PCH
				unsigned PCH_PCH : 1;		// If ADH/PCH is not performed, this command is performed (refresh PCH)
				unsigned PCH_ADH : 1;		// PCH => ADH
				unsigned PCH_DB : 1;		// PCH => DB
				unsigned ADL_PCL : 1;		// ADL => PCL
				unsigned PCL_PCL : 1;		// If ADL/PCL is not performed, this command is performed (refresh PCL)
				unsigned PCL_ADL : 1;		// PCL => ADL
				unsigned PCL_DB : 1;		// PCL => DB
				unsigned ADH_ABH : 1;		// Set the high 8 bits of the external address bus, in accordance with the value of the internal bus ADH
				unsigned ADL_ABL : 1;		// Set the low-order 8 bits of the external address bus, in accordance with the value of the internal bus ADL
				unsigned Z_ADL0 : 1;		// Forced to clear the ADL[0] bit
				unsigned Z_ADL1 : 1;		// Forced to clear the ADL[1] bit
				unsigned Z_ADL2 : 1;		// Forced to clear the ADL[2] bit
				unsigned Z_ADH0 : 1;		// Forced to clear the ADH[0] bit
				unsigned Z_ADH17 : 1;		// Forced to clear the ADH[1-7] bits
				unsigned SB_DB : 1;			// Connect the SB and DB buses
				unsigned SB_ADH : 1;		// Connect SB and ADH buses
				unsigned DL_ADL : 1;		// Write the DL value to the ADL bus
				unsigned DL_ADH : 1;		// Write the DL value to the ADH bus
				unsigned DL_DB : 1;			// Exchange the value of the DL and the internal bus DB. The direction of the exchange depends on the operating mode of the external data bus (read/write)

				unsigned P_DB : 1;			// Place the value of the flags register P on the DB bus
				unsigned DB_P : 1;			// Place the DB bus value on the flag register P
				unsigned DBZ_Z : 1;			// Change the value of the flag according to the /DBZ value
				unsigned DB_N : 1;			// Change the value of the flag according to DB7 bit
				unsigned IR5_C : 1;			// Change the value of the flag according to the IR5 bit
				unsigned DB_C : 1;			// Change the value of the flag according to DB0 bit
				unsigned ACR_C : 1;			// Change the value of the flag according to the ACR value
				unsigned IR5_D : 1;			// Change the value of the flag according to the IR5 bit
				unsigned IR5_I : 1;			// Change the value of the flag according to the IR5 bit
				unsigned DB_V : 1;			// Change the value of the flag according to DB6 bit
				unsigned AVR_V : 1;			// Change the value of the flag according to the AVR value
				unsigned Z_V : 1;			// Clear flag V
			};
			uint64_t raw;
		} cmd{};

		// ExactPacked: the latches of the five command evaluators, one bit each (the latch's g), in
		// the bank of their enable phase; layout and bit helpers in random_logic.h.
		uint64_t cmd_latch2 = 0;	// enabled by PHI2
		uint64_t cmd_latch1 = 0;	// enabled by PHI1

	public:
		M6502() {}
		M6502(bool HLE, bool BCD_Hack);
		M6502(bool HLE, bool BCD_Hack, ExecutionBackend backend);
		ExecutionBackend GetExecutionBackend() const { return execution_backend; }
#ifdef M6502_PASS2_STATS
		// v4 Phase 1 instrumentation: byte images of the core and every sub-object, used to count
		// how often the second settling traversal changes anything (plan item N5).
		std::vector<std::vector<uint8_t>> SnapshotState() const;
#endif
		~M6502();

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t *addr_bus, uint8_t* data_bus);

		void getDebug(DebugInfo* info);

		void getUserRegs(UserRegs* userRegs);

		uint8_t getDebugSingle(int ofs);
		void setDebugSingle(int ofs, uint8_t val);

		uint8_t getUserRegSingle(int ofs);
		void setUserRegSingle(int ofs, uint8_t val);

		/// <summary>
		/// Set the log category mask of the core (one bit per M6502Core::LogCategory).
		/// The mask is stored in the global Log manager, so it can be set at any time,
		/// even before the core instance exists.
		/// </summary>
		void SetLogMask(uint64_t mask);
	};
}
