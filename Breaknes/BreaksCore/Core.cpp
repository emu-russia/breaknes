#include "pch.h"

namespace Breaknes
{
	Core::Core()
	{
		// DEBUG: Add one test DebugInfo and one test memory region to test the debugger GUI.

		DebugInfoEntry* testEntry = new DebugInfoEntry;
		memset(testEntry, 0, sizeof(DebugInfoEntry));
		strcpy(testEntry->category, "Test");
		strcpy(testEntry->name, "test");
		AddDebugInfo(DebugInfoType::DebugInfoType_Test, testEntry, GetTestInfo, this);

		MemDesciptor* testRegion = new MemDesciptor;
		memset(testRegion, 0, sizeof(MemDesciptor));
		strcpy(testRegion->name, "TestMem");
		testRegion->size = 256;
		AddMemRegion(testRegion, ReadTestMem, this);
	}

	Core::~Core()
	{
		DisposeDebugInfo();
		DisposeMemMap();
		DisposeCartridge();
		DisposeBoard();
	}

	void Core::AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry), void* opaque)
	{
		DebugInfoProvider prov;

		prov.entry = entry;
		prov.GetValue = GetValue;
		prov.opaque = opaque;

		switch (type)
		{
			case DebugInfoType::DebugInfoType_Test:
				testInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_Core:
				coreInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_CoreRegs:
				coreRegsInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_APU:
				apuInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_APURegs:
				apuRegsInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_PPU:
				ppuInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_PPURegs:
				ppuRegsInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_Board:
				boardInfo.push_back(prov);
				break;

			case DebugInfoType::DebugInfoType_Cart:
				cartInfo.push_back(prov);
				break;
		}
	}

	void Core::DisposeDebugInfo()
	{
		for (auto it = testInfo.begin(); it != testInfo.end(); ++it)
		{
			delete it->entry;
		}
		testInfo.clear();

		for (auto it = coreInfo.begin(); it != coreInfo.end(); ++it)
		{
			delete it->entry;
		}
		coreInfo.clear();

		for (auto it = coreRegsInfo.begin(); it != coreRegsInfo.end(); ++it)
		{
			delete it->entry;
		}
		coreRegsInfo.clear();

		for (auto it = apuInfo.begin(); it != apuInfo.end(); ++it)
		{
			delete it->entry;
		}
		apuInfo.clear();

		for (auto it = apuRegsInfo.begin(); it != apuRegsInfo.end(); ++it)
		{
			delete it->entry;
		}
		apuRegsInfo.clear();

		for (auto it = ppuInfo.begin(); it != ppuInfo.end(); ++it)
		{
			delete it->entry;
		}
		ppuInfo.clear();

		for (auto it = ppuRegsInfo.begin(); it != ppuRegsInfo.end(); ++it)
		{
			delete it->entry;
		}
		ppuRegsInfo.clear();

		for (auto it = boardInfo.begin(); it != boardInfo.end(); ++it)
		{
			delete it->entry;
		}
		boardInfo.clear();

		for (auto it = cartInfo.begin(); it != cartInfo.end(); ++it)
		{
			delete it->entry;
		}
		cartInfo.clear();
	}

	void Core::AddMemRegion(MemDesciptor* descr, uint8_t(*ReadByte)(void* opaque, size_t addr), void* opaque)
	{
		MemProvider prov;
		prov.descr = descr;
		prov.ReadByte = ReadByte;
		prov.opaque = opaque;
		memMap.push_back(prov);
	}

	void Core::DisposeMemMap()
	{
		for (auto it = memMap.begin(); it != memMap.end(); ++it)
		{
			delete it->descr;
		}
		memMap.clear();
	}

	// DEBUG: To be deleted after GUI debugging
	uint32_t Core::GetTestInfo(void* opaque, DebugInfoEntry* entry)
	{
		return 123;
	}

	// DEBUG: To be deleted after GUI debugging
	uint8_t Core::ReadTestMem(void* opaque, size_t addr)
	{
		return (uint8_t)addr;
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
