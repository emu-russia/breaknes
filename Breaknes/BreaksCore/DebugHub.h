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
	uint8_t bits;			// The number of significant bits in the value
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
	int32_t size;
};
#pragma pack(pop)

struct DebugInfoProvider
{
	DebugInfoEntry* entry;
	uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry);
	void (*SetValue)(void* opaque, DebugInfoEntry* entry, uint32_t value);
	void* opaque;
};

struct MemProvider
{
	MemDesciptor* descr;
	uint8_t(*ReadByte)(void* opaque, size_t addr);
	void(*WriteByte)(void* opaque, size_t addr, uint8_t data);
	void* opaque;
	bool cartRelated;
};

class DebugHub
{
public:

	std::list<DebugInfoProvider> testInfo;
	std::list<DebugInfoProvider> coreInfo;
	std::list<DebugInfoProvider> coreRegsInfo;
	std::list<DebugInfoProvider> apuInfo;
	std::list<DebugInfoProvider> apuRegsInfo;
	std::list<DebugInfoProvider> ppuInfo;
	std::list<DebugInfoProvider> ppuRegsInfo;
	std::list<DebugInfoProvider> boardInfo;
	std::list<DebugInfoProvider> cartInfo;

	std::list<MemProvider> memMap;

	DebugHub();
	~DebugHub();

	/// <summary>
	/// Add a handler for getting debugging information
	/// </summary>
	/// <param name="type">Type of debugging information (corresponds to single PropertyGrid)</param>
	/// <param name="entry">One entry of debugging information. Corresponds to a single signal/register/whatever. (auto-delete)</param>
	/// <param name="GetValue">Delegate to get the value</param>
	/// <param name="SetValue">Delegate to set the value</param>
	/// <param name="opaque">Transparent pointer to pass to the delegate (usually `this`)</param>
	void AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, 
		uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry), 
		void (*SetValue)(void* opaque, DebugInfoEntry* entry, uint32_t value),
		void* opaque);

	/// <summary>
	/// Delete all debugging information handlers
	/// </summary>
	void DisposeDebugInfo();

	/// <summary>
	/// Clear the registered handlers for "Cart" type only.
	/// </summary>
	void DisposeCartDebugInfo();

	/// <summary>
	/// Add a memory region access handler.
	/// </summary>
	/// <param name="descr">Memory descriptor (auto-delete)</param>
	/// <param name="ReadByte">Delegate to read one byte</param>
	/// <param name="WriteByte">Delegate to write one byte</param>
	/// <param name="opaque">Transparent pointer to pass to the delegate (usually `this`)</param>
	/// <param name="cartRelated">true: The memory region refers to the external cartridge.</param>
	void AddMemRegion(MemDesciptor* descr, uint8_t (*ReadByte)(void* opaque, size_t addr), void(*WriteByte)(void* opaque, size_t addr, uint8_t data), void* opaque, bool cartRelated);

	/// <summary>
	/// Clear MemLayout.
	/// </summary>
	void DisposeMemMap();

	/// <summary>
	/// Clear cartRelated MemLayout only.
	/// </summary>
	void DisposeCartMemMap();
};

extern DebugHub* dbg_hub;

void CreateDebugHub();

void DisposeDebugHub();
