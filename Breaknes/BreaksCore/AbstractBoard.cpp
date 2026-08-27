// It just so happens that not all methods are purely abstract.
#include "pch.h"

namespace Breaknes
{
	Board::Board(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1)
	{
		p1_type = p1;
		pal = new RGB_Triplet[8 * 64];
	}

	Board::~Board()
	{
		delete pal;
		if (ppu_regdump)
			delete ppu_regdump;
		if (apu_regdump)
			delete apu_regdump;
		if (nintendulator_log)
			delete nintendulator_log;
	}

	int Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		Mappers::CartridgeFactory cf(p1_type, nesImage, nesImageSize);
		cart = cf.GetInstance();

		if (!cart)
			return -1;

		if (!cart->Valid())
		{
			delete cart;
			cart = nullptr;

			dbg_hub->DisposeCartDebugInfo();
			dbg_hub->DisposeCartMemMap();

			return -2;
		}

		return 0;
	}

	void Board::EjectCartridge()
	{
		if (cart)
		{
			delete cart;
			cart = nullptr;

			dbg_hub->DisposeCartDebugInfo();
			dbg_hub->DisposeCartMemMap();
		}
	}

	void Board::Reset()
	{
	}

	bool Board::InResetState()
	{
		return false;
	}

	size_t Board::GetACLKCounter()
	{
		return apu->GetACLKCounter();
	}

	size_t Board::GetPHICounter()
	{
		return apu->GetPHICounter();
	}

	void Board::SampleAudioSignal(float* sample)
	{
		// NES/Famicom motherboards have some analog circuitry that acts as a LPF/HPF. Now the sound from the simulator comes without filtering, so there may be some ear-unpleasant harmonics.
		// TBD: Add LPF/HPF

		if (sample != nullptr)
		{
			// There are 2 resistors (12k and 20k) on the motherboard that equalize the AUX A/B levels and then mix.
			// Although the internal resistance of the AUX A/B terminals inside the APU we counted with the 100 ohm pull-ups -- the above 2 resistors are part of the "Board", so they count here.

			*sample = (aux.normalized.a * 0.4f /* 20k resistor */ + aux.normalized.b /* 12k resistor */) / 2.0f;

			// This is essentially "muting" AUX A, because the level of AUX A at its peak is about 300 mV, and the level of AUX B at its peak is about 1100 mV.
			// Accordingly, if you do just (A+B)/2, the square channels will be "overshoot".
		}
	}

	void Board::LoadRegDump(uint8_t* data, size_t data_size)
	{
	}

	void Board::EnablePpuRegDump(bool enable, char* regdump_dir)
	{
		if (enable) {

			char filename[0x200]{};
			sprintf(filename, "%s/ppu.regdump", regdump_dir);

			if (ppu_regdump) {
				delete ppu_regdump;
				ppu_regdump = nullptr;
			}
			ppu_regdump = new RegDumper("PPU", GetPHICounter(), filename);
			prev_phi_counter_for_ppuregdump = GetPHICounter();

			printf("PPU regdump enabled to file: %s\n", filename);
		}
		else {
			if (ppu_regdump) {
				delete ppu_regdump;
				ppu_regdump = nullptr;
			}

			printf("PPU regdump disabled\n");
		}
	}

	void Board::EnableApuRegDump(bool enable, char* regdump_dir)
	{
		if (enable) {

			char filename[0x200]{};
			sprintf(filename, "%s/apu.regdump", regdump_dir);

			if (apu_regdump) {
				delete apu_regdump;
				apu_regdump = nullptr;
			}
			apu_regdump = new RegDumper("APU", GetPHICounter(), filename);
			prev_phi_counter_for_apuregdump = GetPHICounter();

			printf("APU regdump enabled to file: %s\n", filename);
		}
		else {
			if (apu_regdump) {
				delete apu_regdump;
				apu_regdump = nullptr;
			}

			printf("APU regdump disabled\n");
		}
	}

	void Board::EnableNintendulatorLog(bool enable)
	{
		if (enable)
		{
			if (nintendulator_log == nullptr)
			{
				nintendulator_log = new NintendulatorLog();
			}

			nintendulator_log->SetReadByteCallback(ReadCPUMemCallback, this);
			nintendulator_log->Enable();
		}
		else
		{
			if (nintendulator_log != nullptr)
			{
				nintendulator_log->Disable();
			}
		}
	}

	uint8_t Board::ReadCPUMemCallback(void* opaque, uint16_t addr)
	{
		Board* board = (Board*)opaque;
		return board->ReadCPUMem(addr);
	}

	/// <summary>
	/// Check whether the core is fetching a new instruction and write one line to the
	/// Nintendulator log if it is enabled.
	/// The log has a discretization of 1 CPU cycle, while Step() simulates half-cycles
	/// (the core is simulated only on the PHI0 edges). The opcode fetch cycle is marked
	/// by the SYNC signal, so we log exactly once per instruction - on the rising edge
	/// of PHI0 (start of the PHI2 phase) of the fetch cycle.
	/// </summary>
	void Board::TreatCoreForNintendulatorLog(uint16_t addr_bus)
	{
		BaseLogic::TriState phi0 = apu->GetPHI0();

		if (nintendulator_log != nullptr && nintendulator_log->Enabled())
		{
			if (phi0 == BaseLogic::TriState::One &&
				prev_phi0_for_nintendulator_log == BaseLogic::TriState::Zero &&
				apu->GetPHI2() == BaseLogic::TriState::One &&
				apu->GetSYNC() == BaseLogic::TriState::One)
			{
				M6502Core::DebugInfo info{};
				core->getDebug(&info);

				NintendulatorLog::Entry entry{};
				entry.pc = addr_bus;		// Address bus holds the opcode address for the whole fetch cycle
				entry.a = info.AC;
				entry.x = info.X;
				entry.y = info.Y;
				entry.s = info.S;
				entry.p = (uint8_t)(
					(info.N_OUT << 7) | (info.V_OUT << 6) | 0x20 |	// The B bit is always 0, as in Nintendulator
					(info.D_OUT << 3) | (info.I_OUT << 2) | (info.Z_OUT << 1) | info.C_OUT);
				entry.phi_counter = apu->GetPHICounter();
				entry.h_counter = ppu->GetHCounter();
				entry.v_counter = ppu->GetVCounter();

				nintendulator_log->LogInstruction(entry);
			}
		}

		prev_phi0_for_nintendulator_log = phi0;
	}

	uint8_t Board::ReadCPUMem(uint16_t addr)
	{
		// CPU address space:
		//  $0000-$1FFF  WRAM (mirrored)
		//  $2000-$3FFF  PPU registers (mirrored every 8)
		//  $4000-$401F  APU & IO registers
		//  $4020-$FFFF  Cartridge PRG

		if (addr < 0x2000)
		{
			return ReadWRAM(addr);
		}
		else if (addr < 0x4000)
		{
			uint8_t reg = addr & 7;

			switch (reg)
			{
				case 0: return (uint8_t)(ppu->Dbg_ReadRegister(offsetof(PPUSim::PPU_Registers, CTRL0)) & 0xff);
				case 1: return (uint8_t)(ppu->Dbg_ReadRegister(offsetof(PPUSim::PPU_Registers, CTRL1)) & 0xff);

				// $2002 Status: the PPU drives bits 7-5, the rest is the open bus (reads as 1s)
				case 2:
				{
					PPUSim::PPU_Interconnects wires{};
					ppu->GetDebugInfo_Wires(wires);

					uint8_t val = 0x1f;
					// The PPU drives bit 7 as the inverted VBlank latch (see FSM::sim_VBlankInt)
					if (ppu->Dbg_GetVBLFlag() == BaseLogic::TriState::Zero)
						val |= 0x80;
					if (wires.n_SPR0HIT == BaseLogic::TriState::Zero)
						val |= 0x40;
					if (wires.SPR_OV == BaseLogic::TriState::One)
						val |= 0x20;
					return val;
				}

				case 4: return (uint8_t)(ppu->Dbg_ReadRegister(offsetof(PPUSim::PPU_Registers, OAMBuffer)) & 0xff);
				case 7: return (uint8_t)(ppu->Dbg_ReadRegister(offsetof(PPUSim::PPU_Registers, ReadBuffer)) & 0xff);

				default:
					return 0;
			}
		}
		else if (addr < 0x4020)
		{
			// APU/IO registers
			if (addr == 0x4015)
			{
				return apu->Dbg_GetStatus();
			}

			return 0;
		}
		else if (cart != nullptr)
		{
			return cart->Dbg_ReadPRGByte(addr);
		}

		return 0;
	}

	uint8_t Board::ReadWRAM(uint16_t addr)
	{
		return 0;
	}

	/// <summary>
	/// Check that the 6502 core is accessing the mapped APU/PPU registers and add an entry to regdump if necessary.
	/// The register operation is committed only on the PHI2 phase of the processor (the signal value is obtained directly from the core)
	/// If you don't do this, you may catch "bogus" register operations when the register address is set during PHI1.
	/// </summary>
	void Board::TreatCoreForRegdump(uint16_t addr_bus, uint8_t data_bus, BaseLogic::TriState phi2, BaseLogic::TriState rnw)
	{
		// APU Regdump
		if (apu_regdump && (addr_bus & ~MappedAPUMask) == MappedAPUBase) {

			uint64_t phi_now = GetPHICounter();
			if (prev_phi_counter_for_apuregdump != phi_now && phi2 == BaseLogic::TriState::One) {

				if (rnw == BaseLogic::TriState::One)
					apu_regdump->LogRegRead(phi_now, addr_bus & MappedAPUMask);
				else if (rnw == BaseLogic::TriState::Zero)
					apu_regdump->LogRegWrite(phi_now, addr_bus & MappedAPUMask, data_bus);
				prev_phi_counter_for_apuregdump = phi_now;
			}
		}
		// PPU Regump (isomorphic)
		if (ppu_regdump && (addr_bus & ~MappedPPUMask) == MappedPPUBase) {

			uint64_t phi_now = GetPHICounter();
			if (prev_phi_counter_for_ppuregdump != phi_now && phi2 == BaseLogic::TriState::One) {

				if (rnw == BaseLogic::TriState::One)
					ppu_regdump->LogRegRead(phi_now, addr_bus & MappedPPUMask);
				else if (rnw == BaseLogic::TriState::Zero)
					ppu_regdump->LogRegWrite(phi_now, addr_bus & MappedPPUMask, data_bus);
				prev_phi_counter_for_ppuregdump = phi_now;
			}
		}
	}

	void Board::GetApuSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		APUSim::AudioSignalFeatures feat{};
		apu->GetSignalFeatures(feat);
		*features = feat;
	}

	size_t Board::GetPCLKCounter()
	{
		return ppu->GetPCLKCounter();
	}

	void Board::SampleVideoSignal(PPUSim::VideoOutSignal* sample)
	{
		if (sample != nullptr)
		{
			*sample = vidSample;
		}
	}

	size_t Board::GetHCounter()
	{
		return ppu->GetHCounter();
	}

	size_t Board::GetVCounter()
	{
		return ppu->GetVCounter();
	}

	void Board::RenderAlwaysEnabled(bool enable)
	{
		ppu->Dbg_RenderAlwaysEnabled(enable);
	}

	void Board::GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features)
	{
		PPUSim::VideoSignalFeatures feat{};
		ppu->GetSignalFeatures(feat);
		*features = feat;
	}

	void Board::ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b)
	{
		if (!pal_cached)
		{
			PPUSim::VideoOutSignal rawIn{}, rgbOut{};

			// 8 Emphasis bands, each with 64 colors.

			for (size_t n = 0; n < (8 * 64); n++)
			{
				rawIn.RAW.raw = (uint16_t)n;
				ppu->ConvertRAWToRGB(rawIn, rgbOut);
				pal[n].r = rgbOut.RGB.RED;
				pal[n].g = rgbOut.RGB.GREEN;
				pal[n].b = rgbOut.RGB.BLUE;
			}

			pal_cached = true;
		}

		size_t n = raw & 0b111'11'1111;

		*r = pal[n].r;
		*g = pal[n].g;
		*b = pal[n].b;
	}

	void Board::SetRAWColorMode(bool enable)
	{
		ppu->SetRAWOutput(enable);
	}

	void Board::SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior)
	{
		ppu->SetOamDecayBehavior(behavior);
	}

	void Board::SetNoiseLevel(float volts)
	{
		ppu->SetCompositeNoise(volts);
	}

	void Board::PpuUseExternalDacLevels(bool use, PPUSim::DacLevels& tab)
	{
		ppu->UseExternalDacLevels(use, tab);
	}

	void Board::GetAllCoreDebugInfo(M6502Core::DebugInfo* info)
	{
		core->getDebug(info);
	}
}
