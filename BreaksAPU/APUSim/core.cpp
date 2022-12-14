// 6502 Core Binding

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	CoreBinding::CoreBinding(APU* parent)
	{
		apu = parent;
	}
	
	CoreBinding::~CoreBinding()
	{
	}

	void CoreBinding::sim()
	{
		//apu->core->sim();
	}
}
