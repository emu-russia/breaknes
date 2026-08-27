// The cartridge edge-connector contract.
//
// This is the signal interface between the motherboard and the cartridge.
// It was defined by `Mappers::AbstractCartridge` before the Mappers component
// was retired (issue #509); it now lives in CartPcb.

#pragma once

#include <cstdint>
#include <cstddef>

#include "../Common/BaseLogicLib/BaseLogic.h"

namespace CartPcb
{
	enum class ConnectorType
	{
		None = 0,
		FamicomStyle,
		NESStyle,
		Max,
	};

	enum class CartInput
	{
		SYSTEM_CLK,			// NES only
		M2,
		nROMSEL,
		RnW,
		nRD,
		nWR,
		nPA13,
		CIC_CLK,			// NES only
		CIC_TO_CART,		// NES only
		Max,
	};

	enum class CartOutput
	{
		VRAM_A10,
		VRAM_nCS,
		nIRQ,
		CIC_RST,			// NES only
		CIC_TO_MB,			// NES only
		Max,
	};

	/// <summary>
	/// A software descriptor of the current audio sample from cartridge port.
	/// Famicom only
	/// </summary>
	union CartAudioOutSignal
	{
		float normalized;
	};

	/// <summary>
	/// The abstract cartridge: the connector contract implemented by cartridges
	/// (the former `Mappers::AbstractCartridge`, moved here with the retirement
	/// of the Mappers component).
	/// </summary>
	class Cartridge
	{
	protected:
		ConnectorType p1_type;

		BaseLogic::TriState gnd = BaseLogic::TriState::Zero;
		BaseLogic::TriState vdd = BaseLogic::TriState::One;

	public:
		Cartridge(ConnectorType _p1_type);
		virtual ~Cartridge();

		virtual bool Valid();

		virtual void sim(
			BaseLogic::TriState cart_in[(size_t)CartInput::Max],
			BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
			uint16_t cpu_addr,
			uint8_t* cpu_data, bool& cpu_data_dirty,
			uint16_t ppu_addr,
			uint8_t* ppu_data, bool& ppu_data_dirty,
			// Famicom only
			CartAudioOutSignal* snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty) = 0;

		/// <summary>
		/// Read one byte from the PRG address space of the cartridge without side effects.
		/// Used by the debugger and the Nintendulator log disassembler.
		/// </summary>
		/// <param name="cpu_addr">CPU bus address</param>
		/// <returns>The byte that would be read, or 0 for unmapped addresses.</returns>
		virtual uint8_t Dbg_ReadPRGByte(size_t cpu_addr) { return 0; }
	};
}
