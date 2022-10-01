// A special type of memory with bank switching logic inside.

#pragma once

namespace NSFPlayer
{
	class BankedSRAM
	{
	public:
		BankedSRAM();
		~BankedSRAM();

		size_t Dbg_GetSize();
		uint8_t Dbg_ReadByte(size_t addr);
		void Dbg_WriteByte(size_t addr, uint8_t data);
	};
}
