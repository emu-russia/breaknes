#include "pch.h"

namespace NSFPlayer
{
	Board* InstantiateBoard(char* boardName, char* apuRev, char* ppuRev, char* p1)
	{
		if (!strcmp(boardName, "NSFPlayer"))
			return new NSFPlayerBoard(boardName, apuRev, ppuRev, p1);
		else if (!strcmp(boardName, "APUPlayer"))
			return new APUPlayerBoard(boardName, apuRev, ppuRev, p1);
		else
			return nullptr;
	}
}
