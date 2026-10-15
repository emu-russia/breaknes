#pragma once
#include <cassert>
#include <cstdint>

namespace M6502Core
{
	// ExactPacked command-latch banks (M6502::cmd_latch2 / M6502::cmd_latch1, v4 round 2).
	//
	// Every latch of the five command evaluators (RegsControl, ALUControl, BusControl, PC_Control,
	// FlagsControl) is one bit of the bank of its enable phase and holds exactly the DLatch's g
	// (Zero or One: every value written is the output of a NOT/NOR/NAND/AND/MUX cone over binary
	// signals). A latch that is the sole source of one command bit sits at that command bit's
	// position, so an evaluator's command bits are a masked copy of its bank, inverted where the
	// command is NOT(latch) or nget() (every latch-driven command but ADD_SB7 and n_ACIN). The
	// other latches take the positions of the command bits that are not latch outputs (BRK's
	// Z_ADL0..2, DB_P, DB_N, DB_V, AVR_V) and bits 59..62. Within one traversal no evaluator reads
	// a latch of the bank it is writing, except FlagsControl's acrc/pin inside its PHI2 block
	// (read back as the descriptor fields they were just set from) and the outputs, which are
	// formed after the bank is merged; so each evaluator writes its bank once per traversal.
	enum CmdLatch2 : unsigned
	{
		L2_YSB = 0, L2_SBY, L2_XSB, L2_SBX, L2_SADL, L2_SSB, L2_SBS, L2_SS,								// RegsControl: cmd 0..7
		L2_NDBADD = 8, L2_DBADD, L2_ZADD, L2_SBADD, L2_ADLADD, L2_ACIN1,									// ALUControl: cmd 8..12; 13
		L2_ANDS1, L2_EORS1, L2_ORS1, L2_SRS1, L2_SUMS1, L2_DAA1, L2_DSA1,									// ALUControl: 14..20 (PHI1 chain sources)
		L2_ADDSB7, L2_ADDSB06, L2_ADDADL,																	// ALUControl: cmd 21..23
		L2_SB_AC = 24, L2_AC_SB, L2_AC_DB,																	// BusControl: cmd 24..26
		L2_ADH_PCH = 27, L2_PCH_PCH, L2_PCH_ADH, L2_PCH_DB2, L2_ADL_PCL, L2_PCL_PCL, L2_PCL_ADL, L2_PCL_DB2,	// PC_Control: cmd 27..34
		L2_ADH_ABH = 35, L2_ADL_ABL,																		// BusControl: cmd 35..36
		L2_SR1 = 37, L2_ACIN2, L2_ACIN3,																	// ALUControl: 37..39
		L2_Z_ADH0 = 40, L2_Z_ADH17, L2_SB_DB, L2_SB_ADH, L2_DL_ADL, L2_DL_ADH, L2_DL_DB,					// BusControl: cmd 40..46
		L2_PDB = 47, L2_ACIN4, L2_DBZ, L2_COUT, L2_IRC, L2_DBC, L2_ACRC, L2_IRD, L2_IRI, L2_MUX1, L2_FF2, L2_ZV,	// Flags cmd 47,49,51..55,58; ALU 48,50,56,57
		L2_PCH_DB1 = 59, L2_DBN, L2_PIN, L2_BIT,															// PC_Control 59; FlagsControl 60..62
	};
	enum CmdLatch1 : unsigned
	{
		L1_REGS_NREADY = 0, L1_ALU_NREADY, L1_BUS_NREADY, L1_PC_NREADY, L1_PCL_DB1, L1_FF1,
		L1_ACIN5 = 13, L1_ANDS2, L1_EORS2, L1_ORS2, L1_SRS2, L1_SUMS2, L1_DAA2, L1_DSA2,					// cmd 13..20
		L1_SR2 = 37,
	};
	constexpr uint64_t LatchMask(unsigned k) { return UINT64_C(1) << k; }
	// The bank bit of a latch written with `value` (Zero or One; the DLatch would store the same byte).
	inline uint64_t LatchBit(BaseLogic::TriState value, unsigned k)
	{
		assert(value == BaseLogic::TriState::Zero || value == BaseLogic::TriState::One);
		return uint64_t(value) << k;
	}
	inline BaseLogic::TriState LatchGet(uint64_t bank, unsigned k) { return BaseLogic::TriState((bank >> k) & 1); }		// DLatch::get
	inline BaseLogic::TriState LatchNGet(uint64_t bank, unsigned k) { return BaseLogic::TriState(((bank >> k) & 1) ^ 1); }	// DLatch::nget
	// The bank bits each evaluator owns (every one of them is written in every traversal of its phase,
	// except L1_BUS_NREADY, whose reference latch is enabled by PHI1 inside a PHI2-only block).
	constexpr uint64_t RegsL2Mask = UINT64_C(0xFF);
	constexpr uint64_t RegsL1Mask = LatchMask(L1_REGS_NREADY);
	constexpr uint64_t AluL2Mask = UINT64_C(0xFFFF00) | LatchMask(L2_SR1) | LatchMask(L2_ACIN2) | LatchMask(L2_ACIN3) |
		LatchMask(L2_ACIN4) | LatchMask(L2_COUT) | LatchMask(L2_MUX1) | LatchMask(L2_FF2);
	constexpr uint64_t AluL1Mask = LatchMask(L1_ALU_NREADY) | LatchMask(L1_FF1) | UINT64_C(0x1FE000) | LatchMask(L1_SR2);
	constexpr uint64_t BusL2Mask = LatchMask(L2_SB_AC) | LatchMask(L2_AC_SB) | LatchMask(L2_AC_DB) | LatchMask(L2_ADH_ABH) |
		LatchMask(L2_ADL_ABL) | UINT64_C(0x7F) << 40;
	constexpr uint64_t PcL2Mask = UINT64_C(0xFF) << 27 | LatchMask(L2_PCH_DB1);
	constexpr uint64_t PcL1Mask = LatchMask(L1_PC_NREADY) | LatchMask(L1_PCL_DB1);
	constexpr uint64_t FlagsL2Mask = LatchMask(L2_PDB) | LatchMask(L2_DBZ) | LatchMask(L2_IRC) | LatchMask(L2_DBC) | LatchMask(L2_ACRC) |
		LatchMask(L2_IRD) | LatchMask(L2_IRI) | LatchMask(L2_ZV) | LatchMask(L2_DBN) | LatchMask(L2_PIN) | LatchMask(L2_BIT);
	static_assert((RegsL2Mask & AluL2Mask) == 0 && ((RegsL2Mask | AluL2Mask) & BusL2Mask) == 0 &&
		((RegsL2Mask | AluL2Mask | BusL2Mask) & PcL2Mask) == 0 && ((RegsL2Mask | AluL2Mask | BusL2Mask | PcL2Mask) & FlagsL2Mask) == 0,
		"the PHI2 latch banks of the five evaluators are disjoint");
	static_assert((RegsL2Mask | AluL2Mask | BusL2Mask | PcL2Mask | FlagsL2Mask) == UINT64_C(0x7FFFFFFFFFFFFFFF), "63 PHI2 latches");
	static_assert((RegsL1Mask & AluL1Mask) == 0 && ((RegsL1Mask | AluL1Mask) & PcL1Mask) == 0, "the PHI1 latch banks are disjoint");

	class RandomLogic
	{
		friend M6502;
		// `replay`: this is the second settling traversal of a binary-phase call; the five command
		// evaluators are then a proven replay of the first traversal and are not re-run (ExactPacked).
		template<bool Packed, int Phase = 0> void sim_Impl(bool replay = false);
#ifdef FASTSIM_SKIP_ORACLE
		void sim_ReplayOracle();
#endif
		M6502* core = nullptr;

	public:
		RegsControl* regs_control = nullptr;
		ALUControl* alu_control = nullptr;
		PC_Control* pc_control = nullptr;
		BusControl* bus_control = nullptr;
		FlagsControl* flags_control = nullptr;
		Flags* flags = nullptr;
		BranchLogic* branch_logic = nullptr;

		RandomLogic(M6502* parent);
		~RandomLogic();

		void sim();
	};
}
