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
    __declspec(dllexport) void CreateBoard()
    {
        if (board == nullptr)
        {
            board = new PPUPlayer::Board();
        }
    }

    __declspec(dllexport) void DestroyBoard()
    {
        if (board != nullptr)
        {
            delete board;
            board = nullptr;
        }
    }

    __declspec(dllexport) void Step()
    {
        board->Step();
    }

    __declspec(dllexport) void CPUWrite(size_t ppuReg, uint8_t val)
    {
        board->CPUWrite(ppuReg, val);
    }

    __declspec(dllexport) size_t GetPCLKCounter()
    {
        return board->GetPCLKCounter();
    }

    __declspec(dllexport) void InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
    {
        board->InsertCartridge(nesImage, nesImageSize);
    }

    __declspec(dllexport) void EjectCartridge()
    {
        board->EjectCartridge();
    }

    __declspec(dllexport) void SampleVideoSignal(float* sample)
    {
        board->SampleVideoSignal(sample);
    }
}
