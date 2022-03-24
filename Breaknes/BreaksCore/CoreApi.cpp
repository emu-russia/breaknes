#include "pch.h"

extern "C"
{

	__declspec(dllexport)
	void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		Breaknes::BoardFactory bf (boardName, apu, ppu, p1);
		core.SwitchBoard(bf.CreateInstance(&core));
	}

	__declspec(dllexport)
	void DestroyBoard()
	{
		core.DisposeDebugInfo();
		core.DisposeMemMap();
		core.DisposeCartridge();
		core.DisposeBoard();
	}

	__declspec(dllexport)
	void InsertCartridge(uint8_t* nesImage, size_t size)
	{
		Breaknes::CartridgeFactory cf(nesImage, size);
		core.AttachCartridge(cf.GetInstance(&core));
	}

	__declspec(dllexport)
	void EjectCartridge()
	{
		core.DisposeCartridge();
	}

	__declspec(dllexport)
	void Sim(size_t numHalfCycles)
	{
		while (numHalfCycles--)
		{
			core.Sim();
		}
	}

	__declspec(dllexport)
	size_t GetDebugInfoEntryCount(DebugInfoType type)
	{
		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				return core.testInfo.size();

			case DebugInfoType::DebugInfoType_Core:
				return core.coreInfo.size();

			case DebugInfoType::DebugInfoType_CoreRegs:
				return core.coreRegsInfo.size();

			case DebugInfoType::DebugInfoType_APU:
				return core.apuInfo.size();

			case DebugInfoType::DebugInfoType_APURegs:
				return core.apuRegsInfo.size();

			case DebugInfoType::DebugInfoType_PPU:
				return core.ppuInfo.size();

			case DebugInfoType::DebugInfoType_PPURegs:
				return core.ppuRegsInfo.size();

			case DebugInfoType::DebugInfoType_Board:
				return core.boardInfo.size();

			case DebugInfoType::DebugInfoType_Cart:
				return core.cartInfo.size();
		}

		return 0;
	}

	void CopyOutDebugInfo(std::list<Breaknes::DebugInfoProvider>& coll, DebugInfoEntry* entries)
	{
		DebugInfoEntry* ptr = entries;

		for (auto it = coll.begin(); it != coll.end(); ++it)
		{
			memcpy(ptr->category, it->entry->category, sizeof(ptr->category));
			memcpy(ptr->name, it->entry->name, sizeof(ptr->name));
			ptr->value = it->GetValue(it->opaque, it->entry);
			ptr++;
		}
	}

	__declspec(dllexport)
	void GetDebugInfo(DebugInfoType type, DebugInfoEntry* entries)
	{
		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				CopyOutDebugInfo(core.testInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_Core:
				CopyOutDebugInfo(core.coreInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_CoreRegs:
				CopyOutDebugInfo(core.coreRegsInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_APU:
				CopyOutDebugInfo(core.apuInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_APURegs:
				CopyOutDebugInfo(core.apuRegsInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_PPU:
				CopyOutDebugInfo(core.ppuInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_PPURegs:
				CopyOutDebugInfo(core.ppuRegsInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_Board:
				CopyOutDebugInfo(core.boardInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_Cart:
				CopyOutDebugInfo(core.cartInfo, entries);
				break;
		}
	}

	__declspec(dllexport)
	size_t GetMemLayout()
	{
		return core.memMap.size();
	}

	__declspec(dllexport)
	void GetMemDescriptor(size_t descrID, MemDesciptor* descr)
	{
		size_t counter = 0;

		for (auto it = core.memMap.begin(); it != core.memMap.end(); ++it)
		{
			if (counter == descrID)
			{
				memcpy(descr, it->descr, sizeof(MemDesciptor));
				break;
			}

			counter++;
		}
	}

	__declspec(dllexport)
	void DumpMem(size_t descrID, uint8_t* ptr)
	{
		Breaknes::MemProvider prov;
		bool provFound = false;

		size_t counter = 0;

		for (auto it = core.memMap.begin(); it != core.memMap.end(); ++it)
		{
			if (counter == descrID)
			{
				prov = *it;
				provFound = true;
				break;
			}

			counter++;
		}

		if (provFound)
		{
			for (size_t addr = 0; addr < prov.descr->size; addr)
			{
				*ptr++ = prov.ReadByte(prov.opaque, addr);
			}
		}
	}

};
