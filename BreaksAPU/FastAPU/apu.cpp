#include "pch.h"

namespace FastAPU
{
	FastAPU::FastAPU(M6502Core::M6502* _core, APUSim::Revision _rev)
	{
		// For ease of integration, the core instance is created by the consumer
		core = _core;
		rev = _rev;

		core_int = new CoreBinding(this);
		clkgen = new CLKGen(this);
		lc[0] = new LengthCounter(this);
		lc[1] = new LengthCounter(this);
		lc[2] = new LengthCounter(this);
		lc[3] = new LengthCounter(this);
		dpcm = new DpcmChan(this);
		noise = new NoiseChan(this);
		square[0] = new SquareChan(this, SquareChanCarryIn::Vdd);
		square[1] = new SquareChan(this, SquareChanCarryIn::Inc);
		tri = new TriangleChan(this);
		regs = new RegsDecoder(this);
		dma = new DMA(this);
		pads = new Pads(this);
		dac = new DAC(this);

		wire.RDY2 = 1;
	}

	FastAPU::~FastAPU()
	{
		delete core_int;
		delete clkgen;
		delete lc[0];
		delete lc[1];
		delete lc[2];
		delete lc[3];
		delete dpcm;
		delete noise;
		delete square[0];
		delete square[1];
		delete tri;
		delete regs;
		delete dma;
		delete pads;
		delete dac;
	}

	void FastAPU::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t* data, uint16_t* addr, APUSim::AudioOutSignal& AUX)
	{
		pads->sim_InputPads(inputs);
		pads->sim_DataBusInput(data);

		dpcm->sim();

		dma->sim();
		dma->sim_DMA_Buffer();
		dma->sim_AddressMux();

		sim_CoreIntegration();

		sim_SoundGenerators();

		pads->sim_OutputPads(outputs, addr);
		pads->sim_DataBusOutput(data);
		dac->sim(AUX);
	}

	void FastAPU::sim_CoreIntegration()
	{
		// Core & stuff

		core_int->sim();
		clkgen->sim();
		regs->sim();
		regs->sim_DebugRegisters();
	}

	void FastAPU::sim_SoundGenerators()
	{
		// The audio generator circuits are discrete to PHI (Core clock), there is no point in simulating them every CLK

		if (wire.PHI0 == PrevPHI_SoundGen) {
			return;
		}
		PrevPHI_SoundGen = wire.PHI0;

		// Sound channels

		wire.SQA_LC = square[0]->get_LC();
		wire.SQB_LC = square[1]->get_LC();
		wire.TRI_LC = tri->get_LC();
		wire.RND_LC = noise->get_LC();

		lc[0]->sim(0, RegOps.W4003, wire.SQA_LC, wire.NOSQA);
		lc[1]->sim(1, RegOps.W4007, wire.SQB_LC, wire.NOSQB);
		lc[2]->sim(2, RegOps.W400B, wire.TRI_LC, wire.NOTRI);
		lc[3]->sim(3, RegOps.W400F, wire.RND_LC, wire.NORND);

		square[0]->sim(RegOps.W4000, RegOps.W4001, RegOps.W4002, RegOps.W4003, wire.NOSQA, &SQA_Out);
		square[1]->sim(RegOps.W4004, RegOps.W4005, RegOps.W4006, RegOps.W4007, wire.NOSQB, &SQB_Out);
		tri->sim();
		noise->sim();
	}

	void FastAPU::SetRAWOutput(bool enable)
	{
		dac->SetRAWOutput(enable);
	}

	void FastAPU::SetNormalizedOutput(bool enable)
	{
		dac->SetNormalizedOutput(enable);
	}

	size_t FastAPU::GetACLKCounter()
	{
		return aclk_counter;
	}

	void FastAPU::ResetACLKCounter()
	{
		aclk_counter = 0;
	}

	size_t FastAPU::GetPHICounter()
	{
		return phi_counter;
	}

	void FastAPU::ResetPHICounter()
	{
		phi_counter = 0;
	}

	int FastAPU::GetDBBit(size_t n)
	{
		return (DB & (1 << n)) != 0 ? 1 : 0;
	}

	void FastAPU::SetDBBit(size_t n, int bit_val)
	{
		uint8_t out = DB & ~(1 << n);
		out |= (bit_val & 1) << n;
		DB = out;
	}

	void FastAPU::GetSignalFeatures(APUSim::AudioSignalFeatures& features)
	{
		size_t clk = 21477272;	// Hz
		size_t div = 12;

		features.SampleRate = (int32_t)(clk * 2);

		// TBD: Add other APU
	}
}
