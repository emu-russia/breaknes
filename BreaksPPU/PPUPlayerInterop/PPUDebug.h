#pragma once

namespace PPUPlayer
{

	struct DebugInfoProvider
	{
		DebugInfoEntry* entry;
		uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
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
		/// <param name="opaque">Transparent pointer to pass to the delegate (usually `this`)</param>
		void AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry, uint8_t& bits), void* opaque);

		/// <summary>
		/// Delete all debugging information handlers
		/// </summary>
		void DisposeDebugInfo();

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
	};

}

extern PPUPlayer::DebugHub* dbg_hub;

void CreateDebugHub();

void DisposeDebugHub();
