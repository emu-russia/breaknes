#pragma once

namespace Breaknes
{
	class Board
	{
	protected:

		// These basic chips are on all variations of motherboards, so we make them available to all inherited classes.

		M6502Core::M6502* cpu = nullptr;
		APUSim::APU* apu = nullptr;
		PPUSim::PPU* ppu = nullptr;

		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;

		APUSim::AudioOutSignal aux{};

		// The cartridge slot supports hotplugging during simulation.

		AbstractCartridge* cart = nullptr;

	public:
		Board(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) {}
		virtual ~Board() {}

		// hmm...
		void InsertCartridge(AbstractCartridge* cart);
		void DestroyCartridge();

		/// <summary>
		/// Simulate 1 half cycle of the test board with NSFPlayer. The simulation of the signal edge is not supported, this is overkill.
		/// </summary>
		virtual void Step() = 0;

		/// <summary>
		/// "Insert" the cartridge as a .nes ROM. In this implementation we are simply trying to instantiate an NROM, but in a more advanced emulation, Cartridge Factory will take care of "inserting" the cartridge.
		/// </summary>
		/// <param name="nesImage">A pointer to the ROM image in memory.</param>
		/// <param name="nesImageSize">ROM image size in bytes.</param>
		/// <returns></returns>
		virtual int InsertCartridge(uint8_t* nesImage, size_t nesImageSize);

		/// <summary>
		/// Remove the cartridge. Logically this means that all terminals associated with the cartridge take the value of `z`.
		/// </summary>
		virtual void EjectCartridge();

		/// <summary>
		/// Make the board /RES pins = 0 for a few CLK half cycles so that the APU/PPU resets all of its internal circuits.
		/// </summary>
		virtual void Reset() = 0;

		/// <summary>
		/// The parent application can check that the board is in the reset process and ignore the audio/video signal for that time.
		/// </summary>
		/// <returns></returns>
		virtual bool InResetState() = 0;

		/// <summary>
		/// Get the values of the ACLK cycle counter.
		/// </summary>
		/// <returns></returns>
		virtual size_t GetACLKCounter();

		/// <summary>
		/// Get the value of the 6502 core cycle counter (PHI Counter)
		/// </summary>
		/// <returns></returns>
		virtual size_t GetPHICounter();

		/// <summary>
		/// Get the current resulting AUX value in normalized [0.0; 1.0] format.
		/// </summary>
		/// <returns></returns>
		virtual void SampleAudioSignal(float* sample);

		/// <summary>
		/// Load the whole NSF data image to the BankedSRAM device.
		/// </summary>
		/// <param name="data">nsf data offset +0x80</param>
		/// <param name="data_size">nsf data size</param>
		/// <param name="load_address">nsf load address (from header)</param>
		virtual void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address) = 0;

		/// <summary>
		/// Enable the bank switching circuit for the BankedSRAM device.
		/// </summary>
		/// <param name="enable"></param>
		virtual void EnableNSFBanking(bool enable) = 0;

		/// <summary>
		/// Load APU/PPU registers dump
		/// </summary>
		/// <param name="data">RegDumpEntry records</param>
		/// <param name="data_size">Dump size (bytes)</param>
		virtual void LoadRegDump(uint8_t* data, size_t data_size) = 0;

		/// <summary>
		/// Get audio signal settings that help with its rendering on the consumer side.
		/// </summary>
		/// <param name="features"></param>
		virtual void GetApuSignalFeatures(APUSim::AudioSignalFeatures* features);
	};
}
