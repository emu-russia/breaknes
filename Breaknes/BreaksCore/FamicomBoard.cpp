// https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/MB/Famicom.md

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	FamicomBoard::FamicomBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1) : Board(apu_rev, ppu_rev, p1)
	{
		// Big chips
		core = new M6502Core::M6502(true, true);
		apu = new APUSim::APU(core, apu_rev);
		ppu = new PPUSim::PPU(ppu_rev);

		// Memory
		wram = new BaseBoard::SRAM("WRAM", wram_bits);
		vram = new BaseBoard::SRAM("VRAM", vram_bits);

		apu->SetNormalizedOutput(true);
	}

	FamicomBoard::~FamicomBoard()
	{
		delete vram;
		delete wram;
		delete ppu;
		delete apu;
		delete core;
	}

	void FamicomBoard::Step()
	{
		// TBD: See if the bus is dirty and deal with it. In the NES/Famicom a dirty bus is a common thing.

		data_bus_dirty = false;
		ADDirty = false;

		// Throw in all the parts and see what's moving there. Don't forget pullups

		nRST = pendingReset ? TriState::Zero : TriState::One;

		Pullup(nIRQ);
		Pullup(nNMI);

		// APU (aka CPU)

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = nNMI;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = nIRQ;
		inputs[(size_t)APUSim::APU_Input::n_RES] = nRST;
		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;	// aka TST

		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);

		CPU_RnW = outputs[(size_t)APUSim::APU_Output::RnW];
		M2 = outputs[(size_t)APUSim::APU_Output::M2];

		// Accesses by the embedded core to APU registers are still broadcast to the address bus via the multiplexer.
		TreatCoreForRegdump(addr_bus, data_bus, apu->GetPHI2(), CPU_RnW);

		nOE1 = outputs[(size_t)APUSim::APU_Output::n_IN0];		// #RDP0 official
		nOE2 = outputs[(size_t)APUSim::APU_Output::n_IN1];		// #RDP1 official
		OUT_0 = outputs[(size_t)APUSim::APU_Output::OUT_0];
		OUT_1 = outputs[(size_t)APUSim::APU_Output::OUT_1];
		OUT_2 = outputs[(size_t)APUSim::APU_Output::OUT_0];

		// pullup (PPU_A[13]); -- wtf?
		// no pullup on R/W -- wtf?
		Pullup(CPU_RnW);

		// DMX (Bus Master)

		// In real CPU in reset mode M2 goes to `Z` state, it does not suit us
		Pullup(M2);			// HACK

		// The demultiplexer stages are mixed up in the Famicom. I'm not sure it makes sense to simulate it so accurately, but let it be

		DMX.sim(
			nY2[1],
			gnd,
			FromByte((addr_bus >> 13) & 1),
			FromByte((addr_bus >> 14) & 1),
			M2,
			FromByte((addr_bus >> 15) & 1),
			nY1, nY2);

		nROMSEL = nY2[3];
		WRAM_nCE = nY1[0];
		PPU_nCE = nY1[1];

		// PPU

		TriState ppu_inputs[(size_t)PPUSim::InputPad::Max]{};
		TriState ppu_outputs[(size_t)PPUSim::OutputPad::Max]{};

		ppu_inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		ppu_inputs[(size_t)PPUSim::InputPad::n_RES] = vdd;		// Famicom Board specific ⚠️
		ppu_inputs[(size_t)PPUSim::InputPad::RnW] = CPU_RnW;
		ppu_inputs[(size_t)PPUSim::InputPad::RS0] = FromByte((addr_bus >> 0) & 1);
		ppu_inputs[(size_t)PPUSim::InputPad::RS1] = FromByte((addr_bus >> 1) & 1);
		ppu_inputs[(size_t)PPUSim::InputPad::RS2] = FromByte((addr_bus >> 2) & 1);
		ppu_inputs[(size_t)PPUSim::InputPad::n_DBE] = PPU_nCE;

		ppu->sim(ppu_inputs, ppu_outputs, &ext_bus, &data_bus, &ad_bus, &pa8_13, vidSample);

		PPU_ALE = ppu_outputs[(size_t)PPUSim::OutputPad::ALE];
		PPU_nRD = ppu_outputs[(size_t)PPUSim::OutputPad::n_RD];
		PPU_nWR = ppu_outputs[(size_t)PPUSim::OutputPad::n_WR];
		nNMI = ppu_outputs[(size_t)PPUSim::OutputPad::n_INT];

		// Cartridge In

		bool LatchOutZ = false;
		PPUAddrLatch.sim(PPU_ALE, TriState::Zero, ad_bus, &LatchedAddr, LatchOutZ);

		ppu_addr = ((uint16_t)pa8_13 << 8) | LatchedAddr;
		PPU_nA13 = NOT(FromByte((ppu_addr >> 13) & 1));

		// Cartridge Port

		if (cart != nullptr)
		{
			TriState cart_in[(size_t)Mappers::CartInput::Max]{};
			TriState cart_out[(size_t)Mappers::CartOutput::Max];

			bool unused;

			cart_in[(size_t)Mappers::CartInput::nRD] = PPU_nRD;
			cart_in[(size_t)Mappers::CartInput::nWR] = PPU_nWR;
			cart_in[(size_t)Mappers::CartInput::nPA13] = PPU_nA13;
			cart_in[(size_t)Mappers::CartInput::M2] = M2;
			cart_in[(size_t)Mappers::CartInput::nROMSEL] = nROMSEL;
			cart_in[(size_t)Mappers::CartInput::RnW] = CPU_RnW;

			cart->sim(
				cart_in,
				cart_out,
				addr_bus & 0x7fff,			// A15 not connected
				&data_bus, data_bus_dirty,
				ppu_addr,
				&ad_bus, ADDirty,
				// Famicom Board specific ⚠️
				&cart_snd,
				nullptr, unused);

			// And here you can add some dirt on the contacts

			VRAM_nCE = cart_out[(size_t)Mappers::CartOutput::VRAM_nCS];
			VRAM_A10 = cart_out[(size_t)Mappers::CartOutput::VRAM_A10];
		}
		else
		{
			// No cartridge in the slot means 'z' on these signals.
			// Simulate this situation in the most painless way possible.

			VRAM_nCE = TriState::One;		// VRAM closed
			VRAM_A10 = TriState::Zero;
			nIRQ = TriState::Z;
		}

		// Memory

		VRAM_Addr = LatchedAddr;
		VRAM_Addr |= ((ppu_addr >> 8) & 1) << 8;
		VRAM_Addr |= ((ppu_addr >> 9) & 1) << 9;
		VRAM_Addr |= ((VRAM_A10 == TriState::One) ? 1 : 0) << 10;

		bool dz = (PPU_nRD == TriState::One && PPU_nWR == TriState::One);
		vram->sim(VRAM_nCE, PPU_nWR, PPU_nRD, &VRAM_Addr, &ad_bus, dz);

		WRAM_Addr = addr_bus & (wram_size - 1);
		wram->sim(WRAM_nCE, CPU_RnW, TriState::Zero, &WRAM_Addr, &data_bus, data_bus_dirty);

		// Tick

		CLK = NOT(CLK);

		if (pendingReset)
		{
			resetHalfClkCounter--;
			if (resetHalfClkCounter == 0)
			{
				pendingReset = false;
			}
		}
	}

	void FamicomBoard::Reset()
	{
		pendingReset = true;

		// See NESBoard for the additional info

		resetHalfClkCounter = 64;
	}

	bool FamicomBoard::InResetState()
	{
		return pendingReset;
	}

	void FamicomBoard::SampleAudioSignal(float* sample)
	{
		// We do everything the same as in the base class, but with the sound from the cartridge taken into account.

		if (sample != nullptr)
		{
			*sample = (aux.normalized.a * 0.4f /* 20k resistor */ + aux.normalized.b /* 12k resistor */ + cart_snd.normalized /* levels pls, someone? */) / 3.0f;
		}
	}

#pragma region "Fami IO"

	FamicomBoardIO::FamicomBoardIO() : IO::IOSubsystem()
	{
	}

	FamicomBoardIO::~FamicomBoardIO()
	{
	}

	int FamicomBoardIO::GetPorts()
	{
		// Only controller ports so far
		return 2;
	}

	void FamicomBoardIO::GetPortSupportedDevices(int port, std::list<IO::DeviceID>& devices)
	{
		devices.clear();

		switch (port)
		{
			case 0:
				devices.push_back(IO::DeviceID::FamiController_1);
				break;

			case 1:
				devices.push_back(IO::DeviceID::FamiController_2);
				break;

			default:
				break;
		}
	}

	void FamicomBoardIO::sim(int port, BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
		for (auto it = devices.begin(); it != devices.end(); ++it) {

			IO::IOMapped* mapped = *it;

			if (mapped->port == port && mapped->handle >= 0) {

				// TODO: Assign input signals to the simulated IO device

				mapped->device->sim(inputs, outputs);

				// TODO: Process the output signals from the device and distribute them across the board
			}
		}
	}

#pragma endregion "Fami IO"
}
