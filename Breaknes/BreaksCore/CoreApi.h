// The top-level API for managed applications.

// TBD: The API will be extended to handle I/O devices (controllers, expansion ports)

#pragma once

extern "C"
{
	/// <summary>
	/// Creates a motherboard instance with all the hardware (APU/PPU/cartridge connector). The cartridge defaults to the Ejected state.
	/// </summary>
	/// <param name="boardName">Revision name of the motherboard. If the board is not supported, a `BogusBoard` will be created.</param>
	/// <param name="apu">APU revision</param>
	/// <param name="ppu">PPU revision</param>
	/// <param name="p1">The form factor of the cartridge connector.</param>
	__declspec(dllexport) void CreateBoard(char* boardName, char* apu, char* ppu, char* p1);

	/// <summary>
	/// Destroys the motherboard instance and all the resources it occupies.
	/// </summary>
	__declspec(dllexport) void DestroyBoard();

	/// <summary>
	/// Insert the cartridge. Cartridge Factory will try to fit .nes ROM into the form factor of the current motherboard's cartridge connector. In addition, a mapper instance will be created if it is supported.
	/// </summary>
	/// <param name="nesImage">.nes ROM image.</param>
	/// <param name="size">Image size (bytes)</param>
	__declspec(dllexport) int InsertCartridge(uint8_t* nesImage, size_t size);

	/// <summary>
	/// Free the cartridge connector from the ROM/mapper and all resources.
	/// </summary>
	__declspec(dllexport) void EjectCartridge();

	/// <summary>
	/// Simulate 1 half cycle of the board. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
	__declspec(dllexport) void Step();

	/// <summary>
	/// Make the board /RES pins = 0 for a few CLK half cycles so that the APU/PPU resets all of its internal circuits.
	/// </summary>
	__declspec(dllexport) void Reset();

	/// <summary>
	/// The parent application can check that the board is in the reset process and ignore the audio/video signal for that time.
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) bool InResetState();

	/// <summary>
	/// Get the values of the ACLK cycle counter.
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) size_t GetACLKCounter();

	/// <summary>
	/// Get the value of the 6502 core cycle counter (PHI Counter)
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) size_t GetPHICounter();

	/// <summary>
	/// Get the current resulting AUX value in normalized [0.0; 1.0] format.
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) void SampleAudioSignal(float* sample);

	/// <summary>
	/// Load the whole NSF data image to the NSFMapper device (NSFPlayerBoard only)
	/// </summary>
	/// <param name="data">nsf data offset +0x80</param>
	/// <param name="data_size">nsf data size</param>
	/// <param name="load_address">nsf load address (from header)</param>
	__declspec(dllexport) void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address);

	/// <summary>
	/// Enable the bank switching circuit for the NSFMapper device (NSFPlayerBoard only)
	/// </summary>
	/// <param name="enable"></param>
	__declspec(dllexport) void EnableNSFBanking(bool enable);

	/// <summary>
	/// Load APU/PPU registers dump (APUPlayer/PPUPlayer only)
	/// </summary>
	/// <param name="data">RegDumpEntry records</param>
	/// <param name="data_size">Dump size (bytes)</param>
	__declspec(dllexport) void LoadRegDump(uint8_t* data, size_t data_size);

	/// <summary>
	/// Get audio signal settings that help with its rendering on the consumer side.
	/// </summary>
	/// <param name="features"></param>
	__declspec(dllexport) void GetApuSignalFeatures(APUSim::AudioSignalFeatures* features);

	/// <summary>
	/// Get the "pixel" counter. Keep in mind that pixels refers to an abstract entity representing the visible or invisible part of the video signal.
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) size_t GetPCLKCounter();

	/// <summary>
	/// Get 1 sample of the video signal.
	/// </summary>
	/// <param name="sample"></param>
	__declspec(dllexport) void SampleVideoSignal(PPUSim::VideoOutSignal* sample);

	/// <summary>
	/// Get the direct value from the PPU H counter.
	/// </summary>
	/// <returns></returns>	
	__declspec(dllexport) size_t GetHCounter();

	/// <summary>
	/// Get the direct value from the PPU V counter.
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport) size_t GetVCounter();

	/// <summary>
	/// Forcibly enable rendering ($2001[3] = $2001[4] always equals 1). 
	/// Used for debugging PPU signals, when the CPU I/F register dump is limited, or when you want to get faster simulation results. 
	/// Keep in mind that with permanently enabled rendering the PPU becomes unstable and this hack should be applied when you know what you're doing.
	/// </summary>
	/// <param name="enable"></param>
	__declspec(dllexport) void RenderAlwaysEnabled(bool enable);

	/// <summary>
	/// Get video signal settings that help with its rendering on the consumer side.
	/// </summary>
	/// <param name="features"></param>
	__declspec(dllexport) void GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features);

	/// <summary>
	/// Convert the raw color to RGB. Can be used for palette generation or PPU video output in RAW mode.
	/// The SYNC level (RAW.Sync) check must be done from the outside.
	/// </summary>
	__declspec(dllexport) void ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b);

	/// <summary>
	/// Use RAW color output. 
	/// RAW color refers to the Chroma/Luma combination that comes to the video generator and the Emphasis bit combination.
	/// </summary>
	/// <param name="enable"></param>
	__declspec(dllexport) void SetRAWColorMode(bool enable);

	/// <summary>
	/// Set one of the ways to decay OAM cells.
	/// </summary>
	__declspec(dllexport) void SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior);

	/// <summary>
	/// Set the noise for the composite video signal (in volts).
	/// </summary>
	/// <param name="volts"></param>
	__declspec(dllexport) void SetNoiseLevel(float volts);

	/// <summary>
	/// Return all core debugging information for BreaksDebug.
	/// </summary>
	/// <param name="info"></param>
	__declspec(dllexport) void GetAllCoreDebugInfo(M6502Core::DebugInfo* info);
};
