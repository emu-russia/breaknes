// Timing Generator

#include "pch.h"

namespace FastAPU
{
	CLKGen::CLKGen(FastAPU* parent)
	{
		apu = parent;
	}

	CLKGen::~CLKGen()
	{
	}

	void CLKGen::sim()
	{
		sim_ACLK();
		sim_SoftCLK_Mode();
		sim_SoftCLK_PLA();
		sim_SoftCLK_Control();
		sim_SoftCLK_LFSR();
	}

	int CLKGen::GetINTFF()
	{
		return int_ff;
	}

	void CLKGen::sim_ACLK()
	{

	}

	void CLKGen::sim_SoftCLK_Mode()
	{
		if (apu->RegOps.W4017 && !apu->wire.ACLK1) {
			reg_mode = apu->GetDBBit(7);
		}
		n_mode = !reg_mode;
		if (apu->wire.ACLK1) {
			md_latch = n_mode;
		}
		mode = !md_latch;
	}

	void CLKGen::sim_SoftCLK_Control()
	{

	}

	void CLKGen::sim_SoftCLK_PLA()
	{

	}

	void CLKGen::sim_SoftCLK_LFSR()
	{

	}
}
