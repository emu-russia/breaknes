#include "pch.h"

Breaknes::Board* board = nullptr;

extern "C"
{
	__declspec(dllexport) int Assemble(char* str, uint8_t* buffer)
	{
		return assemble(str, buffer);
	}

	__declspec(dllexport) void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		if (board == nullptr)
		{
			printf("CreateBoard\n");
			Breaknes::BoardFactory bf(boardName, apu, ppu, p1);
			CreateDebugHub();
			board = bf.CreateInstance();
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

	__declspec(dllexport) int InsertCartridge(uint8_t* nesImage, size_t size)
	{
		if (board != nullptr)
		{
			printf("InsertCartridge: %zi bytes\n", size);
			return board->InsertCartridge(nesImage, size);
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

	__declspec(dllexport) void Step()
	{
		if (board != nullptr)
		{
			board->Step();
		}
	}

	__declspec(dllexport) void Reset()
	{
		if (board != nullptr)
		{
			board->Reset();
		}
	}

	__declspec(dllexport) bool InResetState()
	{
		if (board != nullptr)
		{
			return board->InResetState();
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

	__declspec(dllexport) void LoadRegDump(uint8_t* data, size_t data_size)
	{
		if (board != nullptr)
		{
			board->LoadRegDump(data, data_size);
		}
	}

	__declspec(dllexport) void GetApuSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		if (board != nullptr)
		{
			board->GetApuSignalFeatures(features);
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

	__declspec(dllexport) void GetAllCoreDebugInfo(M6502Core::DebugInfo* info)
	{
		if (board != nullptr)
		{
			board->GetAllCoreDebugInfo(info);
		}
	}
};
