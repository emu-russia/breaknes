#pragma once

namespace Breaknes
{
	class Core
	{
		static uint8_t ReadTestMem(void* opaque, size_t addr);
		static void WriteTestMem(void* opaque, size_t addr, uint8_t data);
		static uint32_t GetTestInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);

		Board* this_board = nullptr;

	public:
		Core();
		~Core();

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
