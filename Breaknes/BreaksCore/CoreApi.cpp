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
		return 0;
	}

	__declspec(dllexport)
	void GetDebugInfo(DebugInfoType type, DebugInfoEntry* entries)
	{

	}

};
