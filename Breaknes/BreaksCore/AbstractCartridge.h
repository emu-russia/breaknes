// The abstract cartridge connector interface must include all the variety of signals, for all motherboard options.

#pragma once

namespace Breaknes
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
	/// A software descriptor of the current audio sample.
	/// </summary>
	union CartAudioOutSignal
	{
		struct NormalizedOut
		{
			float bogus;		// TBD
		} normalized;
	};

	class AbstractCartridge
	{
	protected:
		ConnectorType p1_type;

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
			APUSim::AudioOutSignal *snd_in,
			CartAudioOutSignal *snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty ) = 0;
	};
}
