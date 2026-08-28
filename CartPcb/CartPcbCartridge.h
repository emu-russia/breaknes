// CartPcbCartridge - a cartridge that simulates a Pcb.
//
// Implements the cartridge-port contract (CartPcb::Cartridge) by forwarding the
// edge-connector signals into the Pcb.

#pragma once

#include "Pcb.h"
#include "CartImage.h"
#include "CartPcbPort.h"

namespace CartPcb
{
	class CartPcbCartridge : public Cartridge
	{
	public:
		CartPcbCartridge(ConnectorType p1, Pcb* pcb);
		virtual ~CartPcbCartridge();

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
			uint16_t* exp, bool& exp_dirty) override;

		uint8_t Dbg_ReadPRGByte(size_t cpu_addr) override;

		Pcb* GetPcb() { return pcb; }

	private:
		void AddCartMemDescriptors();

		Pcb* pcb;
		bool valid = false;
	};
}
