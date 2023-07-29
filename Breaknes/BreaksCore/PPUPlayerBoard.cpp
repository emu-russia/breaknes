// A special type of breadboard that contains only PPU. Instead of CPU, a RegDump processor is used, which feeds register operations to the PPU from the dump via CPU I/F.
// Module for maintaining a simulated PPU environment.

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	PPUPlayerBoard::PPUPlayerBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1) : Board(apu_rev, ppu_rev, p1)
	{
		core = new M6502Core::FakeM6502("PPU", MappedPPUBase, MappedAPUMask);
		ppu = new PPUSim::PPU(ppu_rev);
		vram = new BaseBoard::SRAM("VRAM", vram_bits);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	PPUPlayerBoard::~PPUPlayerBoard()
	{
		delete ppu;
		delete vram;
		delete core;
	}

	void PPUPlayerBoard::SimCoreDivider()
	{
		TriState n_CLK = NOT(CLK);

		// Phase splitter

		CLK_FF.set(NOR(NOR(NOT(n_CLK), CLK_FF.get()), NOT(NOT(n_CLK))));
		TriState q = CLK_FF.get();
		TriState nq = CLK_FF.nget();

		// Jhonson counter (back propagate)

		div[5].sim(q, nq, TriState::Zero, div[4].get_sout(TriState::Zero));
		div[4].sim(q, nq, TriState::Zero, div[3].get_sout(TriState::Zero));

		TriState new_phi = NOT(div[5].get_sout(TriState::Zero));
		PHI0 = new_phi;
		TriState rst = NOR(PHI0, div[4].get_sout(TriState::Zero));

		div[3].sim(q, nq, rst, div[2].get_sout(rst));
		div[2].sim(q, nq, rst, div[1].get_sout(rst));
		div[1].sim(q, nq, rst, div[0].get_sout(rst));
		div[0].sim(q, nq, rst, PHI0);
	}

	void DIV_SRBit::sim(TriState q, TriState nq, TriState rst, TriState sin)
	{
		in_latch.set(sin, q);
		out_latch.set(in_latch.nget(), nq);
	}

	TriState DIV_SRBit::get_sout(TriState rst)
	{
		return NOR(out_latch.get(), rst);
	}

	TriState DIV_SRBit::get_nval()
	{
		return in_latch.nget();
	}

	/// <summary>
	/// Simulate 1 half cycle of the test board with PPUPlayer. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
	void PPUPlayerBoard::Step()
	{
		ADDirty = false;
		ext_bus = 0;
		addr_bus = 0;

		// Simulate Divider for Core

		SimCoreDivider();

		// Simulate Core

		TriState core_inputs[(size_t)M6502Core::InputPad::Max]{};
		TriState core_outputs[(size_t)M6502Core::OutputPad::Max];

		core_inputs[(size_t)M6502Core::InputPad::n_NMI] = TriState::One;
		core_inputs[(size_t)M6502Core::InputPad::n_IRQ] = TriState::One;
		core_inputs[(size_t)M6502Core::InputPad::n_RES] = pendingReset ? TriState::Zero : TriState::One;
		core_inputs[(size_t)M6502Core::InputPad::PHI0] = PHI0;
		core_inputs[(size_t)M6502Core::InputPad::RDY] = TriState::One;
		core_inputs[(size_t)M6502Core::InputPad::SO] = TriState::One;

		core->sim(core_inputs, core_outputs, &addr_bus, &data_bus);

		TriState Core_PHI1 = core_outputs[(size_t)M6502Core::OutputPad::PHI1];
		TriState Core_PHI2 = core_outputs[(size_t)M6502Core::OutputPad::PHI2];
		TriState Core_RnW = core_outputs[(size_t)M6502Core::OutputPad::RnW];
		TriState Core_SYNC = core_outputs[(size_t)M6502Core::OutputPad::SYNC];

		bool pendingWrite = Core_RnW == TriState::Zero;
		bool pendingCpuOperation = (addr_bus & ~MappedPPUMask) == MappedPPUBase;
		size_t ppuRegId = addr_bus & 7;

		// CPU I/F operations counter (negedge)

		if (!pendingCpuOperation && prev_pendingCpuOperation)
		{
			CPUOpsProcessed++;
		}
		prev_pendingCpuOperation = pendingCpuOperation;

		// Simulate PPU

		TriState ppu_inputs[(size_t)PPUSim::InputPad::Max]{};
		TriState ppu_outputs[(size_t)PPUSim::OutputPad::Max]{};

		ppu_inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		ppu_inputs[(size_t)PPUSim::InputPad::n_RES] = pendingReset ? TriState::Zero : TriState::One;
		ppu_inputs[(size_t)PPUSim::InputPad::RnW] = pendingWrite ? TriState::Zero : TriState::One;
		ppu_inputs[(size_t)PPUSim::InputPad::RS0] = pendingCpuOperation ? ((ppuRegId & 1) ? TriState::One : TriState::Zero) : TriState::Zero;
		ppu_inputs[(size_t)PPUSim::InputPad::RS1] = pendingCpuOperation ? ((ppuRegId & 2) ? TriState::One : TriState::Zero) : TriState::Zero;
		ppu_inputs[(size_t)PPUSim::InputPad::RS2] = pendingCpuOperation ? ((ppuRegId & 4) ? TriState::One : TriState::Zero) : TriState::Zero;
		ppu_inputs[(size_t)PPUSim::InputPad::n_DBE] = pendingCpuOperation ? TriState::Zero : TriState::One;

		ppu->sim(ppu_inputs, ppu_outputs, &ext_bus, &data_bus, &ad_bus, &pa8_13, vidSample);

		ALE = ppu_outputs[(size_t)PPUSim::OutputPad::ALE];
		n_RD = ppu_outputs[(size_t)PPUSim::OutputPad::n_RD];
		n_WR = ppu_outputs[(size_t)PPUSim::OutputPad::n_WR];
		n_INT = ppu_outputs[(size_t)PPUSim::OutputPad::n_INT];

		Pullup(n_INT);

		// Simulate all other surrounding logic and cartridge

		bool LatchOutZ = false;
		latch.sim(ALE, TriState::Zero, ad_bus, &LatchedAddress, LatchOutZ);

		ppu_addr = ((uint16_t)pa8_13 << 8) | LatchedAddress;
		n_PA13 = NOT(FromByte((ppu_addr >> 13) & 1));

		if (cart != nullptr)
		{
			TriState cart_in[(size_t)Mappers::CartInput::Max]{};
			TriState cart_out[(size_t)Mappers::CartOutput::Max];

			bool unused;

			cart_in[(size_t)Mappers::CartInput::nRD] = n_RD;
			cart_in[(size_t)Mappers::CartInput::nWR] = n_WR;
			cart_in[(size_t)Mappers::CartInput::nPA13] = n_PA13;
			cart_in[(size_t)Mappers::CartInput::nROMSEL] = TriState::One;

			cart->sim (
				cart_in,
				cart_out,
				0,
				nullptr, unused,
				ppu_addr,
				&ad_bus, ADDirty,
				nullptr,
				nullptr, unused );

			n_VRAM_CS = cart_out[(size_t)Mappers::CartOutput::VRAM_nCS];
			VRAM_A10 = cart_out[(size_t)Mappers::CartOutput::VRAM_A10];
		}
		else
		{
			// No cartridge in the slot means 'z' on these signals.
			// Simulate this situation in the most painless way possible.

			n_VRAM_CS = TriState::One;		// VRAM closed
			VRAM_A10 = TriState::Zero;
		}

		VRAM_Addr = LatchedAddress;
		VRAM_Addr |= ((ppu_addr >> 8) & 1) << 8;
		VRAM_Addr |= ((ppu_addr >> 9) & 1) << 9;
		VRAM_Addr |= ((VRAM_A10 == TriState::One) ? 1 : 0) << 10;

		bool dz = (n_RD == TriState::One && n_WR == TriState::One);
		vram->sim(n_VRAM_CS, n_WR, n_RD, &VRAM_Addr, &ad_bus, dz);

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

	/// <summary>
	/// Make the PPU /RES pin = 0 for a few CLK half cycles so that the PPU resets all of its internal circuits.
	/// </summary>
	void PPUPlayerBoard::Reset()
	{
		pendingReset = true;
		resetHalfClkCounter = 64;
	}

	/// <summary>
	/// The parent application can check that the PPU is in the reset process and ignore the video signal for that time.
	/// </summary>
	/// <returns></returns>
	bool PPUPlayerBoard::InResetState()
	{
		return pendingReset;
	}

	void PPUPlayerBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{
		if (core != nullptr)
		{
			CPUOpsProcessed = 0;
			prev_pendingCpuOperation = false;
			core->SetRegDump(data, data_size);
		}
	}
}
