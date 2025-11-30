#include "pch.h"

namespace Breaknes
{
	SignalOffsetPair core_brk_wires[] = {
		"/NMIP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_NMIP), 1,
		"/IRQP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_IRQP), 1,
		"RESP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RESP), 1,
		"BRK6E", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRK6E), 1,
		"BRK7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRK7), 1,
		"DORES", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DORES), 1,
		"/DONMI", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DONMI), 1,
	};
	size_t core_brk_wires_count = sizeof(core_brk_wires) / sizeof(core_brk_wires[0]);

	SignalOffsetPair core_disp_wires[] = {
		"IR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR), 8,
		"/PRDY", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_PRDY), 1,
		"/T2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T2), 1,
		"/T3", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T3), 1,
		"/T4", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T4), 1,
		"/T5", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T5), 1,
		"T0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::T0), 1,
		"/T0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T0), 1,
		"/T1X", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T1X), 1,
		"Z_IR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_IR), 1,
		"FETCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::FETCH), 1,
		"/ready", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_ready), 1,
		"WR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::WR), 1,
		"TRES2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::TRES2), 1,
		"ACRL1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACRL1), 1,
		"ACRL2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACRL2), 1,
		"T1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::T1), 1,
		"T6", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RMW_T6), 1,
		"T7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RMW_T7), 1,
		"ENDS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ENDS), 1,
		"ENDX", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ENDX), 1,
		"TRES1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::TRES1), 1,
		"/TRESX", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_TRESX), 1,
		"BRFW", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRFW), 1,
		"/BRTAKEN", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_BRTAKEN), 1,
	};
	size_t core_disp_wires_count = sizeof(core_disp_wires) / sizeof(core_disp_wires[0]);

	SignalOffsetPair core_alu_wires[] = {
		"n_ACIN", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_ACIN), 1,
		"n_DAA", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DAA), 1,
		"n_DSA", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DSA), 1,
		"ACR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACR), 1,
		"AVR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AVR), 1,
	};
	size_t core_alu_wires_count = sizeof(core_alu_wires) / sizeof(core_alu_wires[0]);

	SignalOffsetPair core_bops_wires[] = {
		"Y_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Y_SB), 1,
		"SB_Y", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_Y), 1,
		"X_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::X_SB), 1,
		"SB_X", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_X), 1,
		"S_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_ADL), 1,
		"S_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_SB), 1,
		"SB_S", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_S), 1,
		"S_S", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_S), 1,
		"NDB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::NDB_ADD), 1,
		"DB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_ADD), 1,
		"Z_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADD), 1,
		"SB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_ADD), 1,
		"ADL_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_ADD), 1,
		"ANDS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ANDS), 1,
		"EORS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::EORS), 1,
		"ORS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ORS), 1,
		"SRS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SRS), 1,
		"SUMS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SUMS), 1,
		"ADD_SB7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_SB7), 1,
		"ADD_SB06", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_SB06), 1,
		"ADD_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_ADL), 1,
		"SB_AC", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_AC), 1,
		"AC_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AC_SB), 1,
		"AC_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AC_DB), 1,
		"ADH_PCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADH_PCH), 1,
		"PCH_PCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_PCH), 1,
		"PCH_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_ADH), 1,
		"PCH_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_DB), 1,
		"ADL_PCL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_PCL), 1,
		"PCL_PCL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_PCL), 1,
		"PCL_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_ADL), 1,
		"PCL_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_DB), 1,
		"ADH_ABH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADH_ABH), 1,
		"ADL_ABL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_ABL), 1,
		"Z_ADL0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL0), 1,
		"Z_ADL1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL1), 1,
		"Z_ADL2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL2), 1,
		"Z_ADH0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADH0), 1,
		"Z_ADH17", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADH17), 1,
		"SB_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_DB), 1,
		"SB_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_ADH), 1,
		"DL_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_ADL), 1,
		"DL_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_ADH), 1,
		"DL_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_DB), 1,
		"n_1PC", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_1PC), 1,
	};
	size_t core_bops_wires_count = sizeof(core_bops_wires) / sizeof(core_bops_wires[0]);

	SignalOffsetPair core_fops_wires[] = {
		"P_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::P_DB), 1,
		"DB_P", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_P), 1,
		"DBZ_Z", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DBZ_Z), 1,
		"DB_N", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_N), 1,
		"IR5_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_C), 1,
		"DB_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_C), 1,
		"ACR_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACR_C), 1,
		"IR5_D", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_D), 1,
		"IR5_I", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_I), 1,
		"DB_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_V), 1,
		"AVR_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AVR_V), 1,
		"Z_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_V), 1,
	};
	size_t core_fops_wires_count = sizeof(core_fops_wires) / sizeof(core_fops_wires[0]);

	SignalOffsetPair core_regs[] = {
		"PCH", offsetof(M6502Core::UserRegs, PCH), 8,
		"PCL", offsetof(M6502Core::UserRegs, PCL), 8,
		"PCHS", offsetof(M6502Core::UserRegs, PCHS), 8,
		"PCLS", offsetof(M6502Core::UserRegs, PCLS), 8,
		"A", offsetof(M6502Core::UserRegs, A), 8,
		"X", offsetof(M6502Core::UserRegs, X), 8,
		"Y", offsetof(M6502Core::UserRegs, Y), 8,
		"S", offsetof(M6502Core::UserRegs, S), 8,
		"C_OUT", offsetof(M6502Core::UserRegs, C_OUT), 8,
		"Z_OUT", offsetof(M6502Core::UserRegs, Z_OUT), 8,
		"I_OUT", offsetof(M6502Core::UserRegs, I_OUT), 8,
		"D_OUT", offsetof(M6502Core::UserRegs, D_OUT), 8,
		"V_OUT", offsetof(M6502Core::UserRegs, V_OUT), 8,
		"N_OUT", offsetof(M6502Core::UserRegs, N_OUT), 8,
	};
	size_t core_regs_count = sizeof(core_regs) / sizeof(core_regs[0]);

	SignalOffsetPair apu_clks_wires[] = {
		"/CLK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_CLK), 1,
		"PHI0", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI0), 1,
		"PHI1", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI1), 1,
		"PHI2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI2), 1,
		"nACLK2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::nACLK2), 1,
		"ACLK1", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::ACLK1), 1,
		"/LFO1", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_LFO1), 1,
		"/LFO2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_LFO2), 1,
	};
	size_t apu_clks_wires_count = sizeof(apu_clks_wires) / sizeof(apu_clks_wires[0]);

	SignalOffsetPair apu_core_wires[] = {
		"RES", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RES), 1,
		"RESCore", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RESCore), 1,
		"RDY", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RDY), 1,
		"RDY2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RDY2), 1,
		"SYNC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SYNC), 1,
		"/M2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_M2), 1,
		"/NMI", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_NMI), 1,
		"/IRQ", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_IRQ), 1,
		"INT", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::INT), 1,
		"R/W", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RnW), 1,
	};
	size_t apu_core_wires_count = sizeof(apu_core_wires) / sizeof(apu_core_wires[0]);

	SignalOffsetPair apu_dma_wires[] = {
		"SPR/CPU", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SPR_CPU), 1,
		"SPR/PPU", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SPR_PPU), 1,
		"RW", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RW), 1,
		"RD", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RD), 1,
		"WR", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::WR), 1,
		"#DMC/AB", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_DMC_AB), 1,
		"RUNDMC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RUNDMC), 1,
		"DMCINT", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DMCINT), 1,
		"DMCRDY", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DMCRDY), 1,
	};
	size_t apu_dma_wires_count = sizeof(apu_dma_wires) / sizeof(apu_dma_wires[0]);

	SignalOffsetPair apu_regops_wires[] = {
		"/R4015", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R4015), 1,
		"/R4016", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R4016), 1,
		"/R4017", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R4017), 1,
		"/R4018", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R4018), 1,
		"/R4019", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R4019), 1,
		"/R401A", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_R401A), 1,
		"W4000", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4000), 1,
		"W4001", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4001), 1,
		"W4002", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4002), 1,
		"W4003", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4003), 1,
		"W4004", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4004), 1,
		"W4005", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4005), 1,
		"W4006", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4006), 1,
		"W4007", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4007), 1,
		"W4008", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4008), 1,
		"W400A", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W400A), 1,
		"W400B", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W400B), 1,
		"W400C", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W400C), 1,
		"W400E", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W400E), 1,
		"W400F", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W400F), 1,
		"W4010", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4010), 1,
		"W4011", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4011), 1,
		"W4012", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4012), 1,
		"W4013", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4013), 1,
		"W4014", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4014), 1,
		"W4015", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4015), 1,
		"W4016", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4016), 1,
		"W4017", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W4017), 1,
		"W401A", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::W401A), 1,
	};
	size_t apu_regops_wires_count = sizeof(apu_regops_wires) / sizeof(apu_regops_wires[0]);

	SignalOffsetPair apu_lc_wires[] = {
		"SQA/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SQA_LC), 1,
		"SQB/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SQB_LC), 1,
		"TRI/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::TRI_LC), 1,
		"RND/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RND_LC), 1,
		"NOSQA", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOSQA), 1,
		"NOSQB", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOSQB), 1,
		"NOTRI", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOTRI), 1,
		"NORND", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NORND), 1,
	};
	size_t apu_lc_wires_count = sizeof(apu_lc_wires) / sizeof(apu_lc_wires[0]);

	SignalOffsetPair apu_wires[] = {
		"DBG", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DBG), 1,
		"/DBGRD", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_DBGRD), 1,
		"LOCK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::LOCK), 1,
	};
	size_t apu_wires_count = sizeof(apu_wires) / sizeof(apu_wires[0]);

	SignalOffsetPair apu_regs[] = {
		"DBOutputLatch", offsetof(APUSim::APU_Registers, DBOutputLatch), 8,
		"DBInputLatch", offsetof(APUSim::APU_Registers, DBInputLatch), 8,
		"OutReg", offsetof(APUSim::APU_Registers, OutReg), 3,
		"LengthCounterSQA", offsetof(APUSim::APU_Registers, LengthCounterSQA), 8,
		"LengthCounterSQB", offsetof(APUSim::APU_Registers, LengthCounterSQB), 8,
		"LengthCounterTRI", offsetof(APUSim::APU_Registers, LengthCounterTRI), 8,
		"LengthCounterRND", offsetof(APUSim::APU_Registers, LengthCounterRND), 8,
		"SQFreqReg0", offsetof(APUSim::APU_Registers, SQFreqReg[0]), 11,
		"SQFreqReg1", offsetof(APUSim::APU_Registers, SQFreqReg[1]), 11,
		"SQShiftReg0", offsetof(APUSim::APU_Registers, SQShiftReg[0]), 3,
		"SQShiftReg1", offsetof(APUSim::APU_Registers, SQShiftReg[1]), 3,
		"SQFreqCounter0", offsetof(APUSim::APU_Registers, SQFreqCounter[0]), 11,
		"SQFreqCounter1", offsetof(APUSim::APU_Registers, SQFreqCounter[1]), 11,
		"SQVolumeReg0", offsetof(APUSim::APU_Registers, SQVolumeReg[0]), 4,
		"SQVolumeReg1", offsetof(APUSim::APU_Registers, SQVolumeReg[1]), 4,
		"SQDecayCounter0", offsetof(APUSim::APU_Registers, SQDecayCounter[0]), 4,
		"SQDecayCounter1", offsetof(APUSim::APU_Registers, SQDecayCounter[1]), 4,
		"SQEnvelopeCounter0", offsetof(APUSim::APU_Registers, SQEnvelopeCounter[0]), 4,
		"SQEnvelopeCounter1", offsetof(APUSim::APU_Registers, SQEnvelopeCounter[1]), 4,
		"SQSweepReg0", offsetof(APUSim::APU_Registers, SQSweepReg[0]), 3,
		"SQSweepReg1", offsetof(APUSim::APU_Registers, SQSweepReg[1]), 3,
		"SQSweepCounter0", offsetof(APUSim::APU_Registers, SQSweepCounter[0]), 3,
		"SQSweepCounter1", offsetof(APUSim::APU_Registers, SQSweepCounter[1]), 3,
		"SQDutyCounter0", offsetof(APUSim::APU_Registers, SQDutyCounter[0]), 3,
		"SQDutyCounter1", offsetof(APUSim::APU_Registers, SQDutyCounter[1]), 3,
		"TRILinearReg", offsetof(APUSim::APU_Registers, TRILinearReg), 7,
		"TRILinearCounter", offsetof(APUSim::APU_Registers, TRILinearCounter), 7,
		"TRIFreqReg", offsetof(APUSim::APU_Registers, TRIFreqReg), 11,
		"TRIFreqCounter", offsetof(APUSim::APU_Registers, TRIFreqCounter), 11,
		"TRIOutputCounter", offsetof(APUSim::APU_Registers, TRIOutputCounter), 5,
		"RNDFreqReg", offsetof(APUSim::APU_Registers, RNDFreqReg), 4,
		"RNDVolumeReg", offsetof(APUSim::APU_Registers, RNDVolumeReg), 4,
		"RNDDecayCounter", offsetof(APUSim::APU_Registers, RNDDecayCounter), 4,
		"RNDEnvelopeCounter", offsetof(APUSim::APU_Registers, RNDEnvelopeCounter), 4,
		"DPCMFreqReg", offsetof(APUSim::APU_Registers, DPCMFreqReg), 4,
		"DPCMSampleReg", offsetof(APUSim::APU_Registers, DPCMSampleReg), 8,
		"DPCMSampleCounter", offsetof(APUSim::APU_Registers, DPCMSampleCounter), 12,
		"DPCMSampleBuffer", offsetof(APUSim::APU_Registers, DPCMSampleBuffer), 8,
		"DPCMSampleBitCounter", offsetof(APUSim::APU_Registers, DPCMSampleBitCounter), 3,
		"DPCMAddressReg", offsetof(APUSim::APU_Registers, DPCMAddressReg), 8,
		"DPCMAddressCounter", offsetof(APUSim::APU_Registers, DPCMAddressCounter), 15,
		"DPCMOutput", offsetof(APUSim::APU_Registers, DPCMOutput), 7,
		"DMABuffer", offsetof(APUSim::APU_Registers, DMABuffer), 8,
		"DMAAddress", offsetof(APUSim::APU_Registers, DMAAddress), 16,
		"Status", offsetof(APUSim::APU_Registers, Status), 8,
	};
	size_t apu_regs_count = sizeof(apu_regs) / sizeof(apu_regs[0]);

	SignalOffsetPair ppu_clks_signals[] = {
		"/CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CLK), 1,
		"CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLK), 1,
		"/PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PCLK), 1,
		"PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PCLK), 1,
	};
	size_t ppu_clks_signals_count = sizeof(ppu_clks_signals) / sizeof(ppu_clks_signals[0]);

	SignalOffsetPair ppu_cpu_signals[] = {
		"RES", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RES), 1,
		"RC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RC), 1,
		"R/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RnW), 1,
		"RS[0]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[0]), 1,
		"RS[1]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[1]), 1,
		"RS[2]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[2]), 1,
		"/DBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DBE), 1,
		"/RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_RD), 1,
		"/WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_WR), 1,
		"/W6_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_1), 1,
		"/W6_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_2), 1,
		"/W5_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_1), 1,
		"/W5_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_2), 1,
		"/R7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R7), 1,
		"/W7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W7), 1,
		"/W4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W4), 1,
		"/W3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W3), 1,
		"/R2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R2), 1,
		"/W1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W1), 1,
		"/W0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W0), 1,
		"/R4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R4), 1,
	};
	size_t ppu_cpu_signals_count = sizeof(ppu_cpu_signals) / sizeof(ppu_cpu_signals[0]);

	SignalOffsetPair ppu_ctrl_signals[] = {
		"I1/32", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::I1_32), 1,
		"OBSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBSEL), 1,
		"BGSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGSEL), 1,
		"O8/16", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::O8_16), 1,
		"/SLAVE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SLAVE), 1,
		"VBL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VBL), 1,
		"B/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BnW), 1,
		"/BGCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BGCLIP), 1,
		"/OBCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBCLIP), 1,
		"BGE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGE), 1,
		"BLACK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BLACK), 1,
		"OBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBE), 1,
		"/TR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TR), 1,
		"/TG", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TG), 1,
		"/TB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TB), 1,
	};
	size_t ppu_ctrl_signals_count = sizeof(ppu_ctrl_signals) / sizeof(ppu_ctrl_signals[0]);

	SignalOffsetPair ppu_hv_signals[] = {
		"H0'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash), 1,
		"H0''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash2), 1,
		"/H1'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH1_Dash), 1,
		"H1''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H1_Dash2), 1,
		"/H2'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH2_Dash), 1,
		"H2''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H2_Dash2), 1,
		"H3''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H3_Dash2), 1,
		"H4''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H4_Dash2), 1,
		"H5''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H5_Dash2), 1,
		"EvenOddOut", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EvenOddOut), 1,
		"HC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::HC), 1,
		"VC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VC), 1,
	};
	size_t ppu_hv_signals_count = sizeof(ppu_hv_signals) / sizeof(ppu_hv_signals[0]);

	SignalOffsetPair ppu_mux_signals[] = {
		"/ZCOL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL0), 1,
		"/ZCOL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL1), 1,
		"ZCOL2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL2), 1,
		"ZCOL3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL3), 1,
		"/ZPRIO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZPRIO), 1,
		"/SPR0HIT", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SPR0HIT), 1,
		"EXT_IN0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[0]), 1,
		"EXT_IN1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[1]), 1,
		"EXT_IN2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[2]), 1,
		"EXT_IN3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[3]), 1,
		"/EXT_OUT0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[0]), 1,
		"/EXT_OUT1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[1]), 1,
		"/EXT_OUT2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[2]), 1,
		"/EXT_OUT3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[3]), 1,
	};
	size_t ppu_mux_signals_count = sizeof(ppu_mux_signals) / sizeof(ppu_mux_signals[0]);

	SignalOffsetPair ppu_spg_signals[] = {
		"BGC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[0]), 1,
		"BGC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[1]), 1,
		"BGC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[2]), 1,
		"BGC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[3]), 1,
		"FH0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[0]), 1,
		"FH1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[1]), 1,
		"FH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[2]), 1,
		"FV0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[0]), 1,
		"FV1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[1]), 1,
		"FV2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[2]), 1,
		"NTV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::NTV), 1,
		"NTH", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::NTH), 1,
		"TV0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[0]), 1,
		"TV1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[1]), 1,
		"TV2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[2]), 1,
		"TV3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[3]), 1,
		"TV4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[4]), 1,
		"TH0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[0]), 1,
		"TH1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[1]), 1,
		"TH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[2]), 1,
		"TH3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[3]), 1,
		"TH4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[4]), 1,
		"THO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[0]), 1,
		"THO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[1]), 1,
		"THO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[2]), 1,
		"THO3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[3]), 1,
		"THO4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[4]), 1,
		"TVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[0]), 1,
		"TVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[1]), 1,
		"TVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[2]), 1,
		"TVO3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[3]), 1,
		"TVO4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[4]), 1,
		"FVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[0]), 1,
		"FVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[1]), 1,
		"FVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[2]), 1,
		"/FVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[0]), 1,
		"/FVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[1]), 1,
		"/FVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[2]), 1,
	};
	size_t ppu_spg_signals_count = sizeof(ppu_spg_signals) / sizeof(ppu_spg_signals[0]);

	SignalOffsetPair ppu_cram_signals[] = {
		"#CB/DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CB_DB), 1,
		"/BW", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BW), 1,
		"#DB/CB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DB_CB), 1,
		"CGA0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CGA[0]), 1,
		"CGA1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CGA[1]), 1,
		"CGA2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CGA[2]), 1,
		"CGA3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CGA[3]), 1,
		"CGA4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CGA[4]), 1,
		"#CC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[0]), 1,
		"#CC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[1]), 1,
		"#CC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[2]), 1,
		"#CC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[3]), 1,
		"#LL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[0]), 1,
		"#LL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[1]), 1,
	};
	size_t ppu_cram_signals_count = sizeof(ppu_cram_signals) / sizeof(ppu_cram_signals[0]);

	SignalOffsetPair ppu_vram_signals[] = {
		"RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RD), 1,
		"WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::WR), 1,
		"/ALE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ALE), 1,
		"TSTEP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TSTEP), 1,
		"DB/PAR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB_PAR), 1,
		"PD/RB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD_RB), 1,
		"TH/MUX", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH_MUX), 1,
		"XRB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::XRB), 1,
	};
	size_t ppu_vram_signals_count = sizeof(ppu_vram_signals) / sizeof(ppu_vram_signals[0]);

	SignalOffsetPair ppu_wires[] = {
		
		"OB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OB), 8,
		"/SPR0_EV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SPR0_EV), 1,
		"OFETCH", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OFETCH), 1,
		"SPR_OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::SPR_OV), 1,
		"OAMCTR2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAMCTR2), 1,
		"/OAM0-7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OAM), 8,
		"OAM8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAM8), 1,
		"PD/FIFO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD_FIFO), 1,
		"OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OV), 8,
		"/WE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_WE), 1,
		"/CLPB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CLPB), 1,
		"CLPO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLPO), 1,
		"/OBJ_RD_ATTR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBJ_RD_ATTR), 1,
		"/OBJ_RD_X", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBJ_RD_X), 1,
		"/OBJ_RD_A", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBJ_RD_A), 1,
		"/OBJ_RD_B", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBJ_RD_B), 1,
		"/PA0-7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Bot), 8,
		"/PA8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[0]), 1,
		"/PA9", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[1]), 1,
		"/PA10", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[2]), 1,
		"/PA11", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[3]), 1,
		"/PA12", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[4]), 1,
		"/PA13", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[5]), 1,
		"DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB), 8,
		"PD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD), 8,
		"PrioZ", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PrioZ), 8,
		"/TX", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TX), 8,
	};
	size_t ppu_wires_count = sizeof(ppu_wires) / sizeof(ppu_wires[0]);

	SignalOffsetPair ppu_fsm_signals[] = {
		"CLIP_O", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_O), 1,
		"CLIP_B", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_B), 1,
		"S/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::S_EV), 1,
		"0/HPOS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::Z_HPOS), 1,
		"/EVAL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_EVAL), 1,
		"E/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::E_EV), 1,
		"I/OAM2", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::I_OAM2), 1,
		"OBJ_READ", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::OBJ_READ), 1,
		"/VIS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_VIS), 1,
		"#F/NT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::nF_NT), 1,
		"F/AT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_AT), 1,
		"F/TA", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TA), 1,
		"F/TB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TB), 1,
		"/FO", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_FO), 1,
		"BPORCH", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BPORCH), 1,
		"SC/CNT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::SC_CNT), 1,
		"SYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::SYNC), 1,
		"BURST", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BURST), 1,
		"VSYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VSYNC), 1,
		"/PICTURE", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_PICTURE), 1,
		"VB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VB), 1,
		"BLNK", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BLNK), 1,
		"RESCL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::RESCL), 1,
		"INT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::INT), 1,
	};
	size_t ppu_fsm_signals_count = sizeof(ppu_fsm_signals) / sizeof(ppu_fsm_signals[0]);

	SignalOffsetPair ppu_eval_signals[] = {
		"OMFG", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMFG), 1,
		"OMSTEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMSTEP), 1,
		"OMOUT", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMOUT), 1,
		"OMV", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMV), 1,
		"OSTEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OSTEP), 1,
		"ORES", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::ORES), 1,
		"TMV", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::TMV), 1,
		"OAP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OAP), 1,
		"COPY_STEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::COPY_STEP), 1,
		"DO_COPY", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::DO_COPY), 1,
		"COPY_OVF", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::COPY_OVF), 1,
		"OVZ", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OVZ), 1,
	};
	size_t ppu_eval_signals_count = sizeof(ppu_eval_signals) / sizeof(ppu_eval_signals[0]);

	SignalOffsetPair ppu_regs[] = {
		"HCounter", offsetof(PPUSim::PPU_Registers, HCounter), 16,
		"VCounter", offsetof(PPUSim::PPU_Registers, VCounter), 16,
		"CTRL0", offsetof(PPUSim::PPU_Registers, CTRL0), 8,
		"CTRL1", offsetof(PPUSim::PPU_Registers, CTRL1), 8,
		"MainOAMCounter", offsetof(PPUSim::PPU_Registers, MainOAMCounter), 8,
		"TempOAMCounter", offsetof(PPUSim::PPU_Registers, TempOAMCounter), 8,
		"OAMBuffer", offsetof(PPUSim::PPU_Registers, OAMBuffer), 8,
		"ReadBuffer", offsetof(PPUSim::PPU_Registers, ReadBuffer), 8,
		"SCC_FH", offsetof(PPUSim::PPU_Registers, SCC_FH), 8,
		"SCC_FV", offsetof(PPUSim::PPU_Registers, SCC_FV), 8,
		"SCC_NTV", offsetof(PPUSim::PPU_Registers, SCC_NTV), 8,
		"SCC_NTH", offsetof(PPUSim::PPU_Registers, SCC_NTH), 8,
		"SCC_TV", offsetof(PPUSim::PPU_Registers, SCC_TV), 8,
		"SCC_TH", offsetof(PPUSim::PPU_Registers, SCC_TH), 8,
	};
	size_t ppu_regs_count = sizeof(ppu_regs) / sizeof(ppu_regs[0]);
}
