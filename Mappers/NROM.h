// Simple Mapper Simulator (0) - NROM.

#pragma once

namespace Mappers
{
	struct NROM_DebugInfo
	{
		uint32_t last_PA;
		uint32_t last_nRD;
		uint32_t last_nWR;
	};

	class NROM : public AbstractCartridge
	{
		bool valid = false;

		uint8_t* PRG = nullptr;
		size_t PRGSize = 0;

		uint8_t* CHR = nullptr;
		size_t CHRSize = 0;

		bool chr_ram = false;

		NROM_DebugInfo nrom_debug{};

		// Connect to PPU A10 for vertical mirroring or PPU A11 for horizontal mirroring.
		bool V_Mirroring = false;

		static uint8_t Dbg_ReadCHRByte(void* opaque, size_t addr);
		static void Dbg_WriteCHRByte(void* opaque, size_t addr, uint8_t data);
		void GetDebugInfo(NROM_DebugInfo& info);
		
		static uint32_t GetCartDebugInfo(void* opaque, DebugInfoEntry* entry);
		static void SetCartDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddCartMemDescriptors();
		void AddCartDebugInfoProviders();

	public:
		NROM(ConnectorType p1, uint8_t* nesImage, size_t nesImageSize);
		virtual ~NROM();

		bool Valid() override;

		void sim(
			BaseLogic::TriState cart_in[(size_t)CartInput::Max],
			BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
			uint16_t cpu_addr,
			uint8_t* cpu_data, bool& cpu_data_dirty,
			uint16_t ppu_addr,
			uint8_t* ppu_data, bool& ppu_data_dirty,
			// Famicom only
			CartAudioOutSignal* snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty);
	};
}
