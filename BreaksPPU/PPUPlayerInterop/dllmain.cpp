// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"

PPUPlayer::Board * board;

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
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
			board = new PPUPlayer::Board(boardName, apu, ppu, p1);
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

	__declspec(dllexport) void CPUWrite(size_t ppuReg, uint8_t val)
	{
		if (board != nullptr)
		{
			printf("CPUWrite: %d 0x%02X; H/V = %zd/%zd, PCLK = %zd\n", 
				(uint8_t)ppuReg, val, board->GetHCounter(), board->GetVCounter(), board->GetPCLKCounter() );
			board->CPUWrite(ppuReg, val);
		}
	}

	__declspec(dllexport) void CPURead(size_t ppuReg)
	{
		if (board != nullptr)
		{
			//printf("CPURead: %d\n", (uint8_t)ppuReg);
			board->CPURead(ppuReg);
		}
	}

	__declspec(dllexport) size_t GetPCLKCounter()
	{
		if (board != nullptr)
		{
			return board->GetPCLKCounter();
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) int InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		if (board != nullptr)
		{
			printf("InsertCartridge: %zi bytes\n", nesImageSize);
			return board->InsertCartridge(nesImage, nesImageSize);
		}
		else
		{
			return -1;
		}
	}

	__declspec(dllexport) void EjectCartridge()
	{
		if (board != nullptr)
		{
			printf("EjectCartridge\n");
			board->EjectCartridge();
		}
	}

	__declspec(dllexport) void SampleVideoSignal(PPUSim::VideoOutSignal* sample)
	{
		if (board != nullptr)
		{
			board->SampleVideoSignal(sample);
		}
	}

	__declspec(dllexport) size_t GetHCounter()
	{
		if (board != nullptr)
		{
			return board->GetHCounter();
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) size_t GetVCounter()
	{
		if (board != nullptr)
		{
			return board->GetVCounter();
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) void Reset()
	{
		if (board != nullptr)
		{
			board->Reset();
		}
	}

	__declspec(dllexport) size_t InResetState()
	{
		if (board != nullptr)
		{
			return board->InResetState() ? 1 : 0;
		}
		else
		{
			return 0;
		}
	}

	__declspec(dllexport) void RenderAlwaysEnabled(bool enable)
	{
		if (board != nullptr)
		{
			board->RenderAlwaysEnabled(enable);
		}
	}

	__declspec(dllexport) void GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features)
	{
		if (board != nullptr)
		{
			board->GetPpuSignalFeatures(features);
		}
		else
		{
			// If the Board is not created return the default values for the NTSC PPU.

			features->SamplesPerPCLK = 8;
			features->PixelsPerScan = 341;
			features->ScansPerField = 262;
			features->BackPorchSize = 40;
			features->Composite = true;
			features->BlackLevel = 1.3f;
			features->WhiteLevel = 1.6f;
			features->SyncLevel = 0.781f;
		}
	}

	__declspec(dllexport) void ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b)
	{
		if (board != nullptr)
		{
			board->ConvertRAWToRGB(raw, r, g, b);
		}
		else
		{
			*r = *g = *b = 0;
		}
	}

	__declspec(dllexport) void SetRAWColorMode(bool enable)
	{
		if (board != nullptr)
		{
			board->SetRAWColorMode(enable);
		}
	}

	__declspec(dllexport) void SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior)
	{
		if (board != nullptr)
		{
			board->SetOamDecayBehavior(behavior);
		}
	}

	__declspec(dllexport) void SetNoiseLevel(float volts)
	{
		if (board != nullptr)
		{
			board->SetNoiseLevel(volts);
		}
	}
}
