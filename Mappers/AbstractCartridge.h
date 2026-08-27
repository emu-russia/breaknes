// The abstract cartridge connector interface must include all the variety of signals, for all motherboard options.

#pragma once

#include "../Common/BaseLogicLib/BaseLogic.h"
#include "../CartPcb/CartPcbPort.h"

namespace Mappers
{
	// The cartridge edge-connector contract moved to CartPcb (issue #509);
	// these aliases keep the migration builds compiling.
	using ConnectorType = CartPcb::ConnectorType;
	using CartInput = CartPcb::CartInput;
	using CartOutput = CartPcb::CartOutput;
	using CartAudioOutSignal = CartPcb::CartAudioOutSignal;

	class AbstractCartridge
	{
	protected:
		ConnectorType p1_type;

		BaseLogic::TriState gnd = BaseLogic::TriState::Zero;
		BaseLogic::TriState vdd = BaseLogic::TriState::One;

	public:
		AbstractCartridge(ConnectorType _p1_type, uint8_t* nesImage, size_t size);
		virtual ~AbstractCartridge();

		virtual bool Valid();

		virtual void sim( 
			BaseLogic::TriState cart_in[(size_t)CartInput::Max],
			BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
			uint16_t cpu_addr,
			uint8_t* cpu_data, bool& cpu_data_dirty, 
			uint16_t ppu_addr,
			uint8_t* ppu_data, bool& ppu_data_dirty,
			// Famicom only
			CartAudioOutSignal *snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty ) = 0;

		/// <summary>
		/// Read one byte from the PRG address space of the cartridge without side effects.
		/// Used by the debugger and the Nintendulator log disassembler.
		/// </summary>
		/// <param name="cpu_addr">CPU bus address</param>
		/// <returns>The byte that would be read, or 0 for unmapped addresses.</returns>
		virtual uint8_t Dbg_ReadPRGByte(size_t cpu_addr) { return 0; }
	};
}
