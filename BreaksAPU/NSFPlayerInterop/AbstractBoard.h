// Managed NFSPlayer application can work with two types of "boards": NSFPlayerBoard (to simulate .nsf) and APUPlayerBoard (to play APU registers dump)

#pragma once

namespace NSFPlayer
{
	class Board
	{
	protected:

		APUSim::APU* apu = nullptr;
		M6502Core::M6502* core = nullptr;

		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;

		APUSim::AudioOutSignal aux{};

	public:
		Board(char* boardName, char* apu, char* ppu, char* p1) {}
		virtual ~Board() {}

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
		virtual int InsertCartridge(uint8_t* nesImage, size_t nesImageSize) = 0;

		/// <summary>
		/// Remove the cartridge. Logically this means that all terminals associated with the cartridge take the value of `z`.
		/// </summary>
		virtual void EjectCartridge() = 0;

		/// <summary>
		/// Make the APU /RES pin = 0 for a few CLK half cycles so that the APU resets all of its internal circuits.
		/// </summary>
		virtual void ResetAPU(uint16_t addr, bool reset_apu_also) = 0;

		/// <summary>
		/// The parent application can check that the APU is in the reset process and ignore the audio signal for that time.
		/// </summary>
		/// <returns></returns>
		virtual bool APUInResetState() = 0;

		virtual size_t GetACLKCounter() = 0;

		virtual size_t GetPHICounter() = 0;

		virtual void SampleAudioSignal(float* sample) = 0;

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
		/// Load APU registers dump
		/// </summary>
		/// <param name="data">APULogEntry records</param>
		/// <param name="data_size">Dump size (bytes)</param>
		virtual void LoadRegDump(uint8_t* data, size_t data_size) = 0;

		/// <summary>
		/// Get audio signal settings that help with its rendering on the consumer side.
		/// </summary>
		/// <param name="features"></param>
		virtual void GetSignalFeatures(APUSim::AudioSignalFeatures* features) = 0;
	};
}