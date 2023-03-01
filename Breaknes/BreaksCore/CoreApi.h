// The top-level API for managed applications.

// TBD: The API will be extended to handle I/O devices (controller, audio, video, etc.)

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

	// TBD: hmmm cartridge api...

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

	// TBD: Doxygen

	__declspec(dllexport) void Step();
	__declspec(dllexport) void Reset();
	__declspec(dllexport) bool InResetState();
	__declspec(dllexport) size_t GetACLKCounter();
	__declspec(dllexport) size_t GetPHICounter();
	__declspec(dllexport) void SampleAudioSignal(float* sample);
	__declspec(dllexport) void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address);
	__declspec(dllexport) void EnableNSFBanking(bool enable);
	__declspec(dllexport) void LoadRegDump(uint8_t* data, size_t data_size);
	__declspec(dllexport) void GetApuSignalFeatures(APUSim::AudioSignalFeatures* features);
	__declspec(dllexport) size_t GetPCLKCounter();
	__declspec(dllexport) void SampleVideoSignal(PPUSim::VideoOutSignal* sample);
	__declspec(dllexport) size_t GetHCounter();
	__declspec(dllexport) size_t GetVCounter();
	__declspec(dllexport) void RenderAlwaysEnabled(bool enable);
	__declspec(dllexport) void GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features);
	__declspec(dllexport) void ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b);
	__declspec(dllexport) void SetRAWColorMode(bool enable);
	__declspec(dllexport) void SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior);
	__declspec(dllexport) void SetNoiseLevel(float volts);
	__declspec(dllexport) void GetAllCoreDebugInfo(M6502Core::DebugInfo* info);
};
