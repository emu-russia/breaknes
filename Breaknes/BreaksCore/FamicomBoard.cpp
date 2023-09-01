// https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/MB/Famicom.md

// Instead of the signal designations /OE1,2 adopted in the nesdev.com community, we use the official names of these signals /RDP0,1

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

		io = new FamicomBoardIO(this);
	}

	FamicomBoard::~FamicomBoard()
	{
		delete io;
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

		nRDP0 = outputs[(size_t)APUSim::APU_Output::n_IN0];
		nRDP1 = outputs[(size_t)APUSim::APU_Output::n_IN1];
		OUT_0 = outputs[(size_t)APUSim::APU_Output::OUT_0];
		OUT_1 = outputs[(size_t)APUSim::APU_Output::OUT_1];
		OUT_2 = outputs[(size_t)APUSim::APU_Output::OUT_2];

		// IO

		if (io_enabled) {
			IOBinding();
		}

		// pullup (PPU_A[13]); -- wtf?
		// no pullup on R/W -- wtf?
		Pullup(CPU_RnW);

		// DMX (Bus Master)

		// In real CPU in reset mode M2 goes to `Z` state, it does not suit us
		// TODO: The pull-up is inside the DMX chip. Soon Famicom will come to disassemble, we will see what is in the chip and make LS139(DMX) properly
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

			CartridgeConnectorSimFailure1();

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

			CartridgeConnectorSimFailure2();

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

		// TODO: Mike
		// TODO: Expansion port sound

		if (sample != nullptr)
		{
			*sample = (
				aux.normalized.a * 0.4f /* 20k resistor */ + 
				aux.normalized.b /* 12k resistor */ + 
				cart_snd.normalized /* levels pls, someone? */) / 3.0f;
		}
	}

	void FamicomBoard::IOBinding()
	{
		// First you need to simulate 368s in the direction CPU->Ports

		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP0;
		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = vdd;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A1] = gnd;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A2] = gnd;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A3] = M2;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A4] = gnd;
		P4_IO.sim(p4_inputs, p4_outputs);

		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP1;
		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = vdd;	// not yet
		p5_inputs[(size_t)BaseBoard::LS368_Input::A1] = M2;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A2] = gnd;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A3] = gnd;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A4] = gnd;
		P5_IO.sim(p5_inputs, p5_outputs);

		// Call the IO subsystem and it will simulate the controllers and other I/O devices if they are connected

		io->sim(0);
		io->sim(1);

		// And now we need to simulate 368s in the direction Ports->CPU (don't forget the resistance matrix of the pull-ups (RM1)!)

		Pullup(p4016_d0);
		Pullup(p2_4016_data);
		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP0;
		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = vdd;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A1] = p4016_d0;	// d0
		p4_inputs[(size_t)BaseBoard::LS368_Input::A2] = p2_4016_data;	// d1
		p4_inputs[(size_t)BaseBoard::LS368_Input::A3] = M2;
		// TODO: It is not very clear how to convert the Mic signal level into a logical value, for now I will do it like this
		p4_inputs[(size_t)BaseBoard::LS368_Input::A4] = mic_level > 0.5f ? TriState::One : TriState::Zero;	// d2
		P4_IO.sim(p4_inputs, p4_outputs);
		SetDataBusIfNotFloating(0, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y1]);
		SetDataBusIfNotFloating(1, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y2]);
		SetDataBusIfNotFloating(2, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y4]);

		Pullup(p2_4017_data[0]);
		Pullup(p2_4017_data[1]);
		Pullup(p2_4017_data[2]);
		Pullup(p2_4017_data[3]);
		Pullup(p4017_d0);
		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP1;
		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = nRDP1;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A1] = M2;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A2] = p2_4017_data[0];	// d1
		p5_inputs[(size_t)BaseBoard::LS368_Input::A3] = p2_4017_data[2];	// d3
		p5_inputs[(size_t)BaseBoard::LS368_Input::A4] = p2_4017_data[3];	// d4
		p5_inputs[(size_t)BaseBoard::LS368_Input::A5] = p2_4017_data[1];	// d2
		p5_inputs[(size_t)BaseBoard::LS368_Input::A6] = p4017_d0;	// d0
		P5_IO.sim(p5_inputs, p5_outputs);
		SetDataBusIfNotFloating(0, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y6]);
		SetDataBusIfNotFloating(1, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y2]);
		SetDataBusIfNotFloating(2, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y5]);
		SetDataBusIfNotFloating(3, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y3]);
		SetDataBusIfNotFloating(4, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y4]);
	}

	void FamicomBoard::SetDataBusIfNotFloating(size_t n, BaseLogic::TriState val)
	{
		if (io_enabled && val != TriState::Z) {
			data_bus &= ~(1 << n);
			data_bus |= ToByte(val) << n;
		}
	}

	void FamicomBoard::CartridgeConnectorSimFailure1()
	{
		// And here you can add some dirt on the contacts
	}

	void FamicomBoard::CartridgeConnectorSimFailure2()
	{
		// And here you can add some dirt on the contacts
	}

#pragma region "Fami IO"

	FamicomBoardIO::FamicomBoardIO(FamicomBoard* board) : IO::IOSubsystem()
	{
		base = board;
	}

	FamicomBoardIO::~FamicomBoardIO()
	{
	}

	int FamicomBoardIO::GetPorts()
	{
		// Only controller ports so far
		return 2;
	}

	std::string FamicomBoardIO::GetPortName(int port)
	{
		switch (port)
		{
			case 0: return "Famicom Controller Port 1";
			case 1: return "Famicom Controller Port 2";
			default:
				break;
		}
		return "";
	}

	void FamicomBoardIO::GetPortSupportedDevices(int port, std::list<IO::DeviceID>& devices)
	{
		devices.clear();

		switch (port)
		{
			case 0:
				devices.push_back(IO::DeviceID::FamiController_1);
				devices.push_back(IO::DeviceID::VirtualFamiController_1);
				break;

			case 1:
				devices.push_back(IO::DeviceID::FamiController_2);
				devices.push_back(IO::DeviceID::VirtualFamiController_2);
				break;

			default:
				break;
		}
	}

	void FamicomBoardIO::sim(int port)
	{
		for (auto it = devices.begin(); it != devices.end(); ++it) {

			IO::IOMapped* mapped = *it;

			if (mapped->port == port && mapped->handle >= 0) {

				// Assign input signals to the simulated IO device

				TriState inputs[2]{};
				TriState outputs[1]{};

				if (port == 0) {

					inputs[0] = base->p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y3];
					Pullup(inputs[0]);	// RM1
					inputs[1] = base->OUT_0;
				}
				else if (port == 1) {

					inputs[0] = base->p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y1];
					Pullup(inputs[0]);	// RM1
					inputs[1] = base->OUT_0;
				}

				mapped->device->sim(inputs, outputs, &base->mic_level);

				// Process the output signals from the device and distribute them across the board

				if (port == 0) {

					base->p4016_d0 = outputs[0];
				}
				else if (port == 1) {

					base->p4017_d0 = outputs[0];
				}
			}
		}
	}

#pragma endregion "Fami IO"
}
