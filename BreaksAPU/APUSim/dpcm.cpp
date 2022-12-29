// Differential Pulse-code Modulation (DPCM)

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	DpcmChan::DpcmChan(APU* parent)
	{
		apu = parent;
	}

	DpcmChan::~DpcmChan()
	{
	}

	void DpcmChan::sim()
	{
	}

	void DpcmChan::sim_IntControl()
	{

	}

	void DpcmChan::sim_EnableControl()
	{

	}

	void DpcmChan::sim_DMAControl()
	{

	}

	void DpcmChan::sim_SampleCounterControl()
	{

	}

	void DpcmChan::sim_SampleBufferControl()
	{

	}

	void DpcmChan::sim_ControlReg()
	{

	}

	void DpcmChan::sim_Decoder1()
	{
		TriState F[4];
		TriState nF[4];

		for (size_t n = 0; n < 4; n++)
		{
			F[n] = Fx[n];
			nF[n] = NOT(Fx[n]);
		}

		Dec1_out[0] = NOR4(F[0], F[1], F[2], F[3]);
		Dec1_out[1] = NOR4(nF[0], F[1], F[2], F[3]);
		Dec1_out[2] = NOR4(F[0], nF[1], F[2], F[3]);
		Dec1_out[3] = NOR4(nF[0], nF[1], F[2], F[3]);
		Dec1_out[4] = NOR4(F[0], F[1], nF[2], F[3]);
		Dec1_out[5] = NOR4(nF[0], F[1], nF[2], F[3]);
		Dec1_out[6] = NOR4(F[0], nF[1], nF[2], F[3]);
		Dec1_out[7] = NOR4(nF[0], nF[1], nF[2], F[3]);

		Dec1_out[8] = NOR4(F[0], F[1], F[2], nF[3]);
		Dec1_out[9] = NOR4(nF[0], F[1], F[2], nF[3]);
		Dec1_out[10] = NOR4(F[0], nF[1], F[2], nF[3]);
		Dec1_out[11] = NOR4(nF[0], nF[1], F[2], nF[3]);
		Dec1_out[12] = NOR4(F[0], F[1], nF[2], nF[3]);
		Dec1_out[13] = NOR4(nF[0], F[1], nF[2], nF[3]);
		Dec1_out[14] = NOR4(F[0], nF[1], nF[2], nF[3]);
		Dec1_out[15] = NOR4(nF[0], nF[1], nF[2], nF[3]);
	}

	void DpcmChan::sim_Decoder2()
	{
		TriState* d = Dec1_out;

		FR[0] = NOR5(d[1], d[4], d[9], d[14], d[15]);
		FR[1] = NOR8(d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13]);
		FR[2] = NOR9(d[0], d[1], d[2], d[3], d[7], d[8], d[10], d[11], d[13]);
		FR[3] = NOR5(d[0], d[2], d[7], d[10], d[15]);
		FR[4] = NOR7(d[1], d[2], d[4], d[8], d[12], d[13], d[14]);
		FR[5] = NOR10(d[1], d[2], d[3], d[7], d[8], d[9], d[12], d[13], d[14], d[15]);
		FR[6] = NOR6(d[1], d[5], d[8], d[12], d[13], d[14]);
		FR[7] = NOR6(d[0], d[2], d[5], d[6], d[8], d[15]);
		FR[8] = NOR9(d[1], d[3], d[5], d[6], d[8], d[9], d[10], d[11], d[12]);
	}

	void DpcmChan::sim_FreqLFSR()
	{

	}

	void DpcmChan::sim_SampleCounterReg()
	{

	}

	void DpcmChan::sim_SampleCounter()
	{

	}

	void DpcmChan::sim_SampleBitCounter()
	{

	}

	void DpcmChan::sim_SampleBuffer()
	{

	}

	void DpcmChan::sim_AddressReg()
	{

	}

	void DpcmChan::sim_AddressCounter()
	{

	}

	void DpcmChan::sim_Output()
	{

	}
}
