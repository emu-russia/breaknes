// Simple Mapper Simulator (7) - AOROM.

#pragma once

namespace Mappers
{
	class AOROM : public Breaknes::AbstractCartridge
	{
		bool valid = false;

		uint8_t* PRG = nullptr;
		size_t PRGSize = 0;

		uint8_t* CHR = nullptr;
		size_t CHRSize = 0;

		static uint8_t Dbg_ReadCHRByte(void* opaque, size_t addr);
		static void Dbg_WriteCHRByte(void* opaque, size_t addr, uint8_t data);

		void AddCartMemDescriptors();

		BaseBoard::LS161 counter{};

	public:
		AOROM(Breaknes::ConnectorType p1, uint8_t* nesImage, size_t nesImageSize);
		virtual ~AOROM();

		bool Valid() override;

		void sim(
			BaseLogic::TriState cart_in[(size_t)Breaknes::CartInput::Max],
			BaseLogic::TriState cart_out[(size_t)Breaknes::CartOutput::Max],
			uint16_t cpu_addr,
			uint8_t* cpu_data, bool& cpu_data_dirty,
			uint16_t ppu_addr,
			uint8_t* ppu_data, bool& ppu_data_dirty,
			// Famicom only
			APUSim::AudioOutSignal* snd_in,
			Breaknes::CartAudioOutSignal* snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty);
	};
}
