// Square Channels

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	SquareChan::SquareChan(APU* parent, SquareChanCarryIn carry_routing)
	{
		apu = parent;
		cin_type = carry_routing;
	}

	SquareChan::~SquareChan()
	{
	}

	void SquareChan::sim()
	{

	}
}
