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
		core.DisposeCartridge();
		core.DisposeBoard();
	}

	__declspec(dllexport)
	int InsertCartridge(uint8_t* nesImage, size_t size)
	{
		Breaknes::CartridgeFactory cf(nesImage, size);
		core.AttachCartridge(cf.GetInstance(&core));
		return 0;
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
};
