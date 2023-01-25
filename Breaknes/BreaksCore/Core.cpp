#include "pch.h"

namespace Breaknes
{
	Core::Core()
	{
		CreateDebugHub();

		// DEBUG: Add one test DebugInfo and one test memory region to test the debugger GUI.

		DebugInfoEntry* testEntry = new DebugInfoEntry;
		memset(testEntry, 0, sizeof(DebugInfoEntry));
		strcpy(testEntry->category, "Test");
		strcpy(testEntry->name, "test");
		dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Test, testEntry, GetTestInfo, this);

		MemDesciptor* testRegion = new MemDesciptor;
		memset(testRegion, 0, sizeof(MemDesciptor));
		strcpy(testRegion->name, "TestMem");
		testRegion->size = 256;
		dbg_hub->AddMemRegion(testRegion, ReadTestMem, WriteTestMem, this, false);
	}

	Core::~Core()
	{
		DisposeDebugHub();
		DisposeCartridge();
		DisposeBoard();
	}

	// DEBUG: To be deleted after GUI debugging
	uint32_t Core::GetTestInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits)
	{
		bits = 8;
		return 123;
	}

	// DEBUG: To be deleted after GUI debugging
	uint8_t Core::ReadTestMem(void* opaque, size_t addr)
	{
		return (uint8_t)addr;
	}

	// DEBUG: To be deleted after GUI debugging
	void Core::WriteTestMem(void* opaque, size_t addr, uint8_t data)
	{
	}

	void Core::SwitchBoard(Board* board)
	{
		// This method requires that there is no previous association with any board, otherwise there will be leaks.
		assert(this_board == nullptr);

		this_board = board;
	}
	
	void Core::DisposeBoard()
	{
		if (this_board)
		{
			delete this_board;
			this_board = nullptr;
		}
	}

	void Core::AttachCartridge(AbstractCartridge* cart)
	{
		if (this_board)
		{
			this_board->InsertCartridge(cart);
		}
	}

	void Core::DisposeCartridge()
	{
		if (this_board)
		{
			this_board->DestroyCartridge();

			// It is also necessary to clean DebugInfo for the previous cartridge + Delete all memory regions of the previous cartridge
			dbg_hub->DisposeCartDebugInfo();
			dbg_hub->DisposeCartMemMap();
		}
	}

	void Core::Sim()
	{
		if (this_board)
		{
			BaseLogic::TriState inputs[1];
			BaseLogic::TriState outputs[1];
			float analogOutputs[1];

			this_board->sim(inputs, outputs, analogOutputs);
		}
	}
}
