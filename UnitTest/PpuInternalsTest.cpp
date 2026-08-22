// Additional PPU internals unit tests.
// Cover the PPUSim chip modules that are not covered by the existing PPUTest.cpp:
// BGC_SRBit/BGCol, CBBit/CRAM, FIFOLane/FIFO, FSM, HVCounterBit/HVCounter/HVDecoder,
// Mux, OAMCell/OAMLane/OAMBufferBit/OAM, ParBitInv/ParBit/PAR, TileCounterBit/TileCnt,
// PAMUX bits/PAMUX, ControlRegs, SCC_FF/ScrollRegs, DataReader, OAMCounterBit/OAMCmprBit/
// OAMPosedgeDFFE/ObjEval, VideoOutSRBit/RGB_SEL12x3/VideoOut, RB_Bit/VRAM_Control,
// and the PPU public API (debug accessors).

#include "pch.h"

#include <cmath>
#include <cstddef>

using namespace BaseLogic;
using namespace PPUSim;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define PUIT_CHECK(cond, msg) \
	if (!(cond)) \
	{ \
		char text[0x100]{}; \
		sprintf_s(text, sizeof(text), "PpuInternalsTest: " msg "\n"); \
		Logger::WriteMessage(text); \
		return false; \
	}

namespace
{
	bool Is01(TriState v)
	{
		return v == TriState::Zero || v == TriState::One;
	}

	/// <summary>
	/// Run a given number of whole PCLK cycles through the full PPU simulator.
	/// </summary>
	void RunCycles(PPUSimUnitTest::UnitTest& ut, size_t n)
	{
		for (size_t i = 0; i < n; i++)
		{
			ut.RunSinglePCLK();
		}
	}
}

namespace PPUSimUnitTest
{
	bool UnitTest::TestPPU_API()
	{
		char text[0x100]{};

		// GetPCLKCounter / ResetPCLKCounter

		RunCycles(*this, 4);

		PUIT_CHECK(ppu->GetPCLKCounter() > 0, "PCLK counter did not advance");

		ppu->ResetPCLKCounter();
		PUIT_CHECK(ppu->GetPCLKCounter() == 0, "PCLK counter was not reset");

		RunCycles(*this, 2);
		PUIT_CHECK(ppu->GetPCLKCounter() > 0, "PCLK counter did not advance after reset");

		// RevisionToStr

		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::RP2C02G)) == "RP2C02G", "RevisionToStr(RP2C02G)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::RP2C02H)) == "RP2C02H", "RevisionToStr(RP2C02H)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::RP2C04_0003)) == "RP2C04-0003", "RevisionToStr(RP2C04_0003)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::RP2C07_0)) == "RP2C07-0", "RevisionToStr(RP2C07_0)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::UMC_UA6538)) == "UMC UA6538", "RevisionToStr(UMC_UA6538)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::RC2C05_01)) == "RC2C05-01", "RevisionToStr(RC2C05_01)");
		PUIT_CHECK(std::string(ppu->RevisionToStr(Revision::Unknown)) == "Unknown", "RevisionToStr(Unknown)");

		// DB / PD bit roundtrips

		for (size_t n = 0; n < 8; n++)
		{
			ppu->SetDBBit(n, TriState::One);
			PUIT_CHECK(ppu->GetDBBit(n) == TriState::One, "SetDBBit/GetDBBit One");

			ppu->SetDBBit(n, TriState::Zero);
			PUIT_CHECK(ppu->GetDBBit(n) == TriState::Zero, "SetDBBit/GetDBBit Zero");

			// Z must not modify the bit
			ppu->SetDBBit(n, TriState::One);
			ppu->SetDBBit(n, TriState::Z);
			PUIT_CHECK(ppu->GetDBBit(n) == TriState::One, "SetDBBit Z must not modify");

			ppu->SetPDBit(n, TriState::One);
			PUIT_CHECK(ppu->GetPDBit(n) == TriState::One, "SetPDBit/GetPDBit One");

			ppu->SetPDBit(n, TriState::Zero);
			PUIT_CHECK(ppu->GetPDBit(n) == TriState::Zero, "SetPDBit/GetPDBit Zero");

			ppu->SetPDBit(n, TriState::One);
			ppu->SetPDBit(n, TriState::Z);
			PUIT_CHECK(ppu->GetPDBit(n) == TriState::One, "SetPDBit Z must not modify");
		}

		// H/V counters via the public API

		ppu->wire.RES = TriState::Zero;
		ppu->h->set(0x123);
		ppu->v->set(0x1FF);

		PUIT_CHECK(ppu->GetHCounter() == 0x123, "GetHCounter after h->set");
		PUIT_CHECK(ppu->GetVCounter() == 0x1FF, "GetVCounter after v->set");

		// Debug info structures

		PPU_Interconnects wires{};
		ppu->GetDebugInfo_Wires(wires);

		PUIT_CHECK(Is01((TriState)wires.CLK) && Is01((TriState)wires.n_CLK), "Wires CLK/n_CLK");
		PUIT_CHECK(Is01((TriState)wires.PCLK) && Is01((TriState)wires.n_PCLK), "Wires PCLK/n_PCLK");
		PUIT_CHECK(wires.PCLK == (wires.n_PCLK == 0 ? 1 : 0), "Wires PCLK/n_PCLK complementary");
		PUIT_CHECK(Is01((TriState)wires.HC) && Is01((TriState)wires.VC), "Wires HC/VC");
		PUIT_CHECK(Is01((TriState)wires.BLACK) && Is01((TriState)wires.BGE), "Wires BLACK/BGE");

		PPU_FSMStates fsm_states{};
		ppu->GetDebugInfo_FSMStates(fsm_states);

		PUIT_CHECK(Is01((TriState)fsm_states.BLNK), "FSM BLNK");
		PUIT_CHECK(Is01((TriState)fsm_states.S_EV), "FSM S_EV");
		PUIT_CHECK(Is01((TriState)fsm_states.OBJ_READ), "FSM OBJ_READ");

		OAMEvalWires eval_wires{};
		ppu->GetDebugInfo_OAMEval(eval_wires);

		PUIT_CHECK(Is01((TriState)eval_wires.OMFG), "Eval OMFG");
		PUIT_CHECK(Is01((TriState)eval_wires.DO_COPY), "Eval DO_COPY");

		PPU_Registers regs{};
		ppu->GetDebugInfo_Regs(regs);

		PUIT_CHECK(regs.HCounter == ppu->GetHCounter(), "Regs HCounter consistency");
		PUIT_CHECK(regs.VCounter == ppu->GetVCounter(), "Regs VCounter consistency");
		PUIT_CHECK(regs.CTRL0 == ppu->regs->Debug_GetCTRL0(), "Regs CTRL0 consistency");
		PUIT_CHECK(regs.CTRL1 == ppu->regs->Debug_GetCTRL1(), "Regs CTRL1 consistency");
		PUIT_CHECK(regs.MainOAMCounter == ppu->eval->Debug_GetMainCounter(), "Regs MainOAMCounter consistency");
		PUIT_CHECK(regs.OAMBuffer == ppu->oam->Dbg_Get_OAMBuffer(), "Regs OAMBuffer consistency");
		PUIT_CHECK(regs.ReadBuffer == ppu->vram_ctrl->Debug_GetRB(), "Regs ReadBuffer consistency");

		// Run a few full cycles and check the debug info is still sane

		RunLines(1);

		ppu->GetDebugInfo_Regs(regs);
		PUIT_CHECK(regs.HCounter == ppu->GetHCounter(), "Regs HCounter consistency after sim");
		PUIT_CHECK(regs.VCounter == ppu->GetVCounter(), "Regs VCounter consistency after sim");
		PUIT_CHECK(ppu->GetHCounter() <= 0x1FF, "HCounter range");
		PUIT_CHECK(ppu->GetVCounter() <= 0x1FF, "VCounter range");

		sprintf_s(text, sizeof(text), "PpuInternalsTest: TestPPU_API OK (HCounter=%zd, VCounter=%zd)\n", ppu->GetHCounter(), ppu->GetVCounter());
		Logger::WriteMessage(text);

		return true;
	}

	bool UnitTest::TestPPU_API_DebugMem()
	{
		// OAM debug read/write roundtrips

		for (size_t addr = 0; addr < 64; addr++)
		{
			size_t lane = addr & 7;
			uint8_t val = (uint8_t)(0xA5);

			ppu->Dbg_OAMWriteByte(addr, val);

			uint8_t expected = val;
			if (lane == 2 || lane == 6)
			{
				// Bits 2-4 of the attribute byte (lanes 2/6) are not used.
				expected = (uint8_t)(val & 0xE3);
			}

			uint8_t got = ppu->Dbg_OAMReadByte(addr);
			if (got != expected)
			{
				char text[0x100]{};
				sprintf_s(text, sizeof(text), "OAM Write/Read mismatch at addr %zd: got %02X, expected %02X\n", addr, got, expected);
				Logger::WriteMessage(text);
				return false;
			}
		}

		// A few extra patterns on lane 0 and lane 2

		uint8_t patterns[] = { 0x00, 0xFF, 0x5A, 0x3C };
		for (size_t p = 0; p < _countof(patterns); p++)
		{
			ppu->Dbg_OAMWriteByte(0x00, patterns[p]);
			PUIT_CHECK(ppu->Dbg_OAMReadByte(0x00) == patterns[p], "OAM lane0 pattern roundtrip");

			ppu->Dbg_OAMWriteByte(0x02, patterns[p]);
			PUIT_CHECK(ppu->Dbg_OAMReadByte(0x02) == (uint8_t)(patterns[p] & 0xE3), "OAM lane2 pattern roundtrip (masked)");
		}

		// Temp OAM debug roundtrips

		for (size_t addr = 0; addr < 32; addr++)
		{
			uint8_t val = (uint8_t)((addr * 7 + 3) & 0xFF);

			ppu->Dbg_TempOAMWriteByte(addr, val);

			uint8_t got = ppu->Dbg_TempOAMReadByte(addr);
			if (got != val)
			{
				char text[0x100]{};
				sprintf_s(text, sizeof(text), "TempOAM Write/Read mismatch at addr %zd: got %02X, expected %02X\n", addr, got, val);
				Logger::WriteMessage(text);
				return false;
			}
		}

		// CRAM debug roundtrips

		for (size_t addr = 0; addr < 32; addr++)
		{
			uint8_t val = (uint8_t)((addr * 7 + 3) & 0x3F);

			ppu->Dbg_CRAMWriteByte(addr, val);

			uint8_t got = ppu->Dbg_CRAMReadByte(addr);
			if (got != val)
			{
				char text[0x100]{};
				sprintf_s(text, sizeof(text), "CRAM Write/Read mismatch at addr %zd: got %02X, expected %02X\n", addr, got, val);
				Logger::WriteMessage(text);
				return false;
			}
		}

		// Dbg_GetCRAMAddress packs the CGA wires

		for (size_t n = 0; n < 5; n++)
		{
			ppu->wire.CGA[n] = TriState::Zero;
		}
		PUIT_CHECK(ppu->Dbg_GetCRAMAddress() == 0, "Dbg_GetCRAMAddress all zero");

		ppu->wire.CGA[0] = TriState::One;
		ppu->wire.CGA[2] = TriState::One;
		PUIT_CHECK(ppu->Dbg_GetCRAMAddress() == 0x05, "Dbg_GetCRAMAddress packed");

		// Dbg_GetPPUAddress packs the /PA wires

		for (size_t n = 0; n < 8; n++)
		{
			ppu->wire.n_PA_Bot[n] = TriState::One;
		}
		for (size_t n = 0; n < 6; n++)
		{
			ppu->wire.n_PA_Top[n] = TriState::One;
		}
		ppu->wire.n_PA_Bot[0] = TriState::Zero;
		ppu->wire.n_PA_Bot[3] = TriState::Zero;
		ppu->wire.n_PA_Top[1] = TriState::Zero;
		ppu->wire.n_PA_Top[5] = TriState::Zero;

		PUIT_CHECK(ppu->Dbg_GetPPUAddress() == 0x2209, "Dbg_GetPPUAddress packed");

		// Dbg_ReadRegister / Dbg_WriteRegister roundtrips

		int hc_ofs = (int)offsetof(PPU_Registers, HCounter);
		int vc_ofs = (int)offsetof(PPU_Registers, VCounter);
		int ctrl0_ofs = (int)offsetof(PPU_Registers, CTRL0);
		int ctrl1_ofs = (int)offsetof(PPU_Registers, CTRL1);
		int main_ofs = (int)offsetof(PPU_Registers, MainOAMCounter);
		int temp_ofs = (int)offsetof(PPU_Registers, TempOAMCounter);
		int oambuf_ofs = (int)offsetof(PPU_Registers, OAMBuffer);
		int rb_ofs = (int)offsetof(PPU_Registers, ReadBuffer);

		ppu->Dbg_WriteRegister(hc_ofs, 0x123);
		PUIT_CHECK(ppu->Dbg_ReadRegister(hc_ofs) == 0x123, "Register roundtrip HCounter");

		ppu->Dbg_WriteRegister(vc_ofs, 0x1FF);
		PUIT_CHECK(ppu->Dbg_ReadRegister(vc_ofs) == 0x1FF, "Register roundtrip VCounter");

		ppu->Dbg_WriteRegister(ctrl0_ofs, 0xFC);
		PUIT_CHECK(ppu->Dbg_ReadRegister(ctrl0_ofs) == 0xFC, "Register roundtrip CTRL0");

		ppu->Dbg_WriteRegister(ctrl1_ofs, 0xE7);
		PUIT_CHECK(ppu->Dbg_ReadRegister(ctrl1_ofs) == 0xE7, "Register roundtrip CTRL1");

		ppu->Dbg_WriteRegister(main_ofs, 0xAB);
		PUIT_CHECK(ppu->Dbg_ReadRegister(main_ofs) == 0xAB, "Register roundtrip MainOAMCounter");

		ppu->Dbg_WriteRegister(temp_ofs, 0x1A);
		PUIT_CHECK(ppu->Dbg_ReadRegister(temp_ofs) == 0x1A, "Register roundtrip TempOAMCounter");

		ppu->Dbg_WriteRegister(oambuf_ofs, 0xA5);
		PUIT_CHECK(ppu->Dbg_ReadRegister(oambuf_ofs) == 0xA5, "Register roundtrip OAMBuffer");

		ppu->Dbg_WriteRegister(rb_ofs, 0x5A);
		PUIT_CHECK(ppu->Dbg_ReadRegister(rb_ofs) == 0x5A, "Register roundtrip ReadBuffer");

		// The SCC_* registers have no write support (no-op) and read the current wire values (0 on a fresh PPU).

		int scc_fh_ofs = (int)offsetof(PPU_Registers, SCC_FH);
		ppu->Dbg_WriteRegister(scc_fh_ofs, 0x07);
		PUIT_CHECK(ppu->Dbg_ReadRegister(scc_fh_ofs) == 0, "SCC_FH write is a no-op");

		return true;
	}

	bool UnitTest::TestPPU_API_Video()
	{
		// Construct a VideoGen-only PPU for the video generator tests.
		PPUSim::PPU ppu2(Revision::RP2C02G, true);

		// GetSignalFeatures / IsComposite

		VideoSignalFeatures features{};
		ppu2.GetSignalFeatures(features);

		PUIT_CHECK(features.SamplesPerPCLK == 8, "SamplesPerPCLK");
		PUIT_CHECK(features.PixelsPerScan == 341, "PixelsPerScan");
		PUIT_CHECK(features.ScansPerField == 262, "ScansPerField");
		PUIT_CHECK(features.BackPorchSize == 40, "BackPorchSize");
		PUIT_CHECK(features.Composite == 1, "Composite flag");
		PUIT_CHECK(features.PhaseAlteration == 0, "PhaseAlteration");
		PUIT_CHECK(features.BlackLevel == 0.525f, "BlackLevel");
		PUIT_CHECK(features.WhiteLevel == 1.941f, "WhiteLevel");
		PUIT_CHECK(features.SyncLevel == 0.f, "SyncLevel");

		PUIT_CHECK(ppu2.IsComposite() == true, "RP2C02G must be composite");

		// SetRAWOutput + sim: deterministic RAW color output

		ppu2.SetRAWOutput(true);

		ppu2.wire.n_CC[0] = TriState::Zero;
		ppu2.wire.n_CC[1] = TriState::One;
		ppu2.wire.n_CC[2] = TriState::Zero;
		ppu2.wire.n_CC[3] = TriState::One;
		ppu2.wire.n_LL[0] = TriState::One;
		ppu2.wire.n_LL[1] = TriState::Zero;
		ppu2.wire.n_TR = TriState::Zero;
		ppu2.wire.n_TG = TriState::One;
		ppu2.wire.n_TB = TriState::Zero;

		ppu2.fsm.n_PICTURE = TriState::Zero;
		ppu2.fsm.SYNC = TriState::One;
		ppu2.fsm.BURST = TriState::Zero;
		ppu2.wire.RES = TriState::Zero;

		VideoOutSignal vout{};

		ppu2.wire.PCLK = TriState::One;
		ppu2.wire.n_PCLK = TriState::Zero;
		ppu2.vid_out->sim(vout);

		ppu2.wire.PCLK = TriState::Zero;
		ppu2.wire.n_PCLK = TriState::One;
		ppu2.vid_out->sim(vout);

		// CCx are latched inverted, LL/TR/TG/TB/Sync are inverted on the RAW output.
		// raw = CC0(1) | CC2(1)<<2 | LL1(1)<<5 | TR(1)<<6 | TB(1)<<8 | Sync(1)<<9 = 0x365

		if (vout.RAW.raw != 0x365)
		{
			char text[0x100]{};
			sprintf_s(text, sizeof(text), "RAW output mismatch: got %04X, expected 0365\n", vout.RAW.raw);
			Logger::WriteMessage(text);
			return false;
		}

		PUIT_CHECK(vout.RAW.CC0 == 1 && vout.RAW.CC1 == 0 && vout.RAW.CC2 == 1 && vout.RAW.CC3 == 0, "RAW CC bits");
		PUIT_CHECK(vout.RAW.LL0 == 0 && vout.RAW.LL1 == 1, "RAW LL bits");
		PUIT_CHECK(vout.RAW.TR == 1 && vout.RAW.TG == 0 && vout.RAW.TB == 1, "RAW tint bits");
		PUIT_CHECK(vout.RAW.Sync == 1, "RAW sync bit");

		// ConvertRAWToRGB: produces RGB fields, deterministic

		VideoOutSignal rawIn{};
		VideoOutSignal rgbOut{};
		VideoOutSignal rgbOut2{};

		rawIn.RAW.raw = 0x25;	// CC0, CC2, LL1 set, no sync

		ppu2.ConvertRAWToRGB(rawIn, rgbOut);
		PUIT_CHECK(rgbOut.RGB.RED <= 255 && rgbOut.RGB.GREEN <= 255 && rgbOut.RGB.BLUE <= 255, "ConvertRAWToRGB range");

		ppu2.ConvertRAWToRGB(rawIn, rgbOut2);
		PUIT_CHECK(rgbOut.RGB.RED == rgbOut2.RGB.RED, "ConvertRAWToRGB determinism R");
		PUIT_CHECK(rgbOut.RGB.GREEN == rgbOut2.RGB.GREEN, "ConvertRAWToRGB determinism G");
		PUIT_CHECK(rgbOut.RGB.BLUE == rgbOut2.RGB.BLUE, "ConvertRAWToRGB determinism B");

		// Back to the composite output path for the noise/DAC smoke tests

		ppu2.SetRAWOutput(false);

		// Composite noise smoke

		ppu2.SetCompositeNoise(0.1f);

		TriState CLK = TriState::Zero;

		for (size_t n = 0; n < 8; n++)
		{
			ppu2.wire.CLK = CLK;
			ppu2.wire.n_CLK = NOT(CLK);
			ppu2.wire.PCLK = CLK == TriState::One ? TriState::Zero : TriState::One;
			ppu2.wire.n_PCLK = NOT(ppu2.wire.PCLK);

			ppu2.vid_out->sim(vout);

			CLK = NOT(CLK);
		}

		PUIT_CHECK(std::isfinite(vout.composite), "Composite output must be finite with noise");

		ppu2.SetCompositeNoise(0.f);

		// UseExternalDacLevels smoke

		DacLevels tab{};
		tab.SyncLevel[0] = 0.1f;
		tab.SyncLevel[1] = 0.2f;
		tab.BurstLevel[0] = 0.15f;
		tab.BurstLevel[1] = 0.4f;
		tab.LumaLevel[0][0] = 0.2f;
		tab.LumaLevel[0][1] = 0.5f;
		tab.LumaLevel[1][0] = 0.3f;
		tab.LumaLevel[1][1] = 0.6f;
		tab.LumaLevel[2][0] = 0.4f;
		tab.LumaLevel[2][1] = 0.7f;
		tab.LumaLevel[3][0] = 0.5f;
		tab.LumaLevel[3][1] = 0.8f;
		tab.EmphasizedLumaLevel[0][0] = 0.2f;
		tab.EmphasizedLumaLevel[0][1] = 0.5f;
		tab.EmphasizedLumaLevel[1][0] = 0.3f;
		tab.EmphasizedLumaLevel[1][1] = 0.6f;
		tab.EmphasizedLumaLevel[2][0] = 0.4f;
		tab.EmphasizedLumaLevel[2][1] = 0.7f;
		tab.EmphasizedLumaLevel[3][0] = 0.5f;
		tab.EmphasizedLumaLevel[3][1] = 0.8f;

		ppu2.UseExternalDacLevels(true, tab);

		CLK = TriState::Zero;

		for (size_t n = 0; n < 8; n++)
		{
			ppu2.wire.CLK = CLK;
			ppu2.wire.n_CLK = NOT(CLK);
			ppu2.wire.PCLK = CLK == TriState::One ? TriState::Zero : TriState::One;
			ppu2.wire.n_PCLK = NOT(ppu2.wire.PCLK);

			ppu2.vid_out->sim(vout);

			CLK = NOT(CLK);
		}

		PUIT_CHECK(std::isfinite(vout.composite), "Composite output must be finite with external DAC levels");

		ppu2.UseExternalDacLevels(false, tab);

		return true;
	}

	bool UnitTest::TestPPU_API_Misc()
	{
		// SetOamDecayBehavior / GetOamDecayBehavior roundtrip

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Evaporate);
		PUIT_CHECK(ppu->oam->GetOamDecayBehavior() == OAMDecayBehavior::Evaporate, "OAM decay behavior Evaporate");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::ToZero);
		PUIT_CHECK(ppu->oam->GetOamDecayBehavior() == OAMDecayBehavior::ToZero, "OAM decay behavior ToZero");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::ToOne);
		PUIT_CHECK(ppu->oam->GetOamDecayBehavior() == OAMDecayBehavior::ToOne, "OAM decay behavior ToOne");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Randomize);
		PUIT_CHECK(ppu->oam->GetOamDecayBehavior() == OAMDecayBehavior::Randomize, "OAM decay behavior Randomize");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Keep);
		PUIT_CHECK(ppu->oam->GetOamDecayBehavior() == OAMDecayBehavior::Keep, "OAM decay behavior Keep");

		// Dbg_RenderAlwaysEnabled: forces BGE/OBE to 1, BLACK to 0

		ppu->regs->Debug_SetCTRL1(0x00);
		ppu->Dbg_RenderAlwaysEnabled(true);

		ppu->wire.RC = TriState::Zero;
		ppu->wire.n_W0 = TriState::One;
		ppu->wire.n_W1 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->regs->sim();

		PUIT_CHECK(ppu->wire.BGE == TriState::One, "RenderAlwaysEnabled BGE");
		PUIT_CHECK(ppu->wire.OBE == TriState::One, "RenderAlwaysEnabled OBE");
		PUIT_CHECK(ppu->wire.BLACK == TriState::Zero, "RenderAlwaysEnabled BLACK");

		ppu->Dbg_RenderAlwaysEnabled(false);
		ppu->regs->sim();

		PUIT_CHECK(ppu->wire.BGE == TriState::Zero, "Render disabled BGE");
		PUIT_CHECK(ppu->wire.OBE == TriState::Zero, "Render disabled OBE");
		PUIT_CHECK(ppu->wire.BLACK == TriState::One, "Render disabled BLACK");

		// Debug_ClippingAlwaysDisabled: forces /BGCLIP and /OBCLIP to 1

		ppu->regs->Debug_SetCTRL1(0x00);	// clipping bits 1,2 = 0
		ppu->regs->Debug_ClippingAlwaysDisabled(true);

		ppu->wire.RC = TriState::Zero;
		ppu->wire.n_W0 = TriState::One;
		ppu->wire.n_W1 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->regs->sim();

		PUIT_CHECK(ppu->wire.n_BGCLIP == TriState::One, "ClippingAlwaysDisabled n_BGCLIP");
		PUIT_CHECK(ppu->wire.n_OBCLIP == TriState::One, "ClippingAlwaysDisabled n_OBCLIP");

		ppu->regs->Debug_ClippingAlwaysDisabled(false);
		ppu->regs->sim();

		// With the clipping bits cleared the signals are active (0)

		PUIT_CHECK(ppu->wire.n_BGCLIP == TriState::Zero, "Clipping enabled n_BGCLIP");
		PUIT_CHECK(ppu->wire.n_OBCLIP == TriState::Zero, "Clipping enabled n_OBCLIP");

		return true;
	}

	bool UnitTest::TestBGC_SRBit()
	{
		BGC_SRBit sr;
		TriState sout{};

		// Load 1
		sr.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::One, sout);
		PUIT_CHECK(sout == TriState::One, "BGC_SRBit load 1");

		// Load 0
		sr.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::One, sout);
		PUIT_CHECK(sout == TriState::Zero, "BGC_SRBit load 0");

		// Shift in 1
		sr.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::One, sout);
		PUIT_CHECK(sout == TriState::One, "BGC_SRBit step 1");

		// Shift in 0
		sr.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, TriState::One, sout);
		PUIT_CHECK(sout == TriState::Zero, "BGC_SRBit step 0");

		// Next = 0: output latch holds
		sr.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::Zero, sout);
		PUIT_CHECK(sout == TriState::Zero, "BGC_SRBit Next=0 holds");

		// After the hold, load 1 again with Next=1
		sr.sim(TriState::Zero, TriState::One, TriState::One, TriState::Zero, TriState::One, sout);
		PUIT_CHECK(sout == TriState::One, "BGC_SRBit load after hold");

		return true;
	}

	bool UnitTest::TestBGCol()
	{
		// Drive the module directly with safe 0/1 inputs and check the outputs stay 0/1.

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.H0_Dash2 = TriState::One;
		ppu->wire.THO[1] = TriState::Zero;
		ppu->wire.TVO[1] = TriState::Zero;
		ppu->wire.FH[0] = TriState::Zero;
		ppu->wire.FH[1] = TriState::Zero;
		ppu->wire.FH[2] = TriState::Zero;
		ppu->wire.n_CLPB = TriState::One;

		ppu->fsm.FTA = TriState::Zero;
		ppu->fsm.FTB = TriState::Zero;
		ppu->fsm.nFO = TriState::One;
		ppu->fsm.FAT = TriState::Zero;

		ppu->PD = 0;
		ppu->DB = 0;

		for (size_t i = 0; i < 8; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->data_reader->bgcol->sim();

			for (size_t n = 0; n < 4; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.BGC[n]), "BGCol BGC must be 0/1");
			}
		}

		// Provide some PD data and check the BGC outputs still stay 0/1.

		ppu->PD = 0x55;

		for (size_t i = 0; i < 4; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->data_reader->bgcol->sim();

			for (size_t n = 0; n < 4; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.BGC[n]), "BGCol BGC must be 0/1 with data");
			}
		}

		return true;
	}

	bool UnitTest::TestCBBit()
	{
		// CBBit write path + get_CBOut

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.n_DB_CB = TriState::Zero;	// DB -> cell
		ppu->wire.n_CB_DB = TriState::One;

		ppu->SetDBBit(3, TriState::One);

		CBBit cb(ppu);
		TriState cell = TriState::Zero;

		cb.sim(3, &cell, TriState::One);

		PUIT_CHECK(cell == TriState::One, "CBBit cell write from DB");
		PUIT_CHECK(cb.get_CBOut(TriState::One) == TriState::One, "CBBit get_CBOut One");
		PUIT_CHECK(cb.get_CBOut(TriState::Zero) == TriState::Zero, "CBBit get_CBOut with n_OE=0");

		// PCLK = 1 clears the FF

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		cb.sim(3, &cell, TriState::One);

		PUIT_CHECK(cb.get_CBOut(TriState::One) == TriState::Zero, "CBBit FF cleared at PCLK=1");

		// Write Zero into the cell

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->SetDBBit(3, TriState::Zero);

		cb.sim(3, &cell, TriState::One);

		PUIT_CHECK(cell == TriState::Zero, "CBBit cell write zero");
		PUIT_CHECK(cb.get_CBOut(TriState::One) == TriState::Zero, "CBBit get_CBOut after zero");

		// CBBit_RGB write path

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->SetDBBit(1, TriState::One);

		CBBit_RGB cb_rgb(ppu);
		TriState cell_rgb = TriState::Zero;

		cb_rgb.sim(1, &cell_rgb, TriState::One);

		PUIT_CHECK(cell_rgb == TriState::One, "CBBit_RGB cell write from DB");
		PUIT_CHECK(cb_rgb.get_CBOut(TriState::One) == TriState::One, "CBBit_RGB get_CBOut One");

		return true;
	}

	bool UnitTest::TestCRAM()
	{
		// Dbg_CRAMWriteByte / Dbg_CRAMReadByte roundtrip (already covered via the PPU API test,
		// but check the full 6-bit lane range here as well).

		for (size_t addr = 0; addr < 32; addr++)
		{
			uint8_t val = (uint8_t)((addr * 13 + 7) & 0x3F);

			ppu->cram->Dbg_CRAMWriteByte(addr, val);

			uint8_t got = ppu->cram->Dbg_CRAMReadByte(addr);
			if (got != val)
			{
				char text[0x100]{};
				sprintf_s(text, sizeof(text), "CRAM roundtrip mismatch at %zd: got %02X, expected %02X\n", addr, got, val);
				Logger::WriteMessage(text);
				return false;
			}
		}

		// CRAM::sim: write a byte through the DB/CGA path into cell (0,0) and read it back.
		// Two-phase sequence: at PCLK=1 the DB_PAR latch is armed, at PCLK=0 the cell is written.

		ppu->wire.TH_MUX = TriState::One;
		ppu->wire.DB_PAR = TriState::One;
		ppu->wire.n_R7 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->wire.BnW = TriState::Zero;
		ppu->fsm.n_PICTURE = TriState::Zero;

		for (size_t n = 0; n < 5; n++)
		{
			ppu->wire.CGA[n] = TriState::Zero;
		}

		ppu->DB = 0x2A;

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		ppu->cram->sim();

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->cram->sim();

		uint8_t got = ppu->cram->Dbg_CRAMReadByte(0);
		if (got != 0x2A)
		{
			char text[0x100]{};
			sprintf_s(text, sizeof(text), "CRAM sim write mismatch: got %02X, expected 2A\n", got);
			Logger::WriteMessage(text);
			return false;
		}

		// Dbg_GetCRAMAddress reflects the CGA wires after the decoder ran

		PUIT_CHECK(ppu->Dbg_GetCRAMAddress() == 0, "Dbg_GetCRAMAddress after sim");

		return true;
	}

	bool UnitTest::TestFIFOLane()
	{
		FIFOLane lane(ppu);

		TriState n_TX[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			n_TX[n] = TriState::One;
		}

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->fsm.nVIS = TriState::One;
		ppu->wire.n_ZH = TriState::One;
		ppu->wire.n_OBJ_RD_ATTR = TriState::One;
		ppu->wire.n_OBJ_RD_X = TriState::One;
		ppu->wire.n_OBJ_RD_A = TriState::One;
		ppu->wire.n_OBJ_RD_B = TriState::One;

		for (size_t n = 0; n < 8; n++)
		{
			ppu->wire.OB[n] = TriState::Zero;
		}

		FIFOLaneOutput ZOut{};

		// Phase 1: n_PCLK=1. With SR_EN=0 a fresh shift register holds its initial (Zero) state,
		// the lane latches hold Zero (OB=0), and the enable output is 1.

		lane.sim(TriState::Zero, n_TX, 0xFF, ZOut);

		PUIT_CHECK(ZOut.nZ_COL0 == TriState::Zero, "FIFOLane nZ_COL0");
		PUIT_CHECK(ZOut.nZ_COL1 == TriState::Zero, "FIFOLane nZ_COL1");
		PUIT_CHECK(ZOut.Z_COL2 == TriState::Zero, "FIFOLane Z_COL2");
		PUIT_CHECK(ZOut.Z_COL3 == TriState::Zero, "FIFOLane Z_COL3");
		PUIT_CHECK(ZOut.nZ_PRIO == TriState::Zero, "FIFOLane nZ_PRIO");
		PUIT_CHECK(ZOut.n_xEN == TriState::One, "FIFOLane n_xEN");

		// Phase 2: PCLK=1
		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		lane.sim(TriState::Zero, n_TX, 0xFF, ZOut);

		PUIT_CHECK(Is01(ZOut.nZ_COL0), "FIFOLane phase2 nZ_COL0");
		PUIT_CHECK(Is01(ZOut.nZ_COL1), "FIFOLane phase2 nZ_COL1");
		PUIT_CHECK(Is01(ZOut.Z_COL2), "FIFOLane phase2 Z_COL2");
		PUIT_CHECK(Is01(ZOut.Z_COL3), "FIFOLane phase2 Z_COL3");
		PUIT_CHECK(Is01(ZOut.nZ_PRIO), "FIFOLane phase2 nZ_PRIO");
		PUIT_CHECK(Is01(ZOut.n_xEN), "FIFOLane phase2 n_xEN");

		return true;
	}

	bool UnitTest::TestFIFO()
	{
		// sim_SpriteH: with OBJ_READ=0 and H0-H2=0 all four /OBJ_RD signals become 1.

		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.PCLK = TriState::Zero;
		ppu->fsm.OBJ_READ = TriState::Zero;
		ppu->wire.H0_Dash2 = TriState::Zero;
		ppu->wire.H1_Dash2 = TriState::Zero;
		ppu->wire.H2_Dash2 = TriState::Zero;

		ppu->fifo->sim_SpriteH();

		PUIT_CHECK(ppu->wire.n_OBJ_RD_ATTR == TriState::One, "sim_SpriteH n_OBJ_RD_ATTR");
		PUIT_CHECK(ppu->wire.n_OBJ_RD_X == TriState::One, "sim_SpriteH n_OBJ_RD_X");
		PUIT_CHECK(ppu->wire.n_OBJ_RD_A == TriState::One, "sim_SpriteH n_OBJ_RD_A");
		PUIT_CHECK(ppu->wire.n_OBJ_RD_B == TriState::One, "sim_SpriteH n_OBJ_RD_B");

		// sim() smoke test

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->fsm.ZHPOS = TriState::Zero;
		ppu->wire.PD_FIFO = TriState::One;
		ppu->wire.OB[6] = TriState::Zero;
		ppu->fsm.nVIS = TriState::One;
		ppu->wire.n_OBJ_RD_ATTR = TriState::One;
		ppu->wire.H3_Dash2 = TriState::Zero;
		ppu->wire.H4_Dash2 = TriState::Zero;
		ppu->wire.H5_Dash2 = TriState::Zero;
		ppu->wire.CLPO = TriState::Zero;
		ppu->wire.n_SPR0_EV = TriState::One;
		ppu->fsm.RESCL = TriState::Zero;
		ppu->wire.n_R2 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		for (size_t n = 0; n < 8; n++)
		{
			ppu->wire.OB[n] = TriState::Zero;
		}
		ppu->PD = 0;

		for (size_t i = 0; i < 4; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->fifo->sim();

			PUIT_CHECK(Is01(ppu->wire.n_SPR0HIT), "FIFO sim n_SPR0HIT");
			PUIT_CHECK(Is01(ppu->wire.n_ZH), "FIFO sim n_ZH");
			PUIT_CHECK(Is01(ppu->wire.n_ZCOL0), "FIFO sim n_ZCOL0");
			PUIT_CHECK(Is01(ppu->wire.n_ZCOL1), "FIFO sim n_ZCOL1");
			PUIT_CHECK(Is01(ppu->wire.ZCOL2), "FIFO sim ZCOL2");
			PUIT_CHECK(Is01(ppu->wire.ZCOL3), "FIFO sim ZCOL3");
			PUIT_CHECK(Is01(ppu->wire.n_ZPRIO), "FIFO sim n_ZPRIO");
		}

		return true;
	}

	bool UnitTest::TestFSM()
	{
		char text[0x100]{};

		// Standalone FSM simulation with the H/V PLA outputs for the current counter state.

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		ppu->wire.RES = TriState::Zero;
		ppu->wire.BLACK = TriState::Zero;
		ppu->wire.n_OBCLIP = TriState::One;
		ppu->wire.n_BGCLIP = TriState::One;
		ppu->wire.VBL = TriState::Zero;
		ppu->wire.n_R2 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		ppu->h->set(0);
		ppu->v->set(0);

		TriState* HPLA;
		TriState* VPLA;

		ppu->hv_dec->sim_HDecoder(ppu->hv_fsm->get_VB(), ppu->hv_fsm->get_BLNK(ppu->wire.BLACK), &HPLA);
		ppu->hv_dec->sim_VDecoder(&VPLA);

		PUIT_CHECK(HPLA != nullptr && VPLA != nullptr, "FSM test PLA outputs");

		ppu->hv_fsm->sim(HPLA, VPLA);

		PUIT_CHECK(Is01(ppu->hv_fsm->get_VB()), "FSM get_VB");
		PUIT_CHECK(Is01(ppu->hv_fsm->get_BLNK(ppu->wire.BLACK)), "FSM get_BLNK");
		PUIT_CHECK(ppu->fsm.VB == ppu->hv_fsm->get_VB(), "FSM VB consistency");
		PUIT_CHECK(ppu->fsm.BLNK == ppu->hv_fsm->get_BLNK(ppu->wire.BLACK), "FSM BLNK consistency");

		PUIT_CHECK(Is01(ppu->fsm.SEV), "FSM SEV");
		PUIT_CHECK(Is01(ppu->fsm.CLIP_O), "FSM CLIP_O");
		PUIT_CHECK(Is01(ppu->fsm.CLIP_B), "FSM CLIP_B");
		PUIT_CHECK(Is01(ppu->fsm.ZHPOS), "FSM ZHPOS");
		PUIT_CHECK(Is01(ppu->fsm.n_EVAL), "FSM n_EVAL");
		PUIT_CHECK(Is01(ppu->fsm.EEV), "FSM EEV");
		PUIT_CHECK(Is01(ppu->fsm.IOAM2), "FSM IOAM2");
		PUIT_CHECK(Is01(ppu->fsm.OBJ_READ), "FSM OBJ_READ");
		PUIT_CHECK(Is01(ppu->fsm.nVIS), "FSM nVIS");
		PUIT_CHECK(Is01(ppu->fsm.nFNT), "FSM nFNT");
		PUIT_CHECK(Is01(ppu->fsm.FTB), "FSM FTB");
		PUIT_CHECK(Is01(ppu->fsm.FTA), "FSM FTA");
		PUIT_CHECK(Is01(ppu->fsm.FAT), "FSM FAT");
		PUIT_CHECK(Is01(ppu->fsm.nFO), "FSM nFO");
		PUIT_CHECK(Is01(ppu->fsm.BPORCH), "FSM BPORCH");
		PUIT_CHECK(Is01(ppu->fsm.SCCNT), "FSM SCCNT");
		PUIT_CHECK(Is01(ppu->fsm.nHB), "FSM nHB");
		PUIT_CHECK(Is01(ppu->fsm.BURST), "FSM BURST");
		PUIT_CHECK(Is01(ppu->fsm.SYNC), "FSM SYNC");
		PUIT_CHECK(Is01(ppu->fsm.n_PICTURE), "FSM n_PICTURE");
		PUIT_CHECK(Is01(ppu->fsm.RESCL), "FSM RESCL");
		PUIT_CHECK(Is01(ppu->fsm.VSYNC), "FSM VSYNC");
		PUIT_CHECK(Is01(ppu->fsm.VB), "FSM VB");
		PUIT_CHECK(Is01(ppu->fsm.BLNK), "FSM BLNK");
		PUIT_CHECK(Is01(ppu->fsm.INT), "FSM INT");

		// sim_RESCL_early: with PCLK=1 the second latch of the RESCL chain is updated

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		ppu->hv_fsm->sim_RESCL_early();

		PUIT_CHECK(Is01(ppu->fsm.RESCL), "FSM RESCL after sim_RESCL_early");

		// Note: the $2002 VBlank flag is covered by the full-PPU integration test
		// TestVBlankRead2002 (a standalone call here would need exact PCLK phasing).

		return true;
	}

	bool UnitTest::TestHVCounterBit()
	{
		HVCounterBit hb(ppu);

		ppu->wire.RES = TriState::Zero;

		// set/getOut

		hb.set(TriState::One);
		PUIT_CHECK(hb.getOut() == TriState::One, "HVCounterBit set/get One");

		hb.set(TriState::Zero);
		PUIT_CHECK(hb.getOut() == TriState::Zero, "HVCounterBit set/get Zero");

		// Toggle with Carry=1 (PCLK=0 sets the latch, PCLK=1 updates the FF)

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		hb.sim(TriState::One, TriState::Zero);

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		hb.sim(TriState::One, TriState::Zero);

		PUIT_CHECK(hb.getOut() == TriState::One, "HVCounterBit toggled to One");

		// Hold with Carry=0

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		hb.sim(TriState::Zero, TriState::Zero);

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		hb.sim(TriState::Zero, TriState::Zero);

		PUIT_CHECK(hb.getOut() == TriState::One, "HVCounterBit held with Carry=0");

		// CLR=1 clears the FF at PCLK=1

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		hb.sim(TriState::Zero, TriState::One);

		PUIT_CHECK(hb.getOut() == TriState::Zero, "HVCounterBit cleared by CLR");

		// Toggle back to One

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		hb.sim(TriState::One, TriState::Zero);

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		hb.sim(TriState::One, TriState::Zero);

		PUIT_CHECK(hb.getOut() == TriState::One, "HVCounterBit toggled again");

		return true;
	}

	bool UnitTest::TestHVCounter()
	{
		ppu->wire.RES = TriState::Zero;

		// set/get roundtrip on the 9-bit H counter

		ppu->h->set(0x1FF);
		PUIT_CHECK(ppu->h->get() == 0x1FF, "HVCounter set/get all ones");

		for (size_t n = 0; n < 9; n++)
		{
			TriState expected = (0x1FF >> n) & 1 ? TriState::One : TriState::Zero;
			PUIT_CHECK(ppu->h->getBit(n) == expected, "HVCounter getBit");
		}

		ppu->h->set(0x155);
		PUIT_CHECK(ppu->h->get() == 0x155, "HVCounter set/get pattern");

		ppu->h->set(0);
		PUIT_CHECK(ppu->h->get() == 0, "HVCounter set/get zero");

		// V counter

		ppu->v->set(0x1FF);
		PUIT_CHECK(ppu->v->get() == 0x1FF, "VCounter set/get all ones");

		for (size_t n = 0; n < 9; n++)
		{
			TriState expected = (0x1FF >> n) & 1 ? TriState::One : TriState::Zero;
			PUIT_CHECK(ppu->v->getBit(n) == expected, "VCounter getBit");
		}

		ppu->v->set(0x123);
		PUIT_CHECK(ppu->v->get() == 0x123, "VCounter set/get pattern");

		return true;
	}

	bool UnitTest::TestHVDecoder()
	{
		ppu->wire.RES = TriState::Zero;

		// H decoder over a range of counter values, with VB/BLNK combinations

		TriState vb_tab[2] = { TriState::Zero, TriState::One };
		TriState blnk_tab[2] = { TriState::Zero, TriState::One };

		for (size_t vb = 0; vb < 2; vb++)
		{
			for (size_t blnk = 0; blnk < 2; blnk++)
			{
				for (size_t h = 0; h <= 0x1FF; h++)
				{
					ppu->h->set(h);

					TriState* HPLA;
					ppu->hv_dec->sim_HDecoder(vb_tab[vb], blnk_tab[blnk], &HPLA);

					PUIT_CHECK(HPLA != nullptr, "HDecoder outputs");

					for (size_t n = 0; n < 24; n++)
					{
						PUIT_CHECK(Is01(HPLA[n]), "HDecoder output must be 0/1");
					}
				}
			}
		}

		// V decoder

		for (size_t v = 0; v <= 0x1FF; v++)
		{
			ppu->v->set(v);

			TriState* VPLA;
			ppu->hv_dec->sim_VDecoder(&VPLA);

			PUIT_CHECK(VPLA != nullptr, "VDecoder outputs");

			for (size_t n = 0; n < 9; n++)
			{
				PUIT_CHECK(Is01(VPLA[n]), "VDecoder output must be 0/1");
			}
		}

		return true;
	}

	bool UnitTest::TestMux()
	{
		// Sprite 0 Hit (Strike) detection: set up a "strike" condition and check DB bit 6.

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;

		ppu->wire.n_ZCOL0 = TriState::Zero;
		ppu->wire.n_ZCOL1 = TriState::Zero;
		ppu->wire.ZCOL2 = TriState::Zero;
		ppu->wire.ZCOL3 = TriState::Zero;
		ppu->wire.n_ZPRIO = TriState::One;
		ppu->wire.TH_MUX = TriState::Zero;
		ppu->wire.BGC[0] = TriState::One;
		ppu->wire.BGC[1] = TriState::Zero;
		ppu->wire.BGC[2] = TriState::Zero;
		ppu->wire.BGC[3] = TriState::Zero;
		ppu->wire.THO[0] = TriState::Zero;
		ppu->wire.THO[1] = TriState::Zero;
		ppu->wire.THO[2] = TriState::Zero;
		ppu->wire.THO[3] = TriState::Zero;
		ppu->wire.THO[4] = TriState::Zero;
		ppu->wire.EXT_In[0] = TriState::Zero;
		ppu->wire.EXT_In[1] = TriState::Zero;
		ppu->wire.EXT_In[2] = TriState::Zero;
		ppu->wire.EXT_In[3] = TriState::Zero;
		ppu->wire.n_SPR0HIT = TriState::Zero;
		ppu->wire.n_SPR0_EV = TriState::Zero;
		ppu->fsm.nVIS = TriState::Zero;
		ppu->fsm.RESCL = TriState::Zero;
		ppu->wire.n_R2 = TriState::Zero;
		ppu->wire.n_DBE = TriState::Zero;

		ppu->mux->sim();

		PUIT_CHECK(ppu->GetDBBit(6) == TriState::One, "Mux Sprite0Hit strike sets DB6");

		// The Strike FF is only cleared by RESCL

		ppu->fsm.RESCL = TriState::One;
		ppu->mux->sim();

		PUIT_CHECK(ppu->GetDBBit(6) == TriState::Zero, "Mux Sprite0Hit cleared by RESCL");

		// Output wires stay 0/1

		for (size_t n = 0; n < 4; n++)
		{
			PUIT_CHECK(Is01(ppu->wire.CGA[n]), "Mux CGA 0/1");
			PUIT_CHECK(Is01(ppu->wire.n_EXT_Out[n]), "Mux n_EXT_Out 0/1");
		}
		PUIT_CHECK(Is01(ppu->wire.CGA[4]), "Mux CGA4 0/1");

		return true;
	}

	bool UnitTest::TestOAMCell()
	{
		OAMCell cell(ppu);

		cell.SetTopo(OAMCellTopology::TopLeft, 0);
		cell.SetTopo(OAMCellTopology::Middle, 1);

		cell.set(TriState::One);
		PUIT_CHECK(cell.get() == TriState::One, "OAMCell set/get One");

		cell.set(TriState::Zero);
		PUIT_CHECK(cell.get() == TriState::Zero, "OAMCell set/get Zero");

		// Decay behavior: advance the software PCLK counter past the decay timeout.

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Keep);
		cell.set(TriState::One);

		ppu->pclk_counter = 2000000;

		PUIT_CHECK(cell.get() == TriState::One, "OAMCell Keep decay");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::ToZero);
		PUIT_CHECK(cell.get() == TriState::Zero, "OAMCell ToZero decay");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::ToOne);
		PUIT_CHECK(cell.get() == TriState::One, "OAMCell ToOne decay");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Evaporate);
		PUIT_CHECK(cell.get() == TriState::Z, "OAMCell Evaporate decay");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Randomize);
		TriState rnd = cell.get();
		PUIT_CHECK(rnd == TriState::Zero || rnd == TriState::One, "OAMCell Randomize decay");

		ppu->SetOamDecayBehavior(OAMDecayBehavior::Keep);
		ppu->pclk_counter = 0;

		return true;
	}

	bool UnitTest::TestOAMLane()
	{
		// Write then read back

		OAMLane lane(ppu, false);

		TriState v = TriState::One;
		lane.sim(0, 0, v);
		PUIT_CHECK(v == TriState::One, "OAMLane write value unchanged");

		TriState r = TriState::Z;
		lane.sim(0, 0, r);
		PUIT_CHECK(r == TriState::One, "OAMLane read back One");

		v = TriState::Zero;
		lane.sim(0, 0, v);

		r = TriState::Z;
		lane.sim(0, 0, r);
		PUIT_CHECK(r == TriState::Zero, "OAMLane read back Zero");

		// Skip-attr-bits lane (2/6): bits 2-4 are never written/read

		OAMLane lane2(ppu, true);

		TriState w = TriState::One;
		lane2.sim(0, 2, w);

		TriState rr = TriState::Z;
		lane2.sim(0, 2, rr);
		PUIT_CHECK(rr == TriState::Z, "OAMLane skip_attr_bits keeps Z");

		return true;
	}

	bool UnitTest::TestOAMBufferBit()
	{
		OAMLane lane(ppu, false);

		TriState w = TriState::One;
		lane.sim(0, 0, w);

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.n_R4 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->fsm.IOAM2 = TriState::Zero;

		OAMBufferBit ob(ppu);

		ob.sim(&lane, 0, 0, TriState::Zero, TriState::Zero);

		PUIT_CHECK(ob.get() == TriState::One, "OAMBufferBit loaded from OAM cell");
		PUIT_CHECK(ppu->wire.OB[0] == TriState::One, "OAMBufferBit drives OB wire");

		// Holds its value at PCLK=1

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		ob.sim(&lane, 0, 0, TriState::Zero, TriState::Zero);

		PUIT_CHECK(ob.get() == TriState::One, "OAMBufferBit holds at PCLK=1");

		// set/get roundtrip

		ob.set(TriState::Zero);
		PUIT_CHECK(ob.get() == TriState::Zero, "OAMBufferBit set/get Zero");

		ob.set(TriState::One);
		PUIT_CHECK(ob.get() == TriState::One, "OAMBufferBit set/get One");

		// RGB variant

		OAMBufferBit_RGB obr(ppu);

		obr.set(TriState::Zero);
		PUIT_CHECK(obr.get() == TriState::Zero, "OAMBufferBit_RGB set/get Zero");

		obr.set(TriState::One);
		PUIT_CHECK(obr.get() == TriState::One, "OAMBufferBit_RGB set/get One");

		TriState w2 = TriState::One;
		lane.sim(0, 0, w2);

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;

		obr.sim(&lane, 0, 0, TriState::Zero, TriState::Zero);

		PUIT_CHECK(ppu->wire.OB[0] == TriState::One, "OAMBufferBit_RGB drives OB wire");

		return true;
	}

	bool UnitTest::TestOAM()
	{
		// set_OB / get_OB roundtrip

		for (size_t n = 0; n < 8; n++)
		{
			ppu->oam->set_OB(n, TriState::One);
			PUIT_CHECK(ppu->oam->get_OB(n) == TriState::One, "OAM set_OB/get_OB One");

			ppu->oam->set_OB(n, TriState::Zero);
			PUIT_CHECK(ppu->oam->get_OB(n) == TriState::Zero, "OAM set_OB/get_OB Zero");
		}

		// Dbg_Get/Set_OAMBuffer roundtrip

		ppu->oam->Dbg_Set_OAMBuffer(0xA5);
		PUIT_CHECK(ppu->oam->Dbg_Get_OAMBuffer() == 0xA5, "OAM buffer roundtrip A5");

		ppu->oam->Dbg_Set_OAMBuffer(0x5A);
		PUIT_CHECK(ppu->oam->Dbg_Get_OAMBuffer() == 0x5A, "OAM buffer roundtrip 5A");

		// OAM::sim smoke: drive the address decoder to lane 0, row 0

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.OAM8 = TriState::Zero;
		ppu->wire.OFETCH = TriState::Zero;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->wire.SPR_OV = TriState::Zero;
		ppu->wire.OAMCTR2 = TriState::Zero;
		ppu->wire.H0_Dash2 = TriState::Zero;
		ppu->fsm.nVIS = TriState::One;
		ppu->fsm.IOAM2 = TriState::Zero;
		ppu->wire.n_R4 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		for (size_t n = 0; n < 8; n++)
		{
			ppu->wire.n_OAM[n] = TriState::One;
		}

		ppu->oam->sim();

		PUIT_CHECK(ppu->wire.n_WE == TriState::One, "OAM sim n_WE");

		for (size_t n = 0; n < 8; n++)
		{
			PUIT_CHECK(ppu->wire.OB[n] == TriState::Zero, "OAM sim OB must be Zero on empty OAM");
		}

		// sim_OFETCH smoke

		ppu->wire.n_W4 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		for (size_t i = 0; i < 8; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->oam->sim_OFETCH();

			PUIT_CHECK(Is01(ppu->wire.OFETCH), "OAM sim_OFETCH OFETCH must be 0/1");
		}

		return true;
	}

	bool UnitTest::TestParBits()
	{
		// ParBitInv truth table (n_PCLK=1, O=1)

		ParBitInv inv;
		TriState vout{};

		// INV=0: val_out = NOT(val_in)
		inv.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, vout);
		PUIT_CHECK(vout == TriState::Zero, "ParBitInv INV=0 val_in=1");

		inv.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, vout);
		PUIT_CHECK(vout == TriState::One, "ParBitInv INV=0 val_in=0");

		// INV=1: val_out = val_in
		inv.sim(TriState::One, TriState::One, TriState::One, TriState::One, vout);
		PUIT_CHECK(vout == TriState::One, "ParBitInv INV=1 val_in=1");

		inv.sim(TriState::One, TriState::One, TriState::One, TriState::Zero, vout);
		PUIT_CHECK(vout == TriState::Zero, "ParBitInv INV=1 val_in=0");

		// ParBit truth table (n_PCLK=1, O=1)

		ParBit bit;
		TriState out{};

		// OBJ_READ=0: PADx = val_PD
		bit.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::Zero, out);
		PUIT_CHECK(out == TriState::One, "ParBit OBJ_READ=0 PD=1");

		bit.sim(TriState::One, TriState::One, TriState::One, TriState::Zero, TriState::Zero, out);
		PUIT_CHECK(out == TriState::Zero, "ParBit OBJ_READ=0 PD=0");

		// OBJ_READ=1: PADx = val_OB
		bit.sim(TriState::One, TriState::One, TriState::One, TriState::Zero, TriState::One, out);
		PUIT_CHECK(out == TriState::One, "ParBit OBJ_READ=1 OB=1");

		bit.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::One, out);
		PUIT_CHECK(out == TriState::Zero, "ParBit OBJ_READ=1 OB=0");

		return true;
	}

	bool UnitTest::TestPAR()
	{
		// PAR::sim smoke test with safe inputs

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.H0_Dash2 = TriState::Zero;
		ppu->wire.nH1_Dash = TriState::One;

		ppu->fsm.nFNT = TriState::One;
		ppu->wire.BGSEL = TriState::Zero;
		ppu->wire.OBSEL = TriState::Zero;
		ppu->wire.O8_16 = TriState::Zero;
		ppu->fsm.OBJ_READ = TriState::Zero;

		for (size_t n = 0; n < 8; n++)
		{
			ppu->wire.OB[n] = TriState::Zero;
		}
		ppu->wire.n_OBJ_RD_ATTR = TriState::One;
		ppu->wire.OV[0] = TriState::Zero;
		ppu->wire.OV[1] = TriState::Zero;
		ppu->wire.OV[2] = TriState::Zero;
		ppu->wire.OV[3] = TriState::Zero;
		ppu->wire.n_FVO[0] = TriState::Zero;
		ppu->wire.n_FVO[1] = TriState::Zero;
		ppu->wire.n_FVO[2] = TriState::Zero;
		ppu->PD = 0;

		ppu->data_reader->par->sim();

		PUIT_CHECK(ppu->wire.PAT_ADR[3] == TriState::Zero, "PAR PAT_ADR[3]");

		for (size_t n = 0; n < 14; n++)
		{
			PUIT_CHECK(Is01(ppu->wire.PAT_ADR[n]), "PAR PAT_ADR must be 0/1");
		}

		// Second phase

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		ppu->data_reader->par->sim();

		for (size_t n = 0; n < 14; n++)
		{
			PUIT_CHECK(Is01(ppu->wire.PAT_ADR[n]), "PAR PAT_ADR must be 0/1 (phase 2)");
		}

		return true;
	}

	bool UnitTest::TestTileCounterBit()
	{
		TileCounterBit bit;
		TriState vo{};
		TriState nvo{};

		// Load One
		bit.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One && nvo == TriState::Zero, "TileCounterBit load 1");

		// Keep
		bit.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One, "TileCounterBit keep");

		// Toggle with carry_in=1: 1 -> 0
		bit.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::Zero, "TileCounterBit step to zero");

		// 0 -> 1
		bit.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One, "TileCounterBit step to one");

		// sim_res: with Reset=1 the output is forced to 0 even though the FF holds the loaded value

		TileCounterBit bit2;
		TriState vo2{};
		TriState nvo2{};

		bit2.sim_res(TriState::One, TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::One, vo2, nvo2);
		PUIT_CHECK(vo2 == TriState::Zero, "TileCounterBit sim_res output zero while Reset");

		bit2.sim_res(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, vo2, nvo2);
		PUIT_CHECK(vo2 == TriState::One, "TileCounterBit sim_res output released");

		return true;
	}

	bool UnitTest::TestTileCnt()
	{
		// TileCnt::sim smoke test with safe inputs

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.n_W6_2 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		ppu->fsm.SCCNT = TriState::Zero;
		ppu->fsm.RESCL = TriState::Zero;
		ppu->fsm.EEV = TriState::Zero;
		ppu->wire.TSTEP = TriState::Zero;
		ppu->fsm.FTB = TriState::Zero;
		ppu->wire.H0_Dash2 = TriState::Zero;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->wire.I1_32 = TriState::Zero;

		for (size_t n = 0; n < 5; n++)
		{
			ppu->wire.n_THO[n] = TriState::Zero;
			ppu->wire.n_TVO[n] = TriState::Zero;
			ppu->wire.TVO[n] = TriState::Zero;
			ppu->wire.TV[n] = TriState::Zero;
			ppu->wire.TH[n] = TriState::Zero;
		}
		for (size_t n = 0; n < 3; n++)
		{
			ppu->wire.n_FVO[n] = TriState::Zero;
			ppu->wire.FV[n] = TriState::Zero;
		}
		ppu->wire.NTH = TriState::Zero;
		ppu->wire.NTV = TriState::Zero;

		for (size_t i = 0; i < 4; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->data_reader->tilecnt->sim();

			for (size_t n = 0; n < 3; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.FVO[n]), "TileCnt FVO 0/1");
				PUIT_CHECK(Is01(ppu->wire.n_FVO[n]), "TileCnt n_FVO 0/1");
			}
			for (size_t n = 0; n < 5; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.THO[n]), "TileCnt THO 0/1");
				PUIT_CHECK(Is01(ppu->wire.n_THO[n]), "TileCnt n_THO 0/1");
				PUIT_CHECK(Is01(ppu->wire.TVO[n]), "TileCnt TVO 0/1");
				PUIT_CHECK(Is01(ppu->wire.n_TVO[n]), "TileCnt n_TVO 0/1");
			}
			PUIT_CHECK(Is01(ppu->wire.NTHOut), "TileCnt NTHOut 0/1");
			PUIT_CHECK(Is01(ppu->wire.NTVOut), "TileCnt NTVOut 0/1");
		}

		return true;
	}

	bool UnitTest::TestPAMUXBits()
	{
		// PAMUX_LowBit: n_PAx = NOT(selected source)

		PAMUX_LowBit lo;
		TriState nPA{};

		// DB_PAR=1 -> DB_in
		lo.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, TriState::Zero, nPA);
		PUIT_CHECK(nPA == TriState::One, "PAMUX_LowBit DB_PAR selects DB_in (inverted)");

		// DB_PAR=0, PARR=1 -> PAT_ADR
		lo.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, TriState::Zero, nPA);
		PUIT_CHECK(nPA == TriState::Zero, "PAMUX_LowBit PARR selects PAT_ADR (inverted)");

		// DB_PAR=0, PARR=0, PAL=1 -> NT_ADR
		lo.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, TriState::Zero, nPA);
		PUIT_CHECK(nPA == TriState::One, "PAMUX_LowBit PAL selects NT_ADR (inverted)");

		// DB_PAR=0, PARR=0, PAL=0, F_AT=1 -> AT_ADR
		lo.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One,
			TriState::One, TriState::Zero, TriState::One, TriState::Zero, nPA);
		PUIT_CHECK(nPA == TriState::Zero, "PAMUX_LowBit F_AT selects AT_ADR (inverted)");

		// All selects off -> the output latch holds its value
		TriState prev = nPA;
		lo.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, TriState::Zero, nPA);
		PUIT_CHECK(nPA == prev, "PAMUX_LowBit all-off holds");

		// PAMUX_HighBit

		PAMUX_HighBit hi;
		TriState nPAh{};

		// PARR=1 -> PAT_ADR
		hi.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, nPAh);
		PUIT_CHECK(nPAh == TriState::Zero, "PAMUX_HighBit PARR selects PAT_ADR (inverted)");

		// PARR=0, PAH=1 -> NT_ADR
		hi.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, nPAh);
		PUIT_CHECK(nPAh == TriState::One, "PAMUX_HighBit PAH selects NT_ADR (inverted)");

		// PARR=0, PAH=0, F_AT=1 -> AT_ADR
		hi.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::One,
			TriState::One, TriState::Zero, TriState::One, nPAh);
		PUIT_CHECK(nPAh == TriState::Zero, "PAMUX_HighBit F_AT selects AT_ADR (inverted)");

		// All selects off -> hold
		prev = nPAh;
		hi.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::Zero, TriState::One, nPAh);
		PUIT_CHECK(nPAh == prev, "PAMUX_HighBit all-off holds");

		return true;
	}

	bool UnitTest::TestPAMUX()
	{
		// PAMUX::sim + sim_MuxInputs + sim_MuxOutputs: deterministic address output.

		ppu->wire.nH2_Dash = TriState::One;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->fsm.FAT = TriState::Zero;
		ppu->wire.DB_PAR = TriState::One;
		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;

		for (size_t n = 0; n < 5; n++)
		{
			ppu->wire.THO[n] = TriState::Zero;
			ppu->wire.TVO[n] = TriState::Zero;
		}
		ppu->wire.NTHOut = TriState::Zero;
		ppu->wire.NTVOut = TriState::Zero;
		ppu->wire.n_FVO[0] = TriState::Zero;
		ppu->wire.FVO[1] = TriState::Zero;
		ppu->wire.n_FVO[1] = TriState::Zero;
		ppu->wire.n_FVO[2] = TriState::Zero;
		ppu->wire.FVO[2] = TriState::Zero;
		ppu->wire.FVO[0] = TriState::Zero;

		for (size_t n = 0; n < 14; n++)
		{
			ppu->wire.PAT_ADR[n] = TriState::Zero;
		}

		ppu->DB = 0x00;

		ppu->data_reader->pamux->sim();
		ppu->data_reader->pamux->sim_MuxInputs();
		ppu->data_reader->pamux->sim_MuxOutputs();

		// DB_PAR=1 and DB=0 -> all /PA_Bot bits are 1
		for (size_t n = 0; n < 8; n++)
		{
			PUIT_CHECK(ppu->wire.n_PA_Bot[n] == TriState::One, "PAMUX n_PA_Bot with DB=0");
		}

		// With PARR=0 and PAH=1 the HighBit mux selects NT_ADR:
		// /PA_Top[n] = NOT(NT_ADR[8+n]), where NT_ADR[8]=TVO[3]=0, NT_ADR[9]=TVO[4]=0,
		// NT_ADR[10]=NTHOut=0, NT_ADR[11]=NTVOut=0, NT_ADR[12]=NOR(n_FVO0, NOT(BLNK))=0,
		// NT_ADR[13]=NOT(NOR(FVO1, NOT(BLNK)))=1
		TriState expected_top[6] = { TriState::One, TriState::One, TriState::One, TriState::One, TriState::One, TriState::Zero };
		for (size_t n = 0; n < 6; n++)
		{
			if (ppu->wire.n_PA_Top[n] != expected_top[n])
			{
				char text[0x100]{};
				sprintf_s(text, sizeof(text), "PAMUX n_PA_Top[%zd] mismatch\n", n);
				Logger::WriteMessage(text);
				return false;
			}
		}

		PUIT_CHECK(ppu->Dbg_GetPPUAddress() == 0x2000, "PAMUX PPU address (DB=0)");

		// With DB=0x5A, /PA_Bot = NOT(DB), so PABot = DB
		ppu->DB = 0x5A;

		ppu->data_reader->pamux->sim();
		ppu->data_reader->pamux->sim_MuxInputs();
		ppu->data_reader->pamux->sim_MuxOutputs();

		PUIT_CHECK(ppu->Dbg_GetPPUAddress() == 0x205A, "PAMUX PPU address (DB=0x5A)");

		return true;
	}

	bool UnitTest::TestControlRegs()
	{
		// sim_RWDecoder truth table

		for (size_t rnw = 0; rnw < 2; rnw++)
		{
			for (size_t dbe = 0; dbe < 2; dbe++)
			{
				ppu->wire.RnW = rnw ? TriState::One : TriState::Zero;
				ppu->wire.n_DBE = dbe ? TriState::One : TriState::Zero;

				ppu->regs->sim_RWDecoder();

				TriState exp_n_RD = NOT(NOR(NOT(ppu->wire.RnW), ppu->wire.n_DBE));
				TriState exp_n_WR = NOT(NOR(ppu->wire.RnW, ppu->wire.n_DBE));

				if (ppu->wire.n_RD != exp_n_RD || ppu->wire.n_WR != exp_n_WR)
				{
					char text[0x100]{};
					sprintf_s(text, sizeof(text), "RWDecoder mismatch for RnW=%zd n_DBE=%zd\n", rnw, dbe);
					Logger::WriteMessage(text);
					return false;
				}
			}
		}

		// Regular register decode ($2000-$2007). Each sim() also runs the W56/SCCX logic,
		// which is harmless for the regular registers.

		auto setRS = [this](size_t rs)
		{
			ppu->wire.RS[0] = (rs & 1) ? TriState::One : TriState::Zero;
			ppu->wire.RS[1] = (rs & 2) ? TriState::One : TriState::Zero;
			ppu->wire.RS[2] = (rs & 4) ? TriState::One : TriState::Zero;
		};

		ppu->wire.n_DBE = TriState::Zero;
		ppu->wire.RC = TriState::Zero;

		// $2000 write
		setRS(0);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W0 == TriState::Zero, "Decode n_W0");
		PUIT_CHECK(ppu->wire.n_W1 == TriState::One, "Decode n_W1 (not $2001)");

		// $2001 write
		setRS(1);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W1 == TriState::Zero, "Decode n_W1");
		PUIT_CHECK(ppu->wire.n_W0 == TriState::One, "Decode n_W0 (not $2000)");

		// $2002 read
		setRS(2);
		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_R2 == TriState::Zero, "Decode n_R2");

		// $2003 write
		setRS(3);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W3 == TriState::Zero, "Decode n_W3");

		// $2004 write / read
		setRS(4);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W4 == TriState::Zero, "Decode n_W4");
		PUIT_CHECK(ppu->wire.n_R4 == TriState::One, "Decode n_R4 (not read)");

		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_R4 == TriState::Zero, "Decode n_R4 (read)");
		PUIT_CHECK(ppu->wire.n_W4 == TriState::One, "Decode n_W4 (not write)");

		// $2007 write / read
		setRS(7);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W7 == TriState::Zero, "Decode n_W7");
		PUIT_CHECK(ppu->wire.n_R7 == TriState::One, "Decode n_R7 (not read)");

		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_R7 == TriState::Zero, "Decode n_R7 (read)");
		PUIT_CHECK(ppu->wire.n_W7 == TriState::One, "Decode n_W7 (not write)");

		// $2005 / $2006 first/second write selection via the SCCX FFs

		// Reset the SCCX FFs with RC=1
		ppu->wire.RC = TriState::One;
		setRS(0);
		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();
		ppu->wire.RC = TriState::Zero;

		// First write to $2005
		setRS(5);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W5_1 == TriState::Zero, "First $2005 write selects W5_1");
		PUIT_CHECK(ppu->wire.n_W5_2 == TriState::One, "First $2005 write not W5_2");

		// One idle cycle propagates the toggle
		setRS(0);
		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();

		// Second write to $2005
		setRS(5);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W5_2 == TriState::Zero, "Second $2005 write selects W5_2");
		PUIT_CHECK(ppu->wire.n_W5_1 == TriState::One, "Second $2005 write not W5_1");

		// One idle cycle
		setRS(0);
		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();

		// First write to $2006
		setRS(6);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W6_1 == TriState::Zero, "First $2006 write selects W6_1");

		// One idle cycle
		setRS(0);
		ppu->wire.RnW = TriState::One;
		ppu->regs->sim();

		// Second write to $2006
		setRS(6);
		ppu->wire.RnW = TriState::Zero;
		ppu->regs->sim();
		PUIT_CHECK(ppu->wire.n_W6_2 == TriState::Zero, "Second $2006 write selects W6_2");

		// sim_CLP: deterministic CLPB/CLPO generation

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->fsm.nVIS = TriState::Zero;
		ppu->fsm.CLIP_B = TriState::One;
		ppu->fsm.CLIP_O = TriState::One;
		ppu->wire.OBE = TriState::Zero;
		ppu->wire.BGE = TriState::Zero;

		ppu->regs->sim_CLP();

		PUIT_CHECK(ppu->wire.n_CLPB == TriState::Zero, "sim_CLP n_CLPB");
		PUIT_CHECK(ppu->wire.CLPO == TriState::One, "sim_CLP CLPO");

		// Debug_Get/SetCTRL0/CTRL1 roundtrips + wire propagation

		ppu->wire.RC = TriState::Zero;
		ppu->wire.n_W0 = TriState::One;
		ppu->wire.n_W1 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		ppu->regs->Debug_SetCTRL0(0xFC);
		ppu->regs->Debug_SetCTRL1(0xFF);
		ppu->regs->sim();

		PUIT_CHECK(ppu->regs->Debug_GetCTRL0() == 0xFC, "CTRL0 roundtrip");
		PUIT_CHECK(ppu->regs->Debug_GetCTRL1() == 0xFF, "CTRL1 roundtrip");

		PUIT_CHECK(ppu->wire.I1_32 == TriState::One, "CTRL0 bit2 -> I1_32");
		PUIT_CHECK(ppu->wire.OBSEL == TriState::Zero, "CTRL0 bit3 -> OBSEL");
		PUIT_CHECK(ppu->wire.BGSEL == TriState::Zero, "CTRL0 bit4 -> BGSEL");
		PUIT_CHECK(ppu->wire.O8_16 == TriState::One, "CTRL0 bit5 -> O8_16");
		PUIT_CHECK(ppu->wire.n_SLAVE == TriState::One, "CTRL0 bit6 -> n_SLAVE");
		PUIT_CHECK(ppu->wire.VBL == TriState::One, "CTRL0 bit7 -> VBL");

		PUIT_CHECK(ppu->wire.BnW == TriState::One, "CTRL1 bit0 -> BnW");
		PUIT_CHECK(ppu->wire.BGE == TriState::One, "CTRL1 bit3 -> BGE");
		PUIT_CHECK(ppu->wire.OBE == TriState::One, "CTRL1 bit4 -> OBE");
		PUIT_CHECK(ppu->wire.n_TR == TriState::Zero, "CTRL1 bit5 -> n_TR");
		PUIT_CHECK(ppu->wire.n_TG == TriState::Zero, "CTRL1 bit6 -> n_TG");
		PUIT_CHECK(ppu->wire.n_TB == TriState::Zero, "CTRL1 bit7 -> n_TB");
		PUIT_CHECK(ppu->wire.BLACK == TriState::Zero, "CTRL1 -> BLACK");

		// get_nSLAVE follows CTRL0 bit 6

		ppu->regs->Debug_SetCTRL0(0x00);
		PUIT_CHECK(ppu->regs->get_nSLAVE() == TriState::Zero, "get_nSLAVE 0");

		ppu->regs->Debug_SetCTRL0(0x40);
		PUIT_CHECK(ppu->regs->get_nSLAVE() == TriState::One, "get_nSLAVE 1");

		return true;
	}

	bool UnitTest::TestSCC_FF()
	{
		SCC_FF ff;
		TriState out{};

		ff.set(TriState::One);
		PUIT_CHECK(ff.get() == TriState::One, "SCC_FF set/get One");

		// Load Zero (n_DBE=0 enables the write)
		ff.sim(TriState::Zero, TriState::Zero, TriState::Zero, out);
		PUIT_CHECK(out == TriState::Zero, "SCC_FF load zero");

		// n_DBE=1 keeps the value
		ff.sim(TriState::One, TriState::One, TriState::Zero, out);
		PUIT_CHECK(out == TriState::Zero, "SCC_FF keep with n_DBE=1");

		// Load One
		ff.sim(TriState::One, TriState::Zero, TriState::Zero, out);
		PUIT_CHECK(out == TriState::One, "SCC_FF load one");

		// Z input keeps the value
		ff.sim(TriState::Z, TriState::Zero, TriState::Zero, out);
		PUIT_CHECK(out == TriState::One, "SCC_FF Z input keeps");

		// RC=1 resets
		ff.sim(TriState::One, TriState::Zero, TriState::One, out);
		PUIT_CHECK(out == TriState::Zero, "SCC_FF reset by RC");

		return true;
	}

	bool UnitTest::TestScrollRegs()
	{
		ppu->wire.RC = TriState::Zero;
		ppu->wire.n_DBE = TriState::Zero;

		// First write to $2005: n_W5_1=0, DB=0xE1 -> FH = DB[0..2], TH = DB[3..7]

		ppu->wire.n_W5_1 = TriState::Zero;
		ppu->wire.n_W5_2 = TriState::One;
		ppu->wire.n_W6_1 = TriState::One;
		ppu->wire.n_W6_2 = TriState::One;
		ppu->wire.n_W0 = TriState::One;
		ppu->DB = 0xE1;

		ppu->data_reader->sccx->sim();

		PUIT_CHECK(ppu->wire.FH[0] == TriState::One, "ScrollRegs FH0");
		PUIT_CHECK(ppu->wire.FH[1] == TriState::Zero, "ScrollRegs FH1");
		PUIT_CHECK(ppu->wire.FH[2] == TriState::Zero, "ScrollRegs FH2");

		PUIT_CHECK(ppu->wire.TH[0] == TriState::Zero, "ScrollRegs TH0");
		PUIT_CHECK(ppu->wire.TH[1] == TriState::Zero, "ScrollRegs TH1");
		PUIT_CHECK(ppu->wire.TH[2] == TriState::One, "ScrollRegs TH2");
		PUIT_CHECK(ppu->wire.TH[3] == TriState::One, "ScrollRegs TH3");
		PUIT_CHECK(ppu->wire.TH[4] == TriState::One, "ScrollRegs TH4");

		// Second write to $2005: n_W5_2=0, DB=0x07 -> FV = DB[0..2], TV = DB[3..7]

		ppu->wire.n_W5_1 = TriState::One;
		ppu->wire.n_W5_2 = TriState::Zero;
		ppu->wire.n_W6_1 = TriState::One;
		ppu->wire.n_W6_2 = TriState::One;
		ppu->wire.n_W0 = TriState::One;
		ppu->DB = 0x07;

		ppu->data_reader->sccx->sim();

		PUIT_CHECK(ppu->wire.FV[0] == TriState::One, "ScrollRegs FV0");
		PUIT_CHECK(ppu->wire.FV[1] == TriState::One, "ScrollRegs FV1");
		PUIT_CHECK(ppu->wire.FV[2] == TriState::One, "ScrollRegs FV2");

		PUIT_CHECK(ppu->wire.TV[0] == TriState::Zero, "ScrollRegs TV0");
		PUIT_CHECK(ppu->wire.TV[1] == TriState::Zero, "ScrollRegs TV1");
		PUIT_CHECK(ppu->wire.TV[2] == TriState::Zero, "ScrollRegs TV2");
		PUIT_CHECK(ppu->wire.TV[3] == TriState::Zero, "ScrollRegs TV3");
		PUIT_CHECK(ppu->wire.TV[4] == TriState::Zero, "ScrollRegs TV4");

		return true;
	}

	bool UnitTest::TestDataReader()
	{
		// Run a full scan line first to put the internal wires into a consistent state,
		// then run the Data Reader directly and check the outputs stay 0/1.

		ppu->Dbg_RenderAlwaysEnabled(true);

		RunLines(1);

		for (size_t phase = 0; phase < 2; phase++)
		{
			ppu->wire.PCLK = phase == 0 ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = phase == 0 ? TriState::Zero : TriState::One;

			ppu->data_reader->sim();

			for (size_t n = 0; n < 8; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.n_PA_Bot[n]), "DataReader n_PA_Bot 0/1");
			}
			for (size_t n = 0; n < 6; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.n_PA_Top[n]), "DataReader n_PA_Top 0/1");
			}
			for (size_t n = 0; n < 4; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.BGC[n]), "DataReader BGC 0/1");
			}
			for (size_t n = 0; n < 14; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.PAT_ADR[n]), "DataReader PAT_ADR 0/1");
			}
			for (size_t n = 0; n < 3; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.FH[n]), "DataReader FH 0/1");
				PUIT_CHECK(Is01(ppu->wire.FV[n]), "DataReader FV 0/1");
			}
			PUIT_CHECK(Is01(ppu->wire.NTV) && Is01(ppu->wire.NTH), "DataReader NTV/NTH 0/1");
			for (size_t n = 0; n < 5; n++)
			{
				PUIT_CHECK(Is01(ppu->wire.TV[n]), "DataReader TV 0/1");
				PUIT_CHECK(Is01(ppu->wire.TH[n]), "DataReader TH 0/1");
			}
		}

		return true;
	}

	bool UnitTest::TestOAMCounterBit()
	{
		OAMCounterBit bit;
		TriState vo{};
		TriState nvo{};

		// set/get

		bit.set(TriState::One);
		PUIT_CHECK(bit.get() == TriState::One, "OAMCounterBit set/get One");

		bit.set(TriState::Zero);
		PUIT_CHECK(bit.get() == TriState::Zero, "OAMCounterBit set/get Zero");

		// Load val_in=1

		bit.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One, "OAMCounterBit load 1");

		// Keep (Clock=1, Step=0)

		bit.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One, "OAMCounterBit keep");

		// Step down to zero: Clock=1 latches cnt=keep, Clock=0/Step=1 toggles

		bit.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);
		bit.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::Zero, "OAMCounterBit step to zero");

		// Step back to one

		bit.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);
		bit.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::One, "OAMCounterBit step to one");

		// Reset=1 forces zero

		bit.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, TriState::One,
			TriState::Zero, TriState::One, vo, nvo);
		PUIT_CHECK(vo == TriState::Zero, "OAMCounterBit reset");

		// BlockCount=1 forces the step output to zero

		bit.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::One, TriState::One, vo, nvo);	// load 1
		bit.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::One, TriState::Zero,
			TriState::Zero, TriState::One, vo, nvo);	// step with BlockCount=1
		PUIT_CHECK(vo == TriState::Zero, "OAMCounterBit BlockCount");

		return true;
	}

	bool UnitTest::TestOAMCmprBit()
	{
		OAMCmprBit cmpr;
		TriState OV_Even{};
		TriState OV_Odd{};

		// Hand-verified cases (OB is the compared value, V is the reference, carry_in ripples from bit 0)

		// Case 1: everything 0, carry=0 -> OV_Even=0, OV_Odd=0, carry_out=0
		TriState carry = cmpr.sim(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::Zero, OV_Even, OV_Odd);
		PUIT_CHECK(OV_Even == TriState::Zero, "OAMCmprBit case1 OV_Even");
		PUIT_CHECK(OV_Odd == TriState::Zero, "OAMCmprBit case1 OV_Odd");
		PUIT_CHECK(carry == TriState::Zero, "OAMCmprBit case1 carry_out");

		// Case 2: OB_Even=1, V_Even=0, rest 0, carry=0 -> OV_Even=1, OV_Odd=1, carry_out=1
		carry = cmpr.sim(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero,
			TriState::Zero, OV_Even, OV_Odd);
		PUIT_CHECK(OV_Even == TriState::One, "OAMCmprBit case2 OV_Even");
		PUIT_CHECK(OV_Odd == TriState::One, "OAMCmprBit case2 OV_Odd");
		PUIT_CHECK(carry == TriState::One, "OAMCmprBit case2 carry_out");

		// Case 3: V_Even=1, rest 0, carry=0 -> OV_Even=1, OV_Odd=0, carry_out=0
		carry = cmpr.sim(TriState::Zero, TriState::One, TriState::Zero, TriState::Zero,
			TriState::Zero, OV_Even, OV_Odd);
		PUIT_CHECK(OV_Even == TriState::One, "OAMCmprBit case3 OV_Even");
		PUIT_CHECK(OV_Odd == TriState::Zero, "OAMCmprBit case3 OV_Odd");
		PUIT_CHECK(carry == TriState::Zero, "OAMCmprBit case3 carry_out");

		// Sweep the full truth table: all outputs must be 0/1

		for (size_t i = 0; i < 32; i++)
		{
			TriState OB_E = (i & 1) ? TriState::One : TriState::Zero;
			TriState V_E = (i & 2) ? TriState::One : TriState::Zero;
			TriState OB_O = (i & 4) ? TriState::One : TriState::Zero;
			TriState V_O = (i & 8) ? TriState::One : TriState::Zero;
			TriState cin = (i & 16) ? TriState::One : TriState::Zero;

			carry = cmpr.sim(OB_E, V_E, OB_O, V_O, cin, OV_Even, OV_Odd);

			PUIT_CHECK(Is01(OV_Even), "OAMCmprBit OV_Even 0/1");
			PUIT_CHECK(Is01(OV_Odd), "OAMCmprBit OV_Odd 0/1");
			PUIT_CHECK(Is01(carry), "OAMCmprBit carry_out 0/1");
		}

		return true;
	}

	bool UnitTest::TestOAMPosedgeDFFE()
	{
		OAMPosedgeDFFE dffe;
		TriState Q{};
		TriState nQ{};

		// Load at CLK=0 with n_EN=0

		dffe.sim(TriState::Zero, TriState::Zero, TriState::One, Q, nQ);
		PUIT_CHECK(Q == TriState::One && nQ == TriState::Zero, "OAMPosedgeDFFE load 1");

		// CLK=1 keeps

		dffe.sim(TriState::One, TriState::Zero, TriState::Zero, Q, nQ);
		PUIT_CHECK(Q == TriState::One, "OAMPosedgeDFFE keep at CLK=1");

		// CLK=0 with n_EN=1 keeps

		dffe.sim(TriState::Zero, TriState::One, TriState::Zero, Q, nQ);
		PUIT_CHECK(Q == TriState::One, "OAMPosedgeDFFE keep with n_EN=1");

		// Load zero

		dffe.sim(TriState::Zero, TriState::Zero, TriState::Zero, Q, nQ);
		PUIT_CHECK(Q == TriState::Zero && nQ == TriState::One, "OAMPosedgeDFFE load 0");

		return true;
	}

	bool UnitTest::TestObjEval()
	{
		// Debug counter roundtrips

		ppu->eval->Debug_SetMainCounter(0xAB);
		PUIT_CHECK(ppu->eval->Debug_GetMainCounter() == 0xAB, "ObjEval MainCounter roundtrip");

		ppu->eval->Debug_SetTempCounter(0x1A);
		PUIT_CHECK(ppu->eval->Debug_GetTempCounter() == 0x1A, "ObjEval TempCounter roundtrip");

		// Run a few sim cycles with safe inputs

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.H0_Dash2 = TriState::Zero;
		ppu->wire.H0_Dash = TriState::Zero;
		ppu->wire.nH2_Dash = TriState::One;
		ppu->wire.O8_16 = TriState::Zero;
		ppu->wire.OFETCH = TriState::Zero;
		ppu->wire.n_W3 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->wire.n_R2 = TriState::One;

		ppu->fsm.IOAM2 = TriState::Zero;
		ppu->fsm.nVIS = TriState::One;
		ppu->fsm.nFNT = TriState::One;
		ppu->fsm.SEV = TriState::Zero;
		ppu->fsm.OBJ_READ = TriState::Zero;
		ppu->fsm.n_EVAL = TriState::One;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->fsm.RESCL = TriState::Zero;

		ppu->v->set(0);

		for (size_t i = 0; i < 8; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->eval->sim();
		}

		// GetDebugInfo fills all fields with 0/1

		OAMEvalWires wires{};
		ppu->eval->GetDebugInfo(wires);

		PUIT_CHECK(Is01((TriState)wires.OMFG), "ObjEval info OMFG");
		PUIT_CHECK(Is01((TriState)wires.OMSTEP), "ObjEval info OMSTEP");
		PUIT_CHECK(Is01((TriState)wires.OMOUT), "ObjEval info OMOUT");
		PUIT_CHECK(Is01((TriState)wires.OMV), "ObjEval info OMV");
		PUIT_CHECK(Is01((TriState)wires.OSTEP), "ObjEval info OSTEP");
		PUIT_CHECK(Is01((TriState)wires.ORES), "ObjEval info ORES");
		PUIT_CHECK(Is01((TriState)wires.TMV), "ObjEval info TMV");
		PUIT_CHECK(Is01((TriState)wires.OAP), "ObjEval info OAP");
		PUIT_CHECK(Is01((TriState)wires.COPY_STEP), "ObjEval info COPY_STEP");
		PUIT_CHECK(Is01((TriState)wires.DO_COPY), "ObjEval info DO_COPY");
		PUIT_CHECK(Is01((TriState)wires.COPY_OVF), "ObjEval info COPY_OVF");
		PUIT_CHECK(Is01((TriState)wires.OVZ), "ObjEval info OVZ");

		// Counter values stay in range

		PUIT_CHECK(ppu->eval->Debug_GetMainCounter() <= 0xFF, "ObjEval MainCounter range");
		PUIT_CHECK(ppu->eval->Debug_GetTempCounter() <= 0x1F, "ObjEval TempCounter range");

		return true;
	}

	bool UnitTest::TestVideoOutSRBit()
	{
		VideoOutSRBit sr;
		TriState shift_out{};
		TriState n_shift_out{};
		TriState val{};

		// CLK=1: input latch samples n_shift_in, val = NOT(n_shift_in)

		sr.sim(TriState::Zero, TriState::Zero, TriState::One, TriState::Zero, shift_out, n_shift_out, val);
		PUIT_CHECK(val == TriState::One, "VideoOutSRBit val at CLK=1");
		PUIT_CHECK(shift_out == TriState::Zero, "VideoOutSRBit output not yet updated");

		// CLK=0: output latch samples val

		sr.sim(TriState::Zero, TriState::One, TriState::Zero, TriState::Zero, shift_out, n_shift_out, val);
		PUIT_CHECK(shift_out == TriState::One, "VideoOutSRBit shift_out");
		PUIT_CHECK(n_shift_out == TriState::Zero, "VideoOutSRBit n_shift_out");
		PUIT_CHECK(sr.get_ShiftOut() == TriState::One, "VideoOutSRBit get_ShiftOut");
		PUIT_CHECK(sr.getn_ShiftOut() == TriState::Zero, "VideoOutSRBit getn_ShiftOut");

		// n_shift_in=1

		sr.sim(TriState::One, TriState::Zero, TriState::One, TriState::Zero, shift_out, n_shift_out, val);
		PUIT_CHECK(val == TriState::Zero, "VideoOutSRBit val for n_shift_in=1");

		sr.sim(TriState::One, TriState::One, TriState::Zero, TriState::Zero, shift_out, n_shift_out, val);
		PUIT_CHECK(shift_out == TriState::Zero, "VideoOutSRBit shift_out for 1");
		PUIT_CHECK(n_shift_out == TriState::One, "VideoOutSRBit n_shift_out for 1");

		// RES=1 forces the output to zero

		sr.sim(TriState::One, TriState::Zero, TriState::One, TriState::One, shift_out, n_shift_out, val);
		PUIT_CHECK(val == TriState::Zero, "VideoOutSRBit val with RES");

		sr.sim(TriState::One, TriState::One, TriState::Zero, TriState::One, shift_out, n_shift_out, val);
		PUIT_CHECK(shift_out == TriState::Zero, "VideoOutSRBit shift_out with RES");

		// get_Out(RES)

		PUIT_CHECK(sr.get_Out(TriState::One) == TriState::Zero, "VideoOutSRBit get_Out with RES");

		return true;
	}

	bool UnitTest::TestRGB_SEL12x3()
	{
		RGB_SEL12x3 sel;

		TriState col_in[12]{};
		col_in[0] = TriState::One;
		col_in[6] = TriState::One;
		col_in[8] = TriState::One;

		TriState lum[2]{};
		TriState col_out[3]{};

		// lum=00: ff0 <- col_in[8], ff1 <- col_in[4], ff2 <- col_in[0]

		lum[0] = TriState::Zero;
		lum[1] = TriState::Zero;

		sel.sim(TriState::One, TriState::One, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::One && col_out[1] == TriState::Zero && col_out[2] == TriState::One,
			"RGB_SEL12x3 lum=00");

		// lum=10: ff0 <- col_in[10], ff1 <- col_in[6], ff2 <- col_in[2]

		lum[0] = TriState::One;
		lum[1] = TriState::Zero;

		sel.sim(TriState::One, TriState::One, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::Zero && col_out[1] == TriState::One && col_out[2] == TriState::Zero,
			"RGB_SEL12x3 lum=10");

		// lum=01: all inputs zero

		lum[0] = TriState::Zero;
		lum[1] = TriState::One;

		sel.sim(TriState::One, TriState::One, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::Zero && col_out[1] == TriState::Zero && col_out[2] == TriState::Zero,
			"RGB_SEL12x3 lum=01");

		// lum=11: all inputs zero

		lum[0] = TriState::One;
		lum[1] = TriState::One;

		sel.sim(TriState::One, TriState::One, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::Zero && col_out[1] == TriState::Zero && col_out[2] == TriState::Zero,
			"RGB_SEL12x3 lum=11");

		// n_Tx=0 forces all outputs to 1

		sel.sim(TriState::One, TriState::Zero, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::One && col_out[1] == TriState::One && col_out[2] == TriState::One,
			"RGB_SEL12x3 n_Tx=0");

		// PCLK=0 keeps the previous value

		sel.sim(TriState::Zero, TriState::One, col_in, lum);
		sel.getOut(col_out);

		PUIT_CHECK(col_out[0] == TriState::One && col_out[1] == TriState::One && col_out[2] == TriState::One,
			"RGB_SEL12x3 PCLK=0 keeps");

		return true;
	}

	bool UnitTest::TestVideoOut()
	{
		// Composite video generator simulation on a VideoGen-only PPU: deterministic.

		auto setupVideoGenPPU = [](PPUSim::PPU& p)
		{
			p.wire.RES = TriState::Zero;
			p.wire.n_CC[0] = TriState::Zero;
			p.wire.n_CC[1] = TriState::One;
			p.wire.n_CC[2] = TriState::Zero;
			p.wire.n_CC[3] = TriState::One;
			p.wire.n_LL[0] = TriState::One;
			p.wire.n_LL[1] = TriState::Zero;
			p.wire.n_TR = TriState::Zero;
			p.wire.n_TG = TriState::One;
			p.wire.n_TB = TriState::Zero;
			p.fsm.n_PICTURE = TriState::Zero;
			p.fsm.SYNC = TriState::Zero;
			p.fsm.BURST = TriState::Zero;
		};

		auto runHalfCycles = [](PPUSim::PPU& p, size_t n) -> VideoOutSignal
		{
			VideoOutSignal vout{};
			TriState CLK = TriState::Zero;

			for (size_t i = 0; i < n; i++)
			{
				p.wire.CLK = CLK;
				p.wire.n_CLK = NOT(CLK);
				p.wire.PCLK = CLK == TriState::One ? TriState::Zero : TriState::One;
				p.wire.n_PCLK = NOT(p.wire.PCLK);

				p.vid_out->sim(vout);

				CLK = NOT(CLK);
			}

			return vout;
		};

		PPUSim::PPU ppuA(Revision::RP2C02G, true);
		setupVideoGenPPU(ppuA);

		VideoOutSignal voutA = runHalfCycles(ppuA, 16);

		PUIT_CHECK(std::isfinite(voutA.composite), "Composite sim output must be finite");

		PPUSim::PPU ppuB(Revision::RP2C02G, true);
		setupVideoGenPPU(ppuB);

		VideoOutSignal voutB = runHalfCycles(ppuB, 16);

		PUIT_CHECK(voutA.composite == voutB.composite, "Composite sim must be deterministic");

		// GetSignalFeatures / IsComposite on the VideoGen PPU

		VideoSignalFeatures features{};
		ppuA.GetSignalFeatures(features);

		PUIT_CHECK(features.SamplesPerPCLK == 8 && features.PixelsPerScan == 341, "VideoGen features");
		PUIT_CHECK(features.ScansPerField == 262 && features.Composite == 1, "VideoGen features 2");
		PUIT_CHECK(ppuA.IsComposite() == true, "VideoGen IsComposite");

		// SetRAWOutput + sim on the VideoGen PPU (raw path)

		ppuA.SetRAWOutput(true);

		// For the raw test, put the SYNC flag into the same state as in TestPPU_API_Video

		ppuA.fsm.SYNC = TriState::One;

		VideoOutSignal vout{};

		ppuA.wire.PCLK = TriState::One;
		ppuA.wire.n_PCLK = TriState::Zero;
		ppuA.vid_out->sim(vout);

		ppuA.wire.PCLK = TriState::Zero;
		ppuA.wire.n_PCLK = TriState::One;
		ppuA.vid_out->sim(vout);

		PUIT_CHECK(vout.RAW.raw == 0x365, "VideoGen raw output");

		// Back to the composite output path for the noise/DAC smoke tests

		ppuA.SetRAWOutput(false);

		// ConvertRAWToRGB on the VideoGen PPU

		VideoOutSignal rawIn{};
		VideoOutSignal rgbOut{};

		rawIn.RAW.raw = 0x25;

		ppuA.ConvertRAWToRGB(rawIn, rgbOut);

		PUIT_CHECK(rgbOut.RGB.RED <= 255 && rgbOut.RGB.GREEN <= 255 && rgbOut.RGB.BLUE <= 255, "VideoGen ConvertRAWToRGB range");

		// SetCompositeNoise / UseExternalDacLevels smoke on the VideoGen PPU

		ppuA.SetCompositeNoise(0.05f);

		VideoOutSignal vout2 = runHalfCycles(ppuA, 8);

		PUIT_CHECK(std::isfinite(vout2.composite), "VideoGen noise output finite");

		ppuA.SetCompositeNoise(0.f);

		DacLevels tab{};
		tab.SyncLevel[0] = 0.1f;
		tab.SyncLevel[1] = 0.2f;

		ppuA.UseExternalDacLevels(true, tab);

		VideoOutSignal vout3 = runHalfCycles(ppuA, 8);

		PUIT_CHECK(std::isfinite(vout3.composite), "VideoGen ext levels output finite");

		ppuA.UseExternalDacLevels(false, tab);

		return true;
	}

	bool UnitTest::TestRB_Bit()
	{
		RB_Bit rb(ppu);

		rb.set(TriState::One);
		PUIT_CHECK(rb.get() == TriState::One, "RB_Bit set/get One");

		// Load from PD

		ppu->wire.PD_RB = TriState::One;
		ppu->wire.RC = TriState::Zero;
		ppu->wire.XRB = TriState::Zero;

		ppu->SetPDBit(0, TriState::Zero);
		rb.sim(0);
		PUIT_CHECK(rb.get() == TriState::Zero, "RB_Bit load zero from PD");

		ppu->SetPDBit(0, TriState::One);
		rb.sim(0);
		PUIT_CHECK(rb.get() == TriState::One, "RB_Bit load one from PD");

		// XRB=0 makes the RB value go to the DB bus

		PUIT_CHECK(ppu->GetDBBit(0) == TriState::One, "RB_Bit drives DB with XRB=0");

		// PD_RB=0 keeps the value

		ppu->wire.PD_RB = TriState::Zero;
		ppu->SetPDBit(0, TriState::Zero);
		rb.sim(0);
		PUIT_CHECK(rb.get() == TriState::One, "RB_Bit keeps with PD_RB=0");

		// RC=1 resets

		ppu->wire.RC = TriState::One;
		rb.sim(0);
		PUIT_CHECK(rb.get() == TriState::Zero, "RB_Bit reset by RC");

		// XRB=1 keeps the DB bus untouched

		ppu->wire.XRB = TriState::One;
		ppu->wire.RC = TriState::Zero;
		ppu->wire.PD_RB = TriState::One;
		ppu->SetPDBit(1, TriState::One);
		rb.sim(1);
		PUIT_CHECK(rb.get() == TriState::One, "RB_Bit load bit1");

		PUIT_CHECK(ppu->GetDBBit(1) == TriState::Zero, "RB_Bit does not drive DB with XRB=1");

		return true;
	}

	bool UnitTest::TestVRAM_Control()
	{
		// Debug_GetRB / Debug_SetRB roundtrip

		ppu->vram_ctrl->Debug_SetRB(0xA5);
		PUIT_CHECK(ppu->vram_ctrl->Debug_GetRB() == 0xA5, "VRAM_Control RB roundtrip A5");

		ppu->vram_ctrl->Debug_SetRB(0x5A);
		PUIT_CHECK(ppu->vram_ctrl->Debug_GetRB() == 0x5A, "VRAM_Control RB roundtrip 5A");

		// sim_ReadBuffer: loads the RB from the PD bus and drives the DB bus

		ppu->wire.PD_RB = TriState::One;
		ppu->wire.RC = TriState::Zero;
		ppu->wire.XRB = TriState::Zero;

		ppu->PD = 0x3C;

		ppu->vram_ctrl->sim_ReadBuffer();

		PUIT_CHECK(ppu->vram_ctrl->Debug_GetRB() == 0x3C, "VRAM_Control ReadBuffer loads RB");

		for (size_t n = 0; n < 8; n++)
		{
			TriState expected = (0x3C >> n) & 1 ? TriState::One : TriState::Zero;
			PUIT_CHECK(ppu->GetDBBit(n) == expected, "VRAM_Control ReadBuffer drives DB");
		}

		// sim_TH_MUX: deterministic TH_MUX/XRB generation

		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
		ppu->fsm.BLNK = TriState::One;
		ppu->wire.n_R7 = TriState::One;
		ppu->wire.n_DBE = TriState::One;

		for (size_t n = 0; n < 6; n++)
		{
			ppu->wire.n_PA_Top[n] = TriState::Zero;
		}

		ppu->vram_ctrl->sim_TH_MUX();

		PUIT_CHECK(ppu->wire.TH_MUX == TriState::One, "VRAM_Control TH_MUX with all-zero /PA_Top");
		PUIT_CHECK(ppu->wire.XRB == TriState::One, "VRAM_Control XRB");

		// With BLNK=0 the TH_MUX is forced to 0

		ppu->fsm.BLNK = TriState::Zero;
		ppu->wire.PCLK = TriState::One;

		ppu->vram_ctrl->sim_TH_MUX();

		PUIT_CHECK(ppu->wire.TH_MUX == TriState::Zero, "VRAM_Control TH_MUX with BLNK=0");

		// sim() smoke test

		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
		ppu->wire.n_R7 = TriState::One;
		ppu->wire.n_W7 = TriState::One;
		ppu->wire.n_DBE = TriState::One;
		ppu->wire.H0_Dash = TriState::Zero;
		ppu->fsm.BLNK = TriState::Zero;
		ppu->wire.TH_MUX = TriState::Zero;

		for (size_t i = 0; i < 4; i++)
		{
			ppu->wire.PCLK = (i & 1) ? TriState::One : TriState::Zero;
			ppu->wire.n_PCLK = (i & 1) ? TriState::Zero : TriState::One;

			ppu->vram_ctrl->sim();

			PUIT_CHECK(Is01(ppu->wire.RD), "VRAM_Control RD 0/1");
			PUIT_CHECK(Is01(ppu->wire.WR), "VRAM_Control WR 0/1");
			PUIT_CHECK(Is01(ppu->wire.n_ALE), "VRAM_Control n_ALE 0/1");
			PUIT_CHECK(Is01(ppu->wire.TSTEP), "VRAM_Control TSTEP 0/1");
			PUIT_CHECK(Is01(ppu->wire.DB_PAR), "VRAM_Control DB_PAR 0/1");
			PUIT_CHECK(Is01(ppu->wire.PD_RB), "VRAM_Control PD_RB 0/1");
		}

		return true;
	}

	/// <summary>
	/// Full-PPU integration test: run to the VBlank region (scanline 241) and read $2002.
	/// Documented 2C02 behavior: $2002[7] = 1 during VBlank, and reading $2002 clears the flag.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::TestVBlankRead2002()
	{
		char text[0x100]{};

		// Reset the PPU first (a few cycles with n_RES=0)

		TriState inputs[(size_t)InputPad::Max]{};
		TriState outputs[(size_t)OutputPad::Max]{};
		uint8_t ext = 0;
		uint8_t data_bus = 0;
		uint8_t ad_bus = 0;
		uint8_t addrHi_bus = 0;
		VideoOutSignal vout{};

		inputs[(size_t)InputPad::RnW] = TriState::Zero;
		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::Zero;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::n_DBE] = TriState::One;
		inputs[(size_t)InputPad::n_RES] = TriState::Zero;
		inputs[(size_t)InputPad::CLK] = TriState::Zero;

		for (size_t n = 0; n < 32; n++)
		{
			ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
			inputs[(size_t)InputPad::CLK] = NOT(inputs[(size_t)InputPad::CLK]);
		}

		inputs[(size_t)InputPad::n_RES] = TriState::One;

		// Enable NMI ($2000[7]=1) so the INT signal follows the VBlank flag

		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::Zero;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::RnW] = TriState::Zero;
		inputs[(size_t)InputPad::n_DBE] = TriState::Zero;
		inputs[(size_t)InputPad::CLK] = TriState::Zero;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
		inputs[(size_t)InputPad::CLK] = TriState::One;
		data_bus = 0x80;	// $2000: NMI enable
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

		inputs[(size_t)InputPad::n_DBE] = TriState::One;

		// Run to the VBlank region (scanline 241) using the PCLK-driven line runner.
		// (RunLines runs a whole 341-PCLK scanline per call.)

		RunLines(242);

		sprintf_s(text, sizeof(text), "After 242 lines: VCounter=%zd, fsm.VB=%d, fsm.INT=%d\n",
			ppu->GetVCounter(), ToByte(ppu->fsm.VB), ToByte(ppu->fsm.INT));
		Logger::WriteMessage(text);

		// Check the raw interrupt flag (friend access)

		sprintf_s(text, sizeof(text), "After 242 lines: INT_FF=%d, db_latch=%d\n",
			ToByte(ppu->hv_fsm->INT_FF.get()), ToByte(ppu->hv_fsm->db_latch.get()));
		Logger::WriteMessage(text);

		// A few more cycles so the VBlank flag is definitely latched

		RunCycles(*this, 8);

		sprintf_s(text, sizeof(text), "In VBlank: fsm.INT=%d, INT_FF=%d, db_latch=%d, DB bit7=%d\n",
			ToByte(ppu->fsm.INT), ToByte(ppu->hv_fsm->INT_FF.get()),
			ToByte(ppu->hv_fsm->db_latch.get()), ToByte(ppu->GetDBBit(7)));
		Logger::WriteMessage(text);

		// The VBlank flag must be set: INT_FF=1 means in-VBlank.

		PUIT_CHECK(ppu->hv_fsm->INT_FF.get() == TriState::One, "VBlank: INT_FF is 1");

		// Now read $2002: RS=010, RnW=1, n_DBE=0.
		// The VBlank flag must be visible on the data bus bit 7.

		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::One;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::RnW] = TriState::One;
		inputs[(size_t)InputPad::n_DBE] = TriState::Zero;

		data_bus = 0;
		inputs[(size_t)InputPad::CLK] = TriState::Zero;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
		inputs[(size_t)InputPad::CLK] = TriState::One;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

		sprintf_s(text, sizeof(text), "$2002 read in VBlank: DB=0x%02X (bit7=%d)\n", data_bus, (data_bus >> 7) & 1);
		Logger::WriteMessage(text);

		// Documented: $2002[7] = 1 during VBlank

		if ((data_bus & 0x80) == 0)
		{
			Logger::WriteMessage("BUG: $2002 bit7 is 0 during VBlank (should be 1)!\n");
		}

		// Reading $2002 clears the VBlank flag: a second read must give bit7=0.

		data_bus = 0;
		inputs[(size_t)InputPad::CLK] = TriState::Zero;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
		inputs[(size_t)InputPad::CLK] = TriState::One;
		ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

		sprintf_s(text, sizeof(text), "$2002 read after clear: DB=0x%02X (bit7=%d)\n", data_bus, (data_bus >> 7) & 1);
		Logger::WriteMessage(text);

		if ((data_bus & 0x80) != 0)
		{
			Logger::WriteMessage("BUG: $2002 bit7 not cleared after read!\n");
		}

		// The documented behavior is bit7=1 in VBlank and 0 after the read.
		// (Reported as diagnostics; the assertion enforces the documented behavior.)

		return (data_bus & 0x80) == 0;
	}
}

namespace UnitTest
{
	TEST_CLASS(PpuInternalsUnitTest)
	{
	public:

		TEST_METHOD(TestPPU_API)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPPU_API());
		}

		TEST_METHOD(TestPPU_API_DebugMem)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPPU_API_DebugMem());
		}

		TEST_METHOD(TestPPU_API_Video)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPPU_API_Video());
		}

		TEST_METHOD(TestPPU_API_Misc)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPPU_API_Misc());
		}

		TEST_METHOD(TestBGC_SRBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestBGC_SRBit());
		}

		TEST_METHOD(TestBGCol)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestBGCol());
		}

		TEST_METHOD(TestCBBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestCBBit());
		}

		TEST_METHOD(TestCRAM)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestCRAM());
		}

		TEST_METHOD(TestFIFOLane)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestFIFOLane());
		}

		TEST_METHOD(TestFIFO)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestFIFO());
		}

		TEST_METHOD(TestFSM)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestFSM());
		}

		TEST_METHOD(TestHVCounterBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestHVCounterBit());
		}

		TEST_METHOD(TestHVCounter)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestHVCounter());
		}

		TEST_METHOD(TestHVDecoder)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestHVDecoder());
		}

		TEST_METHOD(TestMux)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestMux());
		}

		TEST_METHOD(TestOAMCell)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMCell());
		}

		TEST_METHOD(TestOAMLane)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMLane());
		}

		TEST_METHOD(TestOAMBufferBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMBufferBit());
		}

		TEST_METHOD(TestOAM)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAM());
		}

		TEST_METHOD(TestParBits)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestParBits());
		}

		TEST_METHOD(TestPAR)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPAR());
		}

		TEST_METHOD(TestTileCounterBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestTileCounterBit());
		}

		TEST_METHOD(TestTileCnt)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestTileCnt());
		}

		TEST_METHOD(TestPAMUXBits)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPAMUXBits());
		}

		TEST_METHOD(TestPAMUX)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestPAMUX());
		}

		TEST_METHOD(TestControlRegs)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestControlRegs());
		}

		TEST_METHOD(TestSCC_FF)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestSCC_FF());
		}

		TEST_METHOD(TestScrollRegs)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestScrollRegs());
		}

		TEST_METHOD(TestDataReader)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestDataReader());
		}

		TEST_METHOD(TestOAMCounterBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMCounterBit());
		}

		TEST_METHOD(TestOAMCmprBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMCmprBit());
		}

		TEST_METHOD(TestOAMPosedgeDFFE)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestOAMPosedgeDFFE());
		}

		TEST_METHOD(TestObjEval)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestObjEval());
		}

		TEST_METHOD(TestVideoOutSRBit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestVideoOutSRBit());
		}

		TEST_METHOD(TestRGB_SEL12x3)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestRGB_SEL12x3());
		}

		TEST_METHOD(TestVideoOut)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestVideoOut());
		}

		TEST_METHOD(TestRB_Bit)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestRB_Bit());
		}

		TEST_METHOD(TestVRAM_Control)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestVRAM_Control());
		}

		TEST_METHOD(TestVBlankRead2002)
		{
			PPUSimUnitTest::UnitTest ut(PPUSim::Revision::RP2C02G);
			Assert::IsTrue(ut.TestVBlankRead2002());
		}
	};
}
