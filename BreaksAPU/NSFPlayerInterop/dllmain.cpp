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

	__declspec(dllexport) void ResetAPU(uint16_t addr, bool reset_apu_also)
	{
		if (board != nullptr)
		{
			board->ResetAPU(addr, reset_apu_also);
		}
	}

	__declspec(dllexport) bool APUInResetState()
	{
		if (board != nullptr)
		{
			return board->APUInResetState();
		}
		else
		{
			return false;
		}
	}

	__declspec(dllexport) size_t GetACLKCounter()
	{
		if (board != nullptr)
		{
			return board->GetACLKCounter();
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) size_t GetPHICounter()
	{
		if (board != nullptr)
		{
			return board->GetPHICounter();
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) void SampleAudioSignal(float* sample)
	{
		if (board != nullptr)
		{
			board->SampleAudioSignal(sample);
		}
	}

	__declspec(dllexport) void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
		if (board != nullptr)
		{
			board->LoadNSFData(data, data_size, load_address);
		}
	}

	__declspec(dllexport) void EnableNSFBanking(bool enable)
	{
		if (board != nullptr)
		{
			board->EnableNSFBanking(enable);
		}
	}

	__declspec(dllexport) void GetSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		if (board != nullptr)
		{
			board->GetSignalFeatures(features);
		}
	}
}
