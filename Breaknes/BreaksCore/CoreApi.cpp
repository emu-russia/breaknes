#include "pch.h"

extern "C"
{

	__declspec(dllexport)
	void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{

	}

	__declspec(dllexport)
	void DestroyBoard()
	{

	}

	__declspec(dllexport)
	void InsertCartridge(uint8_t* nesImage, size_t size)
	{

	}

	__declspec(dllexport)
	void EjectCartridge()
	{

	}

	__declspec(dllexport)
	void Sim(size_t numHalfCycles)
	{

	}

	__declspec(dllexport)
	size_t GetDebugInfoEntryCount(DebugInfoType type)
	{
		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				return core.testInfo.size();
		}

		return 0;
	}

	__declspec(dllexport)
	void GetDebugInfo(DebugInfoType type, DebugInfoEntry* entries)
	{
		DebugInfoEntry* ptr = entries;

		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
			{
				for (auto it = core.testInfo.begin(); it != core.testInfo.end(); ++it)
				{
					memcpy(ptr, *it, sizeof(DebugInfoEntry));
					ptr++;
				}
			}
			break;
		}
	}

	__declspec(dllexport)
	size_t GetMemLayout()
	{
		return 0;
	}

	__declspec(dllexport)
	void GetMemDescriptor(size_t descrID, MemDesciptor* descr)
	{

	}

	__declspec(dllexport)
	void DumpMem(size_t descrID, uint8_t* ptr)
	{

	}

};
