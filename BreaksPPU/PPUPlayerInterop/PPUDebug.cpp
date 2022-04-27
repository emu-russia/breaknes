// A debug module that basically replicates the Breaknes debug interface and partially implements it for the PPU.

#include "pch.h"

PPUPlayer::DebugHub* dbg_hub;

namespace PPUPlayer
{
	DebugHub::DebugHub()
	{
		// DEBUG: Add one test DebugInfo and one test memory region to test the debugger GUI.

		DebugInfoEntry* testEntry = new DebugInfoEntry;
		memset(testEntry, 0, sizeof(DebugInfoEntry));
		strcpy_s(testEntry->category, sizeof(testEntry->category), "Test");
		strcpy_s(testEntry->name, sizeof(testEntry->name), "test");
		AddDebugInfo(DebugInfoType::DebugInfoType_Test, testEntry, GetTestInfo, this);

		MemDesciptor* testRegion = new MemDesciptor;
		memset(testRegion, 0, sizeof(MemDesciptor));
		strcpy_s(testRegion->name, sizeof(testRegion->name), "TestMem");
		testRegion->size = 256;
		AddMemRegion(testRegion, ReadTestMem, this, false);
	}

	DebugHub::~DebugHub()
	{
		DisposeDebugInfo();
		DisposeMemMap();
	}

	void DebugHub::AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry), void* opaque)
	{
		DebugInfoProvider prov{};

		prov.entry = entry;
		prov.GetValue = GetValue;
		prov.opaque = opaque;

		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				testInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_PPU:
				ppuInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_PPURegs:
				ppuRegsInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_Board:
				boardInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_Cart:
				cartInfo.push_back(prov);
				break;
		}
	}

	void DebugHub::DisposeDebugInfo()
	{
		for (auto it = testInfo.begin(); it != testInfo.end(); ++it)
		{
			delete it->entry;
		}
		testInfo.clear();

		for (auto it = ppuInfo.begin(); it != ppuInfo.end(); ++it)
		{
			delete it->entry;
		}
		ppuInfo.clear();

		for (auto it = ppuRegsInfo.begin(); it != ppuRegsInfo.end(); ++it)
		{
			delete it->entry;
		}
		ppuRegsInfo.clear();

		for (auto it = boardInfo.begin(); it != boardInfo.end(); ++it)
		{
			delete it->entry;
		}
		boardInfo.clear();

		for (auto it = cartInfo.begin(); it != cartInfo.end(); ++it)
		{
			delete it->entry;
		}
		cartInfo.clear();
	}

	void DebugHub::AddMemRegion(MemDesciptor* descr, uint8_t(*ReadByte)(void* opaque, size_t addr), void* opaque, bool cartRelated)
	{
		MemProvider prov{};
		prov.descr = descr;
		prov.ReadByte = ReadByte;
		prov.opaque = opaque;
		prov.cartRelated = cartRelated;
		memMap.push_back(prov);
	}

	void DebugHub::DisposeMemMap()
	{
		for (auto it = memMap.begin(); it != memMap.end(); ++it)
		{
			delete it->descr;
		}
		memMap.clear();
	}

	// DEBUG: To be deleted after GUI debugging
	uint32_t DebugHub::GetTestInfo(void* opaque, DebugInfoEntry* entry)
	{
		return 123;
	}

	// DEBUG: To be deleted after GUI debugging
	uint8_t DebugHub::ReadTestMem(void* opaque, size_t addr)
	{
		return (uint8_t)addr;
	}
}

void CreateDebugHub()
{
	if (dbg_hub == nullptr)
	{
		dbg_hub = new PPUPlayer::DebugHub();
	}
}

void DisposeDebugHub()
{
	if (dbg_hub != nullptr)
	{
		delete dbg_hub;
		dbg_hub = nullptr;
	}
}

// Debug Interop API.

extern "C"
{
	/// <summary>
	/// Get the number of entries for the specified debugging information type.
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	__declspec(dllexport)
	size_t GetDebugInfoEntryCount(DebugInfoType type)
	{
		if (!dbg_hub)
			return 0;

		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				return dbg_hub->testInfo.size();

			case DebugInfoType::DebugInfoType_Core:
				return 0;

			case DebugInfoType::DebugInfoType_CoreRegs:
				return 0;

			case DebugInfoType::DebugInfoType_APU:
				return 0;

			case DebugInfoType::DebugInfoType_APURegs:
				return 0;

			case DebugInfoType::DebugInfoType_PPU:
				return dbg_hub->ppuInfo.size();

			case DebugInfoType::DebugInfoType_PPURegs:
				return dbg_hub->ppuRegsInfo.size();

			case DebugInfoType::DebugInfoType_Board:
				return dbg_hub->boardInfo.size();

			case DebugInfoType::DebugInfoType_Cart:
				return dbg_hub->cartInfo.size();
		}

		return 0;
	}

	void CopyOutDebugInfo(std::list<PPUPlayer::DebugInfoProvider>& coll, DebugInfoEntry* entries)
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

	/// <summary>
	/// Get debug entries of the specified type.
	/// </summary>
	/// <param name="type"></param>
	/// <param name="entries"></param>
	__declspec(dllexport)
	void GetDebugInfo(DebugInfoType type, DebugInfoEntry* entries)
	{
		if (!dbg_hub)
			return;

		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				CopyOutDebugInfo(dbg_hub->testInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_PPU:
				CopyOutDebugInfo(dbg_hub->ppuInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_PPURegs:
				CopyOutDebugInfo(dbg_hub->ppuRegsInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_Board:
				CopyOutDebugInfo(dbg_hub->boardInfo, entries);
				break;

			case DebugInfoType::DebugInfoType_Cart:
				CopyOutDebugInfo(dbg_hub->cartInfo, entries);
				break;
		}
	}

	/// <summary>
	/// Get the number of memory descriptors that are registered in the core
	/// </summary>
	/// <returns></returns>
	__declspec(dllexport)
	size_t GetMemLayout()
	{
		if (!dbg_hub)
			return 0;

		return dbg_hub->memMap.size();
	}

	/// <summary>
	/// Get information about the memory block
	/// </summary>
	/// <param name="descrID"></param>
	/// <param name="descr"></param>
	__declspec(dllexport)
	void GetMemDescriptor(size_t descrID, MemDesciptor* descr)
	{
		if (!dbg_hub)
			return;

		size_t counter = 0;

		for (auto it = dbg_hub->memMap.begin(); it != dbg_hub->memMap.end(); ++it)
		{
			if (counter == descrID)
			{
				memcpy(descr, it->descr, sizeof(MemDesciptor));
				break;
			}

			counter++;
		}
	}

	/// <summary>
	/// Dump the whole memory block. We are emulating NES here, so the dump sizes will be small and there is no point in dumping in parts.
	/// </summary>
	/// <param name="descrID"></param>
	/// <param name="ptr"></param>
	__declspec(dllexport)
	void DumpMem(size_t descrID, uint8_t* ptr)
	{
		if (!dbg_hub)
			return;

		PPUPlayer::MemProvider prov;
		bool provFound = false;

		size_t counter = 0;

		for (auto it = dbg_hub->memMap.begin(); it != dbg_hub->memMap.end(); ++it)
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
			for (size_t addr = 0; addr < prov.descr->size; addr++)
			{
				*ptr++ = prov.ReadByte(prov.opaque, addr);
			}
		}
	}
};
