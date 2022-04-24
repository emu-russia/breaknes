// Picture Address Register

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	PAR::PAR(PPU* parent)
	{
		ppu = parent;
	}

	PAR::~PAR()
	{
	}

	void PAR::sim()
	{
		sim_CountersControl();
		sim_CountersCarry();
		sim_Control();
		sim_FVCounter();
		sim_NTCounters();
		sim_TVCounter();
		sim_THCounter();
		sim_PARInputs();
		sim_PAR();
	}

	void PAR::sim_CountersControl()
	{

	}

	void PAR::sim_CountersCarry()
	{

	}

	void PAR::sim_Control()
	{

	}

	void PAR::sim_FVCounter()
	{

	}

	void PAR::sim_NTCounters()
	{

	}

	void PAR::sim_TVCounter()
	{

	}

	void PAR::sim_THCounter()
	{

	}

	void PAR::sim_PARInputs()
	{

	}

	void PAR::sim_PAR()
	{

	}

	void PAR_CounterBit::sim()
	{
	}

	void PAR_CounterBit::sim_res()
	{
	}

	void PAR_LowBit::sim()
	{
	}

	void PAR_HighBit::sim()
	{
	}
}
