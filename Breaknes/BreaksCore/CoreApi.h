// The top-level API for managed applications.

// TBD: The API will be extended to handle I/O devices (controller, audio, video, etc.)

#pragma once

/// <summary>
/// Types of debugging information to get the internal state of the emulated system.
/// </summary>
enum DebugInfoType
{
	DebugInfoType_Unknown = 0,
	DebugInfoType_Test,
	DebugInfoType_Core,
	DebugInfoType_CoreRegs,
	DebugInfoType_APU,
	DebugInfoType_APURegs,
	DebugInfoType_PPU,
	DebugInfoType_PPURegs,
	DebugInfoType_Board,
	DebugInfoType_Cart,
};

#pragma pack(push, 1)
/// <summary>
/// One entry of debugging information. On the managed application side, a PropertyGrid is created based on the debug entries.
/// </summary>
struct DebugInfoEntry
{
	char category[32];
	char name[32];
	uint32_t value;
};
#pragma pack(pop)

#pragma pack(push, 1)
/// <summary>
/// Memory block descriptor
/// </summary>
struct MemDesciptor
{
	char name[32];
	size_t size;
};
#pragma pack(pop)

extern "C"
{

	/// <summary>
	/// Creates a motherboard instance with all the hardware (APU/PPU/cartridge connector). The cartridge defaults to the Ejected state.
	/// </summary>
	/// <param name="boardName">Revision name of the motherboard. If the board is not supported, a `BogusBoard` will be created.</param>
	/// <param name="apu">APU revision</param>
	/// <param name="ppu">PPU revision</param>
	/// <param name="p1">The form factor of the cartridge connector.</param>
	__declspec(dllexport)
	void CreateBoard(char* boardName, char* apu, char* ppu, char* p1);

	/// <summary>
	/// Destroys the motherboard instance and all the resources it occupies.
	/// </summary>
	__declspec(dllexport)
	void DestroyBoard();

	/// <summary>
	/// Insert the cartridge. Cartridge Factory will try to fit .nes ROM into the form factor of the current motherboard's cartridge connector. In addition, a mapper instance will be created if it is supported.
	/// </summary>
	/// <param name="nesImage">.nes ROM image.</param>
	/// <param name="size">Image size (bytes)</param>
	__declspec(dllexport)
	void InsertCartridge(uint8_t* nesImage, size_t size);

	/// <summary>
	/// Free the cartridge connector from the ROM/mapper and all resources.
	/// </summary>
	__declspec(dllexport)
	void EjectCartridge();

	/// <summary>
	/// Simulate the specified number of half-cycles.
	/// </summary>
	/// <param name="numHalfCycles"></param>
	__declspec(dllexport)
	void Sim(size_t numHalfCycles);

	/// <summary>
	/// Get the number of entries for the specified debugging information type.
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	__declspec(dllexport)
	size_t GetDebugInfoEntryCount(DebugInfoType type);

	/// <summary>
	/// Get debug entries of the specified type.
	/// </summary>
	/// <param name="type"></param>
	/// <param name="entries"></param>
	__declspec(dllexport)
	void GetDebugInfo(DebugInfoType type, DebugInfoEntry* entries);

	/// <summary>
	/// Get the number of memory descriptors that are registered in the core
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport)
	size_t GetMemLayout();

	/// <summary>
	/// Get information about the memory block
	/// </summary>
	/// <param name="descrID"></param>
	/// <param name="descr"></param>
	__declspec(dllexport)
	void GetMemDescriptor(size_t descrID, MemDesciptor* descr);

	/// <summary>
	/// Dump the whole memory block. We are emulating NES here, so the dump sizes will be small and there is no point in dumping in parts.
	/// </summary>
	/// <param name="descrID"></param>
	/// <param name="ptr"></param>
	__declspec(dllexport)
	void DumpMem(size_t descrID, uint8_t* ptr);

};
