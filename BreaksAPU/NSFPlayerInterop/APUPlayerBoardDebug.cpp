// Debugging capabilities of APUPlayer (very limited as we do not have a real 6502)

#include "pch.h"

using namespace BaseLogic;

#define WRAM_NAME "WRAM"
#define APU_WIRES_CATEGORY "APU Wires"
#define APU_REGS_CATEGORY "APU Regs"
#define BOARD_CATEGORY "APUPlayer Board"

namespace NSFPlayer
{
	void APUPlayerBoard::AddBoardMemDescriptors()
	{
		// WRAM

		MemDesciptor* wramRegion = new MemDesciptor;
		memset(wramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(wramRegion->name, sizeof(wramRegion->name), WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);
	}

	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	static SignalOffsetPair apu_wires[] = {
		"/CLK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_CLK), 1,
		"PHI0", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI0), 1,
		"PHI1", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI1), 1,
		"PHI2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::PHI2), 1,
		"RDY", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RDY), 1,
		"RDY2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RDY2), 1,
		"ACLK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::ACLK), 1,
		"/ACLK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_ACLK), 1,
		"RES", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RES), 1,
		"RESCore", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RESCore), 1,
		"/M2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_M2), 1,
		"/NMI", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_NMI), 1,
		"/IRQ", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_IRQ), 1,
		"INT", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::INT), 1,
		"/LFO1", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_LFO1), 1,
		"/LFO2", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_LFO2), 1,
		"R/W", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RnW), 1,
		"SPR/CPU", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SPR_CPU), 1,
		"SPR/PPU", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SPR_PPU), 1,
		"RW", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RW), 1,
		"RD", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RD), 1,
		"WR", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::WR), 1,
		"SYNC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SYNC), 1,
		"#DMC/AB", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_DMC_AB), 1,
		"RUNDMC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RUNDMC), 1,
		"DMCINT", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DMCINT), 1,
		"DMCRDY", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DMCRDY), 1,
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
		"SQA/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SQA_LC), 1,
		"SQB/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::SQB_LC), 1,
		"TRI/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::TRI_LC), 1,
		"RND/LC", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::RND_LC), 1,
		"NOSQA", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOSQA), 1,
		"NOSQB", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOSQB), 1,
		"NOTRI", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NOTRI), 1,
		"NORND", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::NORND), 1,
		"DBG", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::DBG), 1,
		"/DBGRD", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::n_DBGRD), 1,
		"LOCK", offsetof(APUSim::APU_Interconnects, APUSim::APU_Interconnects::LOCK), 1,
	};

	static SignalOffsetPair apu_regs[] = {
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

	static SignalOffsetPair board_signals[] = {
		"BoardCLK", offsetof(APUBoardDebugInfo, CLK), 1,
		"ABus", offsetof(APUBoardDebugInfo, ABus), 16,
		"DBus", offsetof(APUBoardDebugInfo, DBus), 8,
	};

	void APUPlayerBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < _countof(apu_wires); n++)
		{
			SignalOffsetPair* sp = &apu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), APU_WIRES_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(apu_regs); n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), APU_REGS_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APURegs, entry, GetApuRegsDebugInfo, SetApuRegsDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), BOARD_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Board, entry, GetBoardDebugInfo, SetBoardDebugInfo, this);
		}
	}

	uint8_t APUPlayerBoard::DumpWRAM(void* opaque, size_t addr)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;
		return board->wram->Dbg_ReadByte(addr);
	}

	void APUPlayerBoard::WriteWRAM(void* opaque, size_t addr, uint8_t data)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;
		board->wram->Dbg_WriteByte(addr, data);
	}

	uint32_t APUPlayerBoard::GetApuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < _countof(apu_wires); n++)
		{
			SignalOffsetPair* sp = &apu_wires[n];

			if (!strcmp(sp->name, entry->name))
			{
				APUSim::APU_Interconnects wires{};
				board->apu->GetDebugInfo_Wires(wires);

				uint8_t* ptr = (uint8_t*)&wires;
				return ptr[sp->offset];
			}
		}

		return 0;
	}

	uint32_t APUPlayerBoard::GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < _countof(apu_regs); n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				return board->apu->GetDebugInfo_Reg((int)sp->offset);
			}
		}

		return 0;
	}

	void APUPlayerBoard::SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void APUPlayerBoard::SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < _countof(apu_regs); n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->apu->SetDebugInfo_Reg((int)sp->offset, value);
			}
		}
	}

	void APUPlayerBoard::GetDebugInfo(APUBoardDebugInfo& info)
	{
		info.CLK = CLK;
		info.ABus = addr_bus;
		info.DBus = data_bus;
	}

	uint32_t APUPlayerBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				APUBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void APUPlayerBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not used
	}
}
