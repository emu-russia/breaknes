#pragma once

namespace Breaknes
{
	struct DebugInfoProvider
	{
		DebugInfoEntry* entry;
		uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry);
		void* opaque;
	};

	struct MemProvider
	{
		MemDesciptor* descr;
		uint8_t(*ReadByte)(void* opaque, size_t addr);
		void* opaque;
		bool cartRelated;
	};

	class Core
	{
		static uint8_t ReadTestMem(void* opaque, size_t addr);
		static uint32_t GetTestInfo(void* opaque, DebugInfoEntry* entry);

		Board* this_board = nullptr;

	public:
		// TBD: Make std::map[DebugInfoType] of std::list?
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

		Core();
		~Core();

		/// <summary>
		/// Add a handler for getting debugging information
		/// </summary>
		/// <param name="type">Type of debugging information (corresponds to single PropertyGrid)</param>
		/// <param name="entry">One entry of debugging information. Corresponds to a single signal/register/whatever. (auto-delete)</param>
		/// <param name="GetValue">Delegate to get the value</param>
		/// <param name="opaque">Transparent pointer to pass to the delegate (usually `this`)</param>
		void AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry), void* opaque);

		/// <summary>
		/// Delete all debugging information handlers
		/// </summary>
		void DisposeDebugInfo();

		/// <summary>
		/// Add a memory region access handler.
		/// </summary>
		/// <param name="descr">Memory descriptor (auto-delete)</param>
		/// <param name="ReadByte">Delegate to read one byte</param>
		/// <param name="opaque">Transparent pointer to pass to the delegate (usually `this`)</param>
		/// <param name="cartRelated">true: The memory region refers to the external cartridge.</param>
		void AddMemRegion(MemDesciptor* descr, uint8_t (*ReadByte)(void* opaque, size_t addr), void* opaque, bool cartRelated);

		/// <summary>
		/// Clear MemLayout.
		/// </summary>
		void DisposeMemMap();

		/// <summary>
		/// Switch the simulator to the specified board instance.
		/// </summary>
		/// <param name="board"></param>
		void SwitchBoard(Board* board);

		/// <summary>
		/// Destroy all resources and current board
		/// </summary>
		void DisposeBoard();

		/// <summary>
		/// Insert the cartridge. An abstract model is used as a cartridge, which spreads out into many mapper implementations.
		/// </summary>
		/// <param name="cart"></param>
		void AttachCartridge(AbstractCartridge* cart);

		/// <summary>
		/// Remove the cartridge and release any associated resources.
		/// </summary>
		void DisposeCartridge();

		/// <summary>
		/// Perform a simulation of one Half-cycle of the current board.
		/// </summary>
		void Sim();
	};
}
