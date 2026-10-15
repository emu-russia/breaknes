#include "pch.h"
#include <cstring>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdlib>
#endif

using namespace BaseLogic;

namespace APUSimFast
{
	static const Log::LogCategoryDesc ApuLogCategories[] =
	{
		{ Cat_Regs, "Regs" },		// APU register accesses ($4000-$401F)
		{ Cat_DMA, "DMA" },			// OAM DMA activity
		{ Cat_Events, "Events" },	// reset / IRQ events
	};

	const Log::LogCategoryDesc* GetLogCategories(size_t& count)
	{
		count = sizeof(ApuLogCategories) / sizeof(ApuLogCategories[0]);
		return ApuLogCategories;
	}

	void APU::SetLogMask(uint64_t mask)
	{
		Log::SetCategoryMask(Log::Source_APU, mask);
	}

	APU::APU(M6502Core::M6502* _core, Revision _rev, ChipFeature features)
		: core_binding(this), dma_block(this), pads_block(this)
	{
		// For ease of integration, the core instance is created by the consumer
		core = _core;
		rev = _rev;
		fx = features;

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
		dac = new DAC(this);

		wire.RDY2 = TriState::One;
	}

	APU::~APU()
	{
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
		delete dac;
	}

	void APU::sim(TriState inputs[], TriState outputs[], uint8_t* data, uint16_t* addr, AudioOutSignal& AUX)
	{
#ifdef FASTSIM_CALL_CLASSES
		FastSimStats::apu_flags = 0;
#endif
		pads_block.sim_InputPads(inputs);
		pads_block.sim_DataBusInput(data);

		// v4 idle-call class: see sim_IdleCall; FASTSIM_SKIP_ORACLE builds verify it. Public
		// mutations disarm the path directly (debug.cpp, dac.cpp), so no epoch compare is needed.
		if (idle_armed && wire.RES == TriState::Zero && wire.DBG == TriState::Zero && !core_binding.PeekPhi0Edge())
		{
			sim_IdleCall(outputs, data, addr, AUX);
			return;
		}
		sim_FullCall(outputs, data, addr, AUX);
	}

	void APU::sim_FullCall(TriState outputs[], uint8_t* data, uint16_t* addr, AudioOutSignal& AUX)
	{
		dpcm->sim();

		dma_block.sim();
		dma_block.sim_DMA_Buffer();
		dma_block.sim_AddressMux();

		sim_CoreIntegration();

		sim_SoundGenerators();
#ifdef FASTSIM_CALL_CLASSES
		{
			const auto& w = wire;
			const bool write = ((w.W4000 | w.W4001 | w.W4002 | w.W4003 | w.W4004 | w.W4005 | w.W4006 | w.W4007 |
				w.W4008 | w.W400A | w.W400B | w.W400C | w.W400E | w.W400F | w.W4010 | w.W4011 | w.W4012 |
				w.W4013 | w.W4014 | w.W4015 | w.W4016 | w.W4017 | w.W401A) & 1) != 0;
			const bool read = ((w.n_R4015 & w.n_R4016 & w.n_R4017 & w.n_R4018 & w.n_R4019 & w.n_R401A) & 1) == 0;
			FastSimStats::apu_classes[((FastSimStats::apu_flags & 1) ? 1 : 0) | ((write || read) ? 2 : 0) |
				(w.RES == TriState::One ? 4 : 0) | (w.DBG == TriState::One ? 8 : 0) |
				((FastSimStats::apu_flags & 30) ? 16 : 0)]++;
		}
#endif

		pads_block.sim_OutputPads(outputs, addr);
		pads_block.sim_DataBusOutput(data);
		dac->sim(AUX);

		// Arm the idle path for the next call (plan N2-N4; review claim D repaired: settledness
		// of every guarded block is part of the predicate, so multi-call latch propagation still
		// takes the full path). Round 2: the blocks consume the clocks (from CLKGen) and the
		// strobes (from the decoder) one call later than they are produced, and sim_DMA_Buffer
		// derives RW/RD/WR from the decoder's n_R4015/n_DBGRD of the previous call; a call whose
		// evaluation moved ACLK1/nACLK2 or changed a strobe therefore leaves those consumers one
		// call behind, and the next call must be a full call. (Before the fixed-point settledness
		// of 3aed905 such a call always also left a block unsettled, which armed nothing.)
		idle_armed = !core_binding.core_called && !AnyStrobe() && wire.RES == TriState::Zero &&
			wire.DBG == TriState::Zero && dpcm->IsStable() && dma_block.IsStable() && clkgen->IsStable() && regs->IsDecoded() &&
			!clkgen->ClockChanged() && !regs->StrobesChanged();
		// While the idle class holds, no pad except M2 and no audio value can change (see
		// sim_IdleCall): keep what this call published and re-drive it on idle calls.
		if (idle_armed)
		{
			std::memcpy(idle_pads, outputs, sizeof(idle_pads));
			idle_audio_raw = dac->raw_mode;
			idle_aux_a = AUX.AUX.a;
			idle_aux_b = AUX.AUX.b;
		}
	}

	bool APU::AnyStrobe() const
	{
		const auto& w = wire;
		const bool write = ((w.W4000 | w.W4001 | w.W4002 | w.W4003 | w.W4004 | w.W4005 | w.W4006 | w.W4007 |
			w.W4008 | w.W400A | w.W400B | w.W400C | w.W400E | w.W400F | w.W4010 | w.W4011 | w.W4012 |
			w.W4013 | w.W4014 | w.W4015 | w.W4016 | w.W4017 | w.W401A) & 1) != 0;
		const bool read = ((w.n_R4015 & w.n_R4016 & w.n_R4017 & w.n_R4018 & w.n_R4019 & w.n_R401A) & 1) == 0;
		return write || read;
	}

	void APU::sim_IdleCall(TriState outputs[], uint8_t* data, uint16_t* addr, AudioOutSignal& AUX)
	{
		// Exactness argument: with no PHI0 edge this call the core is not invoked and PHI1/PHI2/
		// ACLK1/nACLK2 keep their values, so every guarded block's dependency key is unchanged (the
		// arming call changed neither the clocks nor a strobe after the blocks ran, see
		// sim_FullCall) and, being settled, it would be skipped; no read strobe is active, so no
		// live bus re-drive is owed; the address mux inputs are constant; the sound generators run
		// on PHI0 edges only; the debug lock latch holds when W401A is low. What still moves: the
		// master divider (PHI0 unchanged, n_M2 may toggle), the OAM DMA sample buffer capture on
		// PHI2, the data bus (sampled when RD, driven when WR or SPR_PPU), the M2 pad and the audio
		// object. What provably holds: RW/RD/WR (their inputs SPR_PPU, RnW, n_R4015, n_DBGRD come
		// from blocks that are not evaluated and did not change after the arming call derived
		// them), the address (Ax is driven while RES is low), the six
		// other pads (RnW from RW/RES, n_IN0/1 from the read strobes, OUT_0..2 from the GPIO
		// output latch, which the arming call already copied from the unchanged register under
		// the same nACLK2), PHI0 and the PHI counter (PrevPHI_Core == PHI0 after every full call,
		// and PeekPhi0Edge established that the stepped divider yields that same phase).
#ifdef FASTSIM_CALL_CLASSES
		FastSimStats::apu_classes[0]++;
#endif
		// CoreBinding::sim_DividerBeforeCore without the inert PHI0 store and falling-edge counter
		// test, then sim_DividerAfterCore (n_M2).
		if (wire.n_CLK == TriState::Zero)
		{
			const uint8_t output = core_binding.divider_output;
			uint8_t input = uint8_t(((output << 1) & 0x3f) | ((output & 0x20) == 0));
			if ((output & 0x20) != 0 && (output & 0x10) == 0) input &= uint8_t(~0x0e);
			core_binding.divider_input = input;
		}
		else
		{
			core_binding.divider_output = core_binding.divider_input;
		}
		wire.n_M2 = NOR((core_binding.divider_input & 0x10) != 0 ? TriState::Zero : TriState::One, wire.PHI2);

		std::memcpy(outputs, idle_pads, sizeof(idle_pads));
		// M2 = NOR(n_M2, NotDBG_RES), and NotDBG_RES = NOR(DBG, NOT(RES)) is Zero while DBG and RES are low.
		outputs[(size_t)APU_Output::M2] = NOR(wire.n_M2, TriState::Zero);
		*addr = Ax;

		// DMA::sim_DMA_Buffer with RW/RD/WR retained: SPR_PPU, RnW, n_R4015 and n_DBGRD are produced
		// by blocks an idle call does not evaluate, so RW = NOR(SPR_PPU, NOT(RnW)), RD = RW and
		// WR = NAND3(n_R4015, n_DBGRD, RW) keep the values of the arming call.
		if (wire.PHI2 == TriState::One) dma_block.spr_buf = DB;
		if (wire.SPR_PPU) DB = dma_block.spr_buf;

		// Pads::sim_DataBusOutput
		if (wire.WR == TriState::One) *data = pads_block.output_byte = DB;

		// DAC::sim with the sample the arming call published: audio_word only changes in blocks an
		// idle call does not evaluate, and the output-mode setters disarm the idle class.
		if (idle_audio_raw)
		{
			const uint32_t word = audio_word;
			AUX.RAW.sqa = word & 15;
			AUX.RAW.sqb = (word >> 4) & 15;
			AUX.RAW.tri = (word >> 8) & 15;
			AUX.RAW.rnd = (word >> 12) & 15;
			AUX.RAW.dmc = (word >> 16) & 127;
		}
		else
		{
			AUX.AUX.a = idle_aux_a;
			AUX.AUX.b = idle_aux_b;
		}
#ifdef FASTSIM_SKIP_ORACLE
		sim_IdleOracle(outputs, data, addr, AUX);
#endif
	}

#ifdef FASTSIM_SKIP_ORACLE
	// Re-run the skipped stages and abort if any of them would have changed state or output.
	void APU::sim_IdleOracle(TriState outputs[], uint8_t* data, uint16_t* addr, AudioOutSignal& AUX)
	{
		InternalWires before_wires = wire;
		const uint16_t ax = Ax, cpu_addr = CPU_Addr, spr = SPR_Addr, dmc = DMC_Addr;
		const uint8_t db = DB; const uint32_t word = audio_word;
		const size_t aclk = aclk_counter, phi = phi_counter;
		TriState check_outputs[(size_t)APU_Output::Max]; std::memcpy(check_outputs, outputs, sizeof(check_outputs));
		uint8_t check_data = *data; uint16_t check_addr = *addr; AudioOutSignal check_aux = AUX;
		dpcm->sim(); dma->sim(); dma->sim_DMA_Buffer(); dma->sim_AddressMux();
		clkgen->sim(); regs->sim(); regs->sim_DebugRegisters(); sim_SoundGenerators();
		pads->sim_OutputPads(check_outputs, &check_addr); pads->sim_DataBusOutput(&check_data); dac->sim(check_aux);
		const bool same = std::memcmp(&before_wires, &wire, sizeof(wire)) == 0 && ax == Ax && cpu_addr == CPU_Addr &&
			spr == SPR_Addr && dmc == DMC_Addr && db == DB && word == audio_word && aclk == aclk_counter && phi == phi_counter &&
			std::memcmp(check_outputs, outputs, sizeof(check_outputs)) == 0 && check_data == *data && check_addr == *addr &&
			std::memcmp(&check_aux, &AUX, sizeof(AUX)) == 0;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: APU idle class violated (Ax %04x->%04x DB %02x->%02x word %06x->%06x)\n",
				ax, Ax, db, DB, word, audio_word);
			std::abort();
		}
	}
#endif

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

		// Each channel returns its four output bits; the original channel order is kept.
		const uint32_t sqa = square[0]->sim(wire.W4000, wire.W4001, wire.W4002, wire.W4003, wire.NOSQA);
		const uint32_t sqb = square[1]->sim(wire.W4004, wire.W4005, wire.W4006, wire.W4007, wire.NOSQB);
		const uint32_t tri_out = tri->sim();
		const uint32_t rnd = noise->sim();
		audio_word = (audio_word & 0x7f0000u) | sqa | (sqb << 4) | (tri_out << 8) | (rnd << 12);
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

	void APU::GetSignalFeatures(AudioSignalFeatures& features)
	{
		size_t clk = 21477272;	// Hz
		size_t div = 12;

		features.SampleRate = (int32_t)(clk * 2);

		// TBD: Add other APU
	}

	TriState APU::GetPHI2()
	{
		return wire.PHI2;
	}

	TriState APU::GetPHI0()
	{
		return wire.PHI0;
	}

	TriState APU::GetSYNC()
	{
		return wire.SYNC;
	}
}
