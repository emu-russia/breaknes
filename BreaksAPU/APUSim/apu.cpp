#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	APU::APU(M6502Core::M6502* _core, Revision _rev)
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

		wire.RDY2 = TriState::One;
		wire.RESCore = TriState::Zero;
	}

	APU::~APU()
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

	void APU::sim(TriState inputs[], TriState outputs[], uint8_t* data, uint16_t* addr, AudioOutSignal& AUX)
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

	void APU::sim_CoreIntegration()
	{
		// Core & stuff

		core_int->sim();
		clkgen->sim();
		regs->sim();
		regs->sim_DebugRegisters();
	}

	void APU::sim_SoundGenerators()
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

		lc[0]->sim(0, wire.W4003, wire.SQA_LC, wire.NOSQA);
		lc[1]->sim(1, wire.W4007, wire.SQB_LC, wire.NOSQB);
		lc[2]->sim(2, wire.W400B, wire.TRI_LC, wire.NOTRI);
		lc[3]->sim(3, wire.W400F, wire.RND_LC, wire.NORND);

		square[0]->sim(wire.W4000, wire.W4001, wire.W4002, wire.W4003, wire.NOSQA, SQA_Out);
		square[1]->sim(wire.W4004, wire.W4005, wire.W4006, wire.W4007, wire.NOSQB, SQB_Out);
		tri->sim();
		noise->sim();
	}

	TriState APU::GetDBBit(size_t n)
	{
		TriState DBBit = (DB & (1 << n)) != 0 ? TriState::One : TriState::Zero;
		return DBBit;
	}

	void APU::SetDBBit(size_t n, TriState bit_val)
	{
		if (bit_val != TriState::Z)
		{
			uint8_t out = DB & ~(1 << n);
			out |= (bit_val == BaseLogic::One ? 1 : 0) << n;
			DB = out;
		}
	}

	void APU::SetRAWOutput(bool enable)
	{
		dac->SetRAWOutput(enable);
	}

	void APU::SetNormalizedOutput(bool enable)
	{
		dac->SetNormalizedOutput(enable);
	}

	size_t APU::GetACLKCounter()
	{
		return aclk_counter;
	}

	void APU::ResetACLKCounter()
	{
		aclk_counter = 0;
	}

	size_t APU::GetPHICounter()
	{
		return phi_counter;
	}

	void APU::ResetPHICounter()
	{
		phi_counter = 0;
	}

	void APU::ResetCore(bool enable)
	{
		wire.RESCore = enable ? TriState::One : TriState::Zero;
	}

	void APU::GetSignalFeatures(AudioSignalFeatures& features)
	{
		size_t clk = 21477272;	// Hz
		size_t div = 12;

		features.SampleRate = (int32_t)(clk * 2);

		// TBD: Add other APU
	}
}
