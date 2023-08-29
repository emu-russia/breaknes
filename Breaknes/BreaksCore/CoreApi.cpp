#include "pch.h"

Breaknes::Board* board = nullptr;

extern "C"
{
#ifdef _WIN32
	DLL_EXPORT int Assemble(char* str, uint8_t* buffer)
	{
		return assemble(str, buffer);
	}
#endif

	DLL_EXPORT void CreateBoard(char* boardName, char* apu, char* ppu, char* p1)
	{
		if (board == nullptr)
		{
			printf("CreateBoard\n");
			Breaknes::BoardFactory bf(boardName, apu, ppu, p1);
			CreateDebugHub(false);
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

	DLL_EXPORT void LoadRegDump(uint8_t* data, size_t data_size)
	{
		if (board != nullptr)
		{
			board->LoadRegDump(data, data_size);
		}
	}

	DLL_EXPORT void EnablePpuRegDump(bool enable, char* regdump_dir)
	{
		if (board != nullptr)
		{
			board->EnablePpuRegDump(enable, regdump_dir);
		}
	}

	DLL_EXPORT void EnableApuRegDump(bool enable, char* regdump_dir)
	{
		if (board != nullptr)
		{
			board->EnableApuRegDump(enable, regdump_dir);
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

	DLL_EXPORT size_t IOCreateInstance(uint32_t device_id)
	{
		if (board != nullptr && board->io != nullptr)
		{
			int handle = board->io->CreateInstance((IO::DeviceID)device_id);
			printf("IOCreateInstance: 0x%08X, handle: %d\n", device_id, handle);
			return handle;
		}
		else {
			return -1;
		}
	}

	DLL_EXPORT void IODisposeInstance(size_t handle)
	{
		if (board != nullptr && board->io != nullptr)
		{
			printf("IODisposeInstance: %d\n", (int)handle);
			board->io->DisposeInstance((int)handle);
		}
	}

	DLL_EXPORT void IOAttach(size_t port, size_t handle)
	{
		if (board != nullptr && board->io != nullptr)
		{
			board->io->Attach((int)port, (int)handle);
		}
	}

	DLL_EXPORT void IODetach(size_t port, size_t handle)
	{
		if (board != nullptr && board->io != nullptr)
		{
			board->io->Detach((int)port, (int)handle);
		}
	}

	DLL_EXPORT void IOSetState(size_t handle, size_t io_state, uint32_t value)
	{
		if (board != nullptr && board->io != nullptr)
		{
			board->io->SetState((int)handle, io_state, value);
		}
	}

	DLL_EXPORT uint32_t IOGetState(size_t handle, size_t io_state)
	{
		if (board != nullptr && board->io != nullptr)
		{
			return board->io->GetState((int)handle, io_state);
		}
		else {
			return 0;
		}
	}

	DLL_EXPORT size_t IOGetNumStates(size_t handle)
	{
		if (board != nullptr && board->io != nullptr)
		{
			return board->io->GetNumStates((int)handle);
		}
		else {
			return 0;
		}
	}

	DLL_EXPORT void IOGetStateName(size_t handle, size_t io_state, char* name, size_t name_size)
	{
		if (board != nullptr && board->io != nullptr)
		{
			auto state_name = board->io->GetStateName((int)handle, io_state);
			if (state_name.size() < name_size) {
				strncpy(name, state_name.c_str(), name_size);
				name[state_name.size()] = 0;
			}
		}
	}
};
