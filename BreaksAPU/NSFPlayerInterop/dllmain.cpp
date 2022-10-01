// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

NSFPlayer::Board* board;

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

// Interop API.

extern "C"
{
	__declspec(dllexport) void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		if (board == nullptr)
		{
			printf("CreateBoard\n");
			CreateDebugHub();
			board = new NSFPlayer::Board(boardName, apu, ppu, p1);
		}
	}

	__declspec(dllexport) void DestroyBoard()
	{
		if (board != nullptr)
		{
			printf("DestroyBoard\n");
			delete board;
			board = nullptr;
			DisposeDebugHub();
		}
	}

	__declspec(dllexport) void Step()
	{
		if (board != nullptr)
		{
			//printf("Step\n");
			board->Step();
		}
	}
}
