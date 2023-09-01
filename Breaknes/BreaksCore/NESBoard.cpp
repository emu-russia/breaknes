#include "pch.h"

// Instead of the signal designations /OE1,2 adopted in the nesdev.com community, we use the official names of these signals /RDP0,1

using namespace BaseLogic;

namespace Breaknes
{
	NESBoard::NESBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1) : Board (apu_rev, ppu_rev, p1)
	{
		// Big chips
		core = new M6502Core::M6502(true, true);
		apu = new APUSim::APU(core, apu_rev);
		ppu = new PPUSim::PPU(ppu_rev);

		// Memory
		wram = new BaseBoard::SRAM("WRAM", wram_bits);
		vram = new BaseBoard::SRAM("VRAM", vram_bits, false);

		apu->SetNormalizedOutput(true);

		io = new NESBoardIO(this);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}
	
	NESBoard::~NESBoard()
	{
		delete io;
		delete vram;
		delete wram;
		delete ppu;
		delete apu;
		delete core;
	}

	void NESBoard::Step()
	{
		// TBD: See if the bus is dirty and deal with it. In the NES/Famicom a dirty bus is a common thing.

		data_bus_dirty = false;
		ADDirty = false;

		// Throw in all the parts and see what's moving there. Don't forget pullups

		Pullup(nIRQ);
		Pullup(nNMI);

		// APU (aka CPU)

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		inputs[(size_t)APUSim::APU_Input::n_NMI] = nNMI;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = nIRQ;
		inputs[(size_t)APUSim::APU_Input::n_RES] = pendingReset_CPU ? TriState::Zero : TriState::One;
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
		OUT_2 = outputs[(size_t)APUSim::APU_Output::OUT_0];

		Pullup(nRDP0);
		Pullup(nRDP1);
		Pullup(OUT_0);

		// pullup (PPU_A[13]); -- wtf?
		// no pullup on R/W -- wtf?
		Pullup(CPU_RnW);

		// DMX (Bus Master)

		// In real CPU in reset mode M2 goes to `Z` state, it does not suit us
		// TODO: The pull-up is inside the DMX chip. Soon Famicom will come to disassemble, we will see what is in the chip and make LS139(DMX) properly
		Pullup(M2);			// HACK

		DMX.sim(
			gnd,
			nY1[1],
			M2,
			FromByte((addr_bus >> 15) & 1),
			FromByte((addr_bus >> 13) & 1),
			FromByte((addr_bus >> 14) & 1),
			nY1, nY2 );

		nROMSEL = nY1[3];
		WRAM_nCE = nY2[0];
		PPU_nCE = nY2[1];

		// PPU

		//DumpCpuIF();

		TriState ppu_inputs[(size_t)PPUSim::InputPad::Max]{};
		TriState ppu_outputs[(size_t)PPUSim::OutputPad::Max]{};

		ppu_inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		ppu_inputs[(size_t)PPUSim::InputPad::n_RES] = pendingReset_PPU ? TriState::Zero : TriState::One;;		// NES Board specific ⚠️
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
			cart_in[(size_t)Mappers::CartInput::SYSTEM_CLK] = CLK;		// NES Board specific ⚠️

			CartridgeConnectorSimFailure1();

			cart->sim(
				cart_in,
				cart_out,
				addr_bus & 0x7fff,			// A15 not connected
				&data_bus, data_bus_dirty,
				ppu_addr,
				&ad_bus, ADDirty,
				nullptr,
				// NES Board specific ⚠️
				&exp_bus, unused);

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

		// IO

		IOBinding();

		// Tick

		CLK = NOT(CLK);

		if (pendingReset_CPU)
		{
			resetHalfClkCounter_CPU--;
			if (resetHalfClkCounter_CPU == 0)
			{
				pendingReset_CPU = false;
			}
		}

		if (pendingReset_PPU)
		{
			resetHalfClkCounter_PPU--;
			if (resetHalfClkCounter_PPU == 0)
			{
				pendingReset_PPU = false;
			}
		}
	}

	void NESBoard::Reset()
	{
		pendingReset_CPU = true;
		pendingReset_PPU = true;

		// By setting the reset time you can adjust the "CPU/PPU Alignment" phenomenon.
		// The real board has a capacitor that controls the reset and also the CIC interferes with it, but we simplify all this.
		// Discussion here: https://forums.nesdev.org/viewtopic.php?t=19792   (nice phenomenon btw)

		resetHalfClkCounter_CPU = 6400000;
		resetHalfClkCounter_PPU = 64;
	}

	bool NESBoard::InResetState()
	{
		return pendingReset_PPU;
	}

	void NESBoard::DumpCpuIF()
	{
		if (PPU_nCE == TriState::Zero && CPU_RnW == TriState::Zero) {
			int reg_sel = addr_bus & 7;
			printf("Write PPU %d=0x%02X, phi2: %d, phi counter: 0x%llx\n", reg_sel, data_bus, apu->GetPHI2(), GetPHICounter());
		}
	}

	void NESBoard::IOBinding()
	{
		// There is no need to simulate 368s on input, they only work in the Port->CPU direction

		// Call the IO subsystem and it will simulate the controllers and other I/O devices if they are connected

		io->sim(0);
		io->sim(1);

		// And now we need to simulate 368s in the direction Ports->CPU (don't forget the resistance array of the pull-ups!)

		Pullup(p4016_data[0]);
		Pullup(p4016_data[1]);
		Pullup(p4016_data[2]);
		Pullup(p4016_data[3]);
		Pullup(p4016_data[4]);
		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP0;
		p4_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = nRDP0;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A1] = p4016_data[0];
		p4_inputs[(size_t)BaseBoard::LS368_Input::A2] = p4016_data[1];
		p4_inputs[(size_t)BaseBoard::LS368_Input::A3] = gnd;
		p4_inputs[(size_t)BaseBoard::LS368_Input::A4] = p4016_data[2];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A5] = p4016_data[3];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A6] = p4016_data[4];
		P4_IO.sim(p4_inputs, p4_outputs);
		SetDataBusIfNotFloating(0, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y1]);
		SetDataBusIfNotFloating(1, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y2]);
		SetDataBusIfNotFloating(2, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y4]);
		SetDataBusIfNotFloating(3, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y5]);
		SetDataBusIfNotFloating(4, p4_outputs[(size_t)BaseBoard::LS368_Output::n_Y6]);

		Pullup(p4017_data[0]);
		Pullup(p4017_data[1]);
		Pullup(p4017_data[2]);
		Pullup(p4017_data[3]);
		Pullup(p4017_data[4]);
		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G1] = nRDP1;
		p5_inputs[(size_t)BaseBoard::LS368_Input::n_G2] = nRDP1;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A1] = p4017_data[0];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A2] = p4017_data[1];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A3] = gnd;
		p5_inputs[(size_t)BaseBoard::LS368_Input::A4] = p4017_data[2];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A5] = p4017_data[3];
		p5_inputs[(size_t)BaseBoard::LS368_Input::A6] = p4017_data[4];
		P5_IO.sim(p5_inputs, p5_outputs);
		SetDataBusIfNotFloating(0, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y1]);
		SetDataBusIfNotFloating(1, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y2]);
		SetDataBusIfNotFloating(2, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y4]);
		SetDataBusIfNotFloating(3, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y5]);
		SetDataBusIfNotFloating(4, p5_outputs[(size_t)BaseBoard::LS368_Output::n_Y6]);
	}

	void NESBoard::SetDataBusIfNotFloating(size_t n, BaseLogic::TriState val)
	{
		if (io_enabled && val != TriState::Z) {
			data_bus &= ~(1 << n);
			data_bus |= ToByte(val) << n;
		}
	}

	void NESBoard::CartridgeConnectorSimFailure1()
	{
		// And here you can add some dirt on the contacts
	}

	void NESBoard::CartridgeConnectorSimFailure2()
	{
		// And here you can add some dirt on the contacts
	}

#pragma region "NES IO"

	NESBoardIO::NESBoardIO(NESBoard* board) : IO::IOSubsystem()
	{
		base = board;
	}

	NESBoardIO::~NESBoardIO()
	{
	}

	int NESBoardIO::GetPorts()
	{
		// Only controller ports so far
		return 2;
	}

	std::string NESBoardIO::GetPortName(int port)
	{
		switch (port)
		{
			case 0: return "NES Controller Port 1";
			case 1: return "NES Controller Port 2";
			default:
				break;
		}
		return "";
	}

	void NESBoardIO::GetPortSupportedDevices(int port, std::list<IO::DeviceID>& devices)
	{
		devices.clear();

		switch (port)
		{
			case 0:
				devices.push_back(IO::DeviceID::NESController);
				devices.push_back(IO::DeviceID::VirtualNESController);
				break;

			case 1:
				devices.push_back(IO::DeviceID::NESController);
				devices.push_back(IO::DeviceID::VirtualNESController);
				break;

			default:
				break;
		}
	}

	void NESBoardIO::sim(int port)
	{
		for (auto it = devices.begin(); it != devices.end(); ++it) {

			IO::IOMapped* mapped = *it;

			if (mapped->port == port && mapped->handle >= 0) {

				// Assign input signals to the simulated IO device

				TriState inputs[2]{};
				TriState outputs[3]{};

				if (port == 0) {

					inputs[0] = base->nRDP0;
					inputs[1] = base->OUT_0;
				}
				else if (port == 1) {

					inputs[0] = base->nRDP1;
					inputs[1] = base->OUT_0;
				}

				mapped->device->sim(inputs, outputs, nullptr);

				// Process the output signals from the device and distribute them across the board

				if (port == 0) {

					base->p4016_data[0] = outputs[0];
					base->p4016_data[3] = outputs[2];
					base->p4016_data[4] = outputs[1];
				}
				else if (port == 1) {

					base->p4017_data[0] = outputs[0];
					base->p4017_data[3] = outputs[2];
					base->p4017_data[4] = outputs[1];
				}
			}
		}
	}

#pragma endregion "NES IO"
}
