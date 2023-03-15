#include "pch.h"

Breaknes::Board* board = nullptr;

extern "C"
{
	DLL_EXPORT int Assemble(char* str, uint8_t* buffer)
	{
		return assemble(str, buffer);
	}

	DLL_EXPORT void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		if (board == nullptr)
		{
			printf("CreateBoard\n");
			Breaknes::BoardFactory bf(boardName, apu, ppu, p1);
			CreateDebugHub();
			board = bf.CreateInstance();
		}
	}

	DLL_EXPORT void DestroyBoard()
	{
		if (board != nullptr)
		{
			printf("DestroyBoard\n");
			delete board;
			board = nullptr;
			DisposeDebugHub();
		}
	}

	DLL_EXPORT int InsertCartridge(uint8_t* nesImage, size_t size)
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

	DLL_EXPORT void EjectCartridge()
	{
		if (board != nullptr)
		{
			printf("EjectCartridge\n");
			board->EjectCartridge();
		}
	}

	DLL_EXPORT void Step()
	{
		if (board != nullptr)
		{
			board->Step();
		}
	}

	DLL_EXPORT void Reset()
	{
		if (board != nullptr)
		{
			board->Reset();
		}
	}

	DLL_EXPORT bool InResetState()
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

	DLL_EXPORT size_t GetACLKCounter()
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

	DLL_EXPORT size_t GetPHICounter()
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

	DLL_EXPORT void SampleAudioSignal(float* sample)
	{
		if (board != nullptr)
		{
			board->SampleAudioSignal(sample);
		}
	}

	DLL_EXPORT void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
		if (board != nullptr)
		{
			board->LoadNSFData(data, data_size, load_address);
		}
	}

	DLL_EXPORT void EnableNSFBanking(bool enable)
	{
		if (board != nullptr)
		{
			board->EnableNSFBanking(enable);
		}
	}

	DLL_EXPORT void LoadRegDump(uint8_t* data, size_t data_size)
	{
		if (board != nullptr)
		{
			board->LoadRegDump(data, data_size);
		}
	}

	DLL_EXPORT void GetApuSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		if (board != nullptr)
		{
			board->GetApuSignalFeatures(features);
		}
	}

	DLL_EXPORT size_t GetPCLKCounter()
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

	DLL_EXPORT void SampleVideoSignal(PPUSim::VideoOutSignal* sample)
	{
		if (board != nullptr)
		{
			board->SampleVideoSignal(sample);
		}
	}

	DLL_EXPORT size_t GetHCounter()
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

	DLL_EXPORT size_t GetVCounter()
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

	DLL_EXPORT void RenderAlwaysEnabled(bool enable)
	{
		if (board != nullptr)
		{
			board->RenderAlwaysEnabled(enable);
		}
	}

	DLL_EXPORT void GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features)
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

	DLL_EXPORT void ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b)
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

	DLL_EXPORT void SetRAWColorMode(bool enable)
	{
		if (board != nullptr)
		{
			board->SetRAWColorMode(enable);
		}
	}

	DLL_EXPORT void SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior)
	{
		if (board != nullptr)
		{
			board->SetOamDecayBehavior(behavior);
		}
	}

	DLL_EXPORT void SetNoiseLevel(float volts)
	{
		if (board != nullptr)
		{
			board->SetNoiseLevel(volts);
		}
	}

	DLL_EXPORT void GetAllCoreDebugInfo(M6502Core::DebugInfo* info)
	{
		if (board != nullptr)
		{
			board->GetAllCoreDebugInfo(info);
		}
	}
};
