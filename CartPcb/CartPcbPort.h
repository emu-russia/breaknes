// The cartridge edge-connector contract.
//
// This is the signal interface between the motherboard and the cartridge.
// It was defined by `Mappers::AbstractCartridge` before the Mappers component
// was retired (issue #509); it now lives in CartPcb. During the migration the
// `Mappers` namespace aliases these types (see Mappers/AbstractCartridge.h).

#pragma once

#include <cstdint>
#include <cstddef>

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
}
