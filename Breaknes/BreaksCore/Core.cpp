#include "pch.h"

namespace Breaknes
{
	Core::Core()
	{
		DebugInfoEntry* testEntry = new DebugInfoEntry;

		memset(testEntry, 0, sizeof(DebugInfoEntry));

		strcpy(testEntry->category, "Test");
		strcpy(testEntry->name, "test");
		testEntry->value =123;

		testInfo.push_back(testEntry);
	}

	Core::~Core()
	{
		for (auto it = testInfo.begin(); it != testInfo.end(); ++it)
		{
			delete* it;
		}
	}
}
