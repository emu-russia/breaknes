#include "pch.h"

Breaknes::Board* board = nullptr;

extern "C"
{
    __declspec(dllexport)
    int Assemble(char* str, uint8_t* buffer)
    {
        return assemble(str, buffer);
    }

	__declspec(dllexport)
	void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		if (board == nullptr)
		{
			printf("CreateBoard\n");
			Breaknes::BoardFactory bf(boardName, apu, ppu, p1);
			CreateDebugHub();
			board = bf.CreateInstance();
		}
	}

	__declspec(dllexport)
	void DestroyBoard()
	{
		if (board != nullptr)
		{
			printf("DestroyBoard\n");
			delete board;
			board = nullptr;
			DisposeDebugHub();
		}
	}

    // hmmm cartridge api...

	__declspec(dllexport)
	int InsertCartridge(uint8_t* nesImage, size_t size)
	{
		Breaknes::CartridgeFactory cf(nesImage, size);
		//core.AttachCartridge(cf.GetInstance(&core));
		return 0;
	}

	__declspec(dllexport)
	void EjectCartridge()
	{
		//core.DisposeCartridge();
	}

	__declspec(dllexport)
	void Step()
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
};
