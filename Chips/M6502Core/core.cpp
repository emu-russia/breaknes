#include "pch.h"
#include <type_traits>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#endif

using namespace BaseLogic;

#ifdef M6502_PASS2_STATS
#include <cstdio>
#include <cstring>
#include <vector>
namespace M6502Core
{
	namespace
	{
		constexpr size_t Pass2Objects = 19;
		const char* const Pass2Names[Pass2Objects] = { "core", "predecode", "ir", "ext", "brk", "disp", "random",
			"regs_control", "alu_control", "pc_control", "bus_control", "flags_control", "flags", "branch_logic",
			"addr_bus", "regs", "alu", "pc", "data_bus" };
		const char* const WireFieldNames[] = { "n_NMI", "n_IRQ", "n_RES", "PHI0", "RDY", "SO", "PHI1", "PHI2", "n_PRDY", "n_NMIP", "n_IRQP", "RESP", "n_ready", "T0", "FETCH", "Z_IR", "ACRL1", "ACRL2", "n_T0", "n_T1X", "WR", "RMW_T6", "RMW_T7", "n_1PC", "ENDS", "ENDX", "TRES1", "n_TRESX", "n_T2", "n_T3", "n_T4", "n_T5", "n_IMPLIED", "n_TWOCYCLE", "BRK6E", "BRK7", "DORES", "n_DONMI", "BRK5_RDY", "B_OUT", "BRFW", "n_BRTAKEN", "PC_DB", "n_ADL_PCL", "n_IR5" };
		const char* const DispatcherLatchNames[] = { "acr_latch1", "acr_latch2", "t67_latch", "t6_latch1", "t6_latch2", "t7_latch1", "t7_latch2", "tres2_latch", "tresx_latch1", "tresx_latch2", "fetch_latch", "wr_latch", "ready_latch1", "ready_latch2", "ends_latch1", "ends_latch2", "nready_latch", "step_latch1", "step_latch2", "t1_latch", "comp_latch1", "comp_latch2", "comp_latch3", "rdydelay_latch1", "rdydelay_latch2", "t0_latch", "t1x_latch", "br_latch1", "br_latch2", "ipc_latch1", "ipc_latch2", "ipc_latch3" };
		constexpr size_t WireFieldCount = sizeof(WireFieldNames) / sizeof(WireFieldNames[0]);
		constexpr size_t DispatcherLatchCount = sizeof(DispatcherLatchNames) / sizeof(DispatcherLatchNames[0]);
		struct Pass2Stats
		{
			uint64_t calls[2]{}, changed[2]{}, external_changed[2]{};
			uint64_t objects[2][Pass2Objects]{};
			// v4 track-cpu goal 1: separate the second Top traversal from the second Bottom traversal,
			// and test the Bottom2 replay predicate ("Bottom2 reads exactly what Bottom1 read").
			uint64_t top2_changed[2]{}, bottom2_changed[2]{}, bottom2_objects[2][Pass2Objects]{};
			uint64_t inputs_equal_ex_buses[2]{}, inputs_equal_incl_buses[2]{};
			uint64_t bottom2_changed_inputs_equal_ex_buses[2]{}, bottom2_changed_inputs_equal_incl_buses[2]{};
			uint64_t bottom2_core_bytes[2][1024]{};
			uint64_t bottom2_skipped[2]{};	// PHI1 second Bottom traversals skipped by the replay predicate
			// Decoder key reachability: timing patterns and distinct (opcode, timing) keys selected by pass 2.
			uint64_t txbits_hist[64]{};
			uint64_t keys_seen[256]{};
			// Per-byte change histograms for the core object (named through the wire struct) and the dispatcher.
			uint64_t core_bytes[2][1024]{};
			uint64_t disp_bytes[2][256]{};
			size_t wire_offset = 0, wire_size = 0, core_size = 0, disp_size = 0;
			const char* CoreByteName(size_t offset, char* scratch, size_t scratch_size) const
			{
				if (offset >= wire_offset && offset < wire_offset + wire_size && offset - wire_offset < WireFieldCount)
					return WireFieldNames[offset - wire_offset];
				std::snprintf(scratch, scratch_size, "core+%zu", offset);
				return scratch;
			}
			~Pass2Stats()
			{
				std::fprintf(stderr, "{\"m6502_pass2_stats\":{");
				for (unsigned phi = 0; phi < 2; phi++)
				{
					std::fprintf(stderr, "%s\"phi%u\":{\"calls\":%llu,\"state_changed_by_pass2\":%llu,\"external_changed_by_pass2\":%llu,\"objects\":{",
						phi ? "," : "", phi + 1, (unsigned long long)calls[phi], (unsigned long long)changed[phi], (unsigned long long)external_changed[phi]);
					bool first = true;
					for (size_t n = 0; n < Pass2Objects; n++) if (objects[phi][n])
					{
						std::fprintf(stderr, "%s\"%s\":%llu", first ? "" : ",", Pass2Names[n], (unsigned long long)objects[phi][n]);
						first = false;
					}
					std::fprintf(stderr, "},\"core_bytes\":{");
					first = true; char scratch[32];
					for (size_t n = 0; n < core_size && n < 1024; n++) if (core_bytes[phi][n])
					{
						std::fprintf(stderr, "%s\"%s\":%llu", first ? "" : ",", CoreByteName(n, scratch, sizeof(scratch)), (unsigned long long)core_bytes[phi][n]);
						first = false;
					}
					std::fprintf(stderr, "},\"disp_bytes\":{");
					first = true;
					for (size_t n = 0; n < disp_size && n < 256; n++) if (disp_bytes[phi][n])
					{
						// Dispatcher layout: M6502* core (8 bytes) then DLatch members (1 byte each) in declaration order.
						if (n >= 8 && n - 8 < DispatcherLatchCount) std::fprintf(stderr, "%s\"%s\":%llu", first ? "" : ",", DispatcherLatchNames[n - 8], (unsigned long long)disp_bytes[phi][n]);
						else std::fprintf(stderr, "%s\"disp+%zu\":%llu", first ? "" : ",", n, (unsigned long long)disp_bytes[phi][n]);
						first = false;
					}
					std::fprintf(stderr, "},\"bottom2_skipped\":%llu,\"top2_changed\":%llu,\"bottom2_changed\":%llu,\"bottom2_inputs_equal_ex_buses\":%llu,\"bottom2_inputs_equal_incl_buses\":%llu,"
						"\"bottom2_changed_while_inputs_equal_ex_buses\":%llu,\"bottom2_changed_while_inputs_equal_incl_buses\":%llu,\"bottom2_objects\":{",
						(unsigned long long)bottom2_skipped[phi], (unsigned long long)top2_changed[phi], (unsigned long long)bottom2_changed[phi], (unsigned long long)inputs_equal_ex_buses[phi],
						(unsigned long long)inputs_equal_incl_buses[phi], (unsigned long long)bottom2_changed_inputs_equal_ex_buses[phi],
						(unsigned long long)bottom2_changed_inputs_equal_incl_buses[phi]);
					first = true;
					for (size_t n = 0; n < Pass2Objects; n++) if (bottom2_objects[phi][n])
					{
						std::fprintf(stderr, "%s\"%s\":%llu", first ? "" : ",", Pass2Names[n], (unsigned long long)bottom2_objects[phi][n]);
						first = false;
					}
					std::fprintf(stderr, "},\"bottom2_core_bytes\":{");
					first = true;
					for (size_t n = 0; n < core_size && n < 1024; n++) if (bottom2_core_bytes[phi][n])
					{
						std::fprintf(stderr, "%s\"%s\":%llu", first ? "" : ",", CoreByteName(n, scratch, sizeof(scratch)), (unsigned long long)bottom2_core_bytes[phi][n]);
						first = false;
					}
					std::fprintf(stderr, "}}");
				}
				std::fprintf(stderr, ",\"txbits_hist\":{");
				bool first = true;
				for (unsigned n = 0; n < 64; n++) if (txbits_hist[n])
				{
					std::fprintf(stderr, "%s\"%u\":%llu", first ? "" : ",", n, (unsigned long long)txbits_hist[n]);
					first = false;
				}
				uint64_t distinct_keys = 0;
				for (unsigned n = 0; n < 256; n++) for (uint64_t w = keys_seen[n]; w; w &= w - 1) distinct_keys++;
				std::fprintf(stderr, "},\"distinct_opcode_timing_keys\":%llu", (unsigned long long)distinct_keys);
				std::fprintf(stderr, "}}\n");
			}
		} pass2_stats;
	}

	std::vector<std::vector<uint8_t>> M6502::SnapshotState() const
	{
		std::vector<std::vector<uint8_t>> result;
		auto add = [&](const void* object, size_t size) {
			const auto* bytes = static_cast<const uint8_t*>(object);
			result.emplace_back(bytes, bytes + size);
		};
		add(this, sizeof(*this)); add(predecode, sizeof(*predecode)); add(ir, sizeof(*ir)); add(ext, sizeof(*ext));
		add(brk, sizeof(*brk)); add(disp, sizeof(*disp)); add(random, sizeof(*random));
		add(random->regs_control, sizeof(*random->regs_control)); add(random->alu_control, sizeof(*random->alu_control));
		add(random->pc_control, sizeof(*random->pc_control)); add(random->bus_control, sizeof(*random->bus_control));
		add(random->flags_control, sizeof(*random->flags_control)); add(random->flags, sizeof(*random->flags));
		add(random->branch_logic, sizeof(*random->branch_logic));
		add(addr_bus, sizeof(*addr_bus)); add(regs, sizeof(*regs)); add(alu, sizeof(*alu)); add(pc, sizeof(*pc)); add(data_bus, sizeof(*data_bus));
		return result;
	}
}
#endif

namespace M6502Core
{
	static const Log::LogCategoryDesc CoreLogCategories[] =
	{
		{ Cat_Events, "Events" },	// RES / NMI / IRQ edge events
		{ Cat_Bus, "Bus" },			// external bus write cycles
	};

	const Log::LogCategoryDesc* GetLogCategories(size_t& count)
	{
		count = sizeof(CoreLogCategories) / sizeof(CoreLogCategories[0]);
		return CoreLogCategories;
	}

	void M6502::SetLogMask(uint64_t mask)
	{
		Log::SetCategoryMask(Log::Source_Core, mask);
	}

	M6502::M6502(bool HLE, bool BCD_Hack) : M6502(HLE, BCD_Hack, ExecutionBackend::Reference) {}

	M6502::M6502(bool HLE, bool BCD_Hack, ExecutionBackend backend)
	{
		execution_backend = backend == ExecutionBackend::ExactPacked && HLE && BCD_Hack ?
			ExecutionBackend::ExactPacked : ExecutionBackend::Reference;
		HLE_Mode = HLE;

		if (execution_backend == ExecutionBackend::Reference) decoder = new Decoder;
		else (void)PackedDecoder::Lookup(0, 0);
		predecode = new PreDecode(this);
		ir = new IR(this);
		ext = new ExtraCounter(this);
		brk = new BRKProcessing(this);
		disp = new Dispatcher(this);
		random = new RandomLogic(this);

		addr_bus = new AddressBus(this);
		regs = new Regs(this);
		alu = new ALU(this);
		alu->SetBCDHack(BCD_Hack);
		pc = new ProgramCounter(this, HLE_Mode);
		data_bus = new DataBus(this);
	}

	M6502::~M6502()
	{
		delete decoder;
		delete predecode;
		delete ir;
		delete ext;
		delete brk;
		delete disp;
		delete random;

		delete addr_bus;
		delete regs;
		delete alu;
		delete pc;
		delete data_bus;
	}

	void M6502::sim_Top(TriState inputs[], uint8_t* data_bus)
	{
		if (execution_backend == ExecutionBackend::ExactPacked) sim_TopImpl<true>(inputs,data_bus);
		else sim_TopImpl<false>(inputs,data_bus);
	}

	template<bool Packed, int Phase> void M6502::sim_TopImpl(TriState inputs[], uint8_t* data_bus, bool second_pass)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		wire.n_NMI = inputs[(size_t)InputPad::n_NMI];
		wire.n_IRQ = inputs[(size_t)InputPad::n_IRQ];
		wire.n_RES = inputs[(size_t)InputPad::n_RES];
		wire.PHI0 = inputs[(size_t)InputPad::PHI0];
		wire.RDY = inputs[(size_t)InputPad::RDY];
		wire.SO = inputs[(size_t)InputPad::SO];

		// Logic associated with external input terminals

		wire.PHI1 = NOT(wire.PHI0);
		wire.PHI2 = wire.PHI0;

		P::Set2(prdy_latch1, NOT(wire.RDY), wire.PHI2);
		P::Set1(prdy_latch2, prdy_latch1.nget(), wire.PHI1);
		wire.n_PRDY = prdy_latch2.nget();

		if (P::Phi2One(wire.PHI2) && nNMI_Cache != wire.n_NMI)
		{
			nmip_ff.set(NOR(NOR(nmip_ff.get(), AND(NOT(wire.n_NMI), wire.PHI2)), AND(wire.n_NMI, wire.PHI2)));
			nNMI_Cache = wire.n_NMI;
			LOG_CORE(Cat_Events, "NMI %s", wire.n_NMI == TriState::Zero ? "asserted" : "deasserted");
		}
		wire.n_NMIP = NOT(nmip_ff.get());

		if (P::Phi2One(wire.PHI2) && nIRQ_Cache != wire.n_IRQ)
		{
			irqp_ff.set(NOR(NOR(irqp_ff.get(), AND(NOT(wire.n_IRQ), wire.PHI2)), AND(wire.n_IRQ, wire.PHI2)));
			nIRQ_Cache = wire.n_IRQ;
			LOG_CORE(Cat_Events, "IRQ %s", wire.n_IRQ == TriState::Zero ? "asserted" : "deasserted");
		}
		P::Set1(irqp_latch, irqp_ff.get(), wire.PHI1);
		wire.n_IRQP = irqp_latch.nget();

		if (P::Phi2One(wire.PHI2) && nRES_Cache != wire.n_RES)
		{
			resp_ff.set(NOR(NOR(resp_ff.get(), AND(wire.n_RES, wire.PHI2)), AND(NOT(wire.n_RES), wire.PHI2)));
			nRES_Cache = wire.n_RES;
			LOG_CORE(Cat_Events, "RESET %s", wire.n_RES == TriState::Zero ? "asserted" : "deasserted");
		}
		P::Set1(resp_latch, resp_ff.get(), wire.PHI1);
		wire.RESP = resp_latch.nget();

		// Dispatcher and other auxiliary logic

		disp->sim_BeforeDecoderImpl<Phase>();

		// Packed: the Top blocks that test the phase at run time take it as the compile-time Phase.
		if constexpr (Packed) predecode->simImpl<Phase>(data_bus);
		else predecode->sim(data_bus);

		if constexpr (Packed) ir->simImpl<Phase>();
		else ir->sim();

		if constexpr (Packed) ext->sim_HLEImpl<Phase>();
		else if (HLE_Mode)
		{
			ext->sim_HLE();
		}
		else
		{
			ext->sim();
		}

		TxBits = 0;
		TxBits |= ((size_t)wire.n_T0 << 0);
		TxBits |= ((size_t)wire.n_T1X << 1);
		TxBits |= ((size_t)wire.n_T2 << 2);
		TxBits |= ((size_t)wire.n_T3 << 3);
		TxBits |= ((size_t)wire.n_T4 << 4);
		TxBits |= ((size_t)wire.n_T5 << 5);

		if constexpr (Packed)
		{
			wire.n_IR5 = (ir->IROut & 32) ? Zero : One;
			packed_decoder = &PackedDecoder::Lookup(ir->IROut, static_cast<uint8_t>(TxBits));
		}
		else
		{
			DecoderInput decoder_in{};

			uint8_t IR = ir->IROut;

			TriState IR0 = IR & 0b00000001 ? TriState::One : TriState::Zero;
			TriState IR1 = IR & 0b00000010 ? TriState::One : TriState::Zero;
			TriState IR2 = IR & 0b00000100 ? TriState::One : TriState::Zero;
			TriState IR3 = IR & 0b00001000 ? TriState::One : TriState::Zero;
			TriState IR4 = IR & 0b00010000 ? TriState::One : TriState::Zero;
			TriState IR5 = IR & 0b00100000 ? TriState::One : TriState::Zero;
			TriState IR6 = IR & 0b01000000 ? TriState::One : TriState::Zero;
			TriState IR7 = IR & 0b10000000 ? TriState::One : TriState::Zero;

			wire.n_IR5 = NOT(IR5);

			decoder_in.n_IR0 = NOT(IR0);
			decoder_in.n_IR1 = NOT(IR1);
			decoder_in.IR01 = OR(IR0, IR1);
			decoder_in.n_IR2 = NOT(IR2);
			decoder_in.IR2 = IR2;
			decoder_in.n_IR3 = NOT(IR3);
			decoder_in.IR3 = IR3;
			decoder_in.n_IR4 = NOT(IR4);
			decoder_in.IR4 = IR4;
			decoder_in.n_IR5 = wire.n_IR5;
			decoder_in.IR5 = IR5;
			decoder_in.n_IR6 = NOT(IR6);
			decoder_in.IR6 = IR6;
			decoder_in.n_IR7 = NOT(IR7);
			decoder_in.IR7 = IR7;

			decoder_in.n_T0 = wire.n_T0;
			decoder_in.n_T1X = wire.n_T1X;
			decoder_in.n_T2 = wire.n_T2;
			decoder_in.n_T3 = wire.n_T3;
			decoder_in.n_T4 = wire.n_T4;
			decoder_in.n_T5 = wire.n_T5;


			decoder->sim(decoder_in.packed_bits, &decoder_out);
		}

		// Interrupt handling

		brk->sim_BeforeRandomImpl<Packed, Phase>();

		// Random Logic

		disp->sim_BeforeRandomLogicImpl<Packed, Phase>();

		random->sim_Impl<Packed, Phase>(second_pass);

		brk->sim_AfterRandomImpl<Packed, Phase>();

		disp->sim_AfterRandomLogicImpl<Packed, Phase>();
	}

	void M6502::sim_Bottom(TriState inputs[], TriState outputs[], uint16_t* ext_addr_bus, uint8_t* ext_data_bus)
	{
		if (execution_backend == ExecutionBackend::ExactPacked) sim_BottomImpl<true>(inputs,outputs,ext_addr_bus,ext_data_bus);
		else sim_BottomImpl<false>(inputs,outputs,ext_addr_bus,ext_data_bus);
	}

	template<bool Packed, int Phase> void M6502::sim_BottomImpl(TriState inputs[], TriState outputs[], uint16_t* ext_addr_bus, uint8_t* ext_data_bus)
	{
		static_assert(Packed || Phase == 0, "the reference schedule is the generic phase");
		using P = Phi<Phase>;
		// Bottom Part

		// When you simulate the lower part, you have to turn on the man in you to the fullest and imagine that you are possessed by Chuck Peddle.
		// From the development history of the 6502, we know that one developer did the upper part and another (Chuck) did the lower part. 
		// So to simulate the lower part you have to abstract as much as possible and work only with what you have (registers, buses and set of control commands from the upper part).

		// Packed: the datapath blocks that test the phase at run time take it as the compile-time Phase, and
		// the HLE/BCD-hack variants are called directly (ExactPacked requires both; the flags are set only by
		// the M6502 constructor). The generic Phase 0 keeps the run-time tests.

		// Bus load from DL: DL_DB, DL_ADL, DL_ADH

		if constexpr (Packed) data_bus->sim_GetExternalBusImpl<Phase>(ext_data_bus);
		else data_bus->sim_GetExternalBus(ext_data_bus);

		// Registers to the SB bus: Y_SB, X_SB, S_SB

		if constexpr (Packed) regs->sim_StoreSBImpl<Phase>();
		else regs->sim_StoreSB();

		// ADD saving on SB/ADL: ADD_SB7, ADD_SB06, ADD_ADL

		alu->sim_StoreADD();

		// Saving AC: AC_SB, AC_DB

		// BusControl zeros AC_SB/AC_DB during PHI2, so StoreAC has no effect.
		if constexpr (Packed) { if (!P::Phi2One(wire.PHI2)) alu->sim_StoreAC(); }
		else alu->sim_StoreAC();

		// Saving flags on DB bus: P_DB

		random->flags->sim_Store();

		// Stack pointer saving on ADL bus: S_ADL

		regs->sim_StoreOldS();

		// Increment PC: n_1PC

		// The packed backend requires HLE; its PC increment stores only in PHI2.
		if constexpr (Packed) { if (P::Phi2One(wire.PHI2)) pc->sim_HLEImpl<Phase>(); }
		else pc->sim();

		// Saving PC to buses: PCL_ADL, PCH_ADH, PCL_DB, PCH_DB

		if constexpr (Packed) pc->sim_StoreHLE();
		else pc->sim_Store();

		// Bus multiplexing: SB_DB, SB_ADH

		alu->sim_BusMux();

		// Constant generator: Z_ADL0, Z_ADL1, Z_ADL2, Z_ADH0, Z_ADH17

		addr_bus->sim_ConstGen();

		// Loading ALU operands: NDB_ADD, DB_ADD, Z_ADD, SB_ADD, ADL_ADD

		// ALUControl zeros all five operand-load commands during PHI2.
		if constexpr (Packed) { if (!P::Phi2One(wire.PHI2)) alu->sim_Load(); }
		else alu->sim_Load();

		// ALU operation: ANDS, EORS, ORS, SRS, SUMS, n_ACIN, n_DAA, n_DSA
		// BCD correction via SB bus: SB_AC

		if constexpr (Packed) alu->sim_HLEImpl<Phase>();
		else if (HLE_Mode)
		{
			alu->sim_HLE();
		}
		else
		{
			alu->sim();
		}

		// Load flags: DB_P, DBZ_Z, DB_N, IR5_C, DB_C, IR5_D, IR5_I, DB_V, Z_V, ACR_C, AVR_V

		random->flags->sim_LoadImpl<Phase>();

		// Load registers: SB_X, SB_Y, SB_S / S_S

		// In PHI2 LoadSB only repeats S_out=~S_in already done by StoreSB;
		// no intervening PHI2 block changes S_in.
		if constexpr (Packed) { if (!P::Phi2One(wire.PHI2)) regs->sim_LoadSBImpl<Phase>(); }
		else regs->sim_LoadSB();

		// PC loading from buses / keep: ADH_PCH/PCH_PCH, ADL_PCL/PCL_PCL

		// PC_Control zeros all four PC load/refresh commands during PHI2.
		if constexpr (Packed) { if (!P::Phi2One(wire.PHI2)) pc->sim_LoadHLE(); }
		else pc->sim_Load();

		// Saving DB to DOR

		if constexpr (Packed) data_bus->sim_SetExternalBusImpl<Phase>(ext_data_bus);
		else data_bus->sim_SetExternalBus(ext_data_bus);

		// Set external address bus: ADH_ABH, ADL_ABL

		if constexpr (Packed) addr_bus->sim_OutputImpl<Phase>(ext_addr_bus);
		else addr_bus->sim_Output(ext_addr_bus);

		// Outputs

		P::Set1(rw_latch, wire.WR, wire.PHI1);

		outputs[(size_t)OutputPad::PHI1] = wire.PHI1;
		outputs[(size_t)OutputPad::PHI2] = wire.PHI2;
		outputs[(size_t)OutputPad::RnW] = rw_latch.nget();
		outputs[(size_t)OutputPad::SYNC] = disp->getT1();
	}

	M6502::BottomControls M6502::CaptureBottomControls()
	{
		return BottomControls{ cmd.raw, wire.WR, disp->getT1(), wire.BRK6E, brk->getB_OUT(wire.BRK6E) };
	}

	// The PHI1 Bottom traversal is a network of bus drivers ("replace when clean, AND when dirty",
	// the ADD->SB bit replacement, the constant generator's zeroing), the two bus-connect muxes,
	// register loads from the buses and the flag loads. Re-applying it with identical controls to
	// its own result is the identity except through these write-then-read paths:
	//   1. P_DB drives the flags onto DB (replacing bits other than 5): the flag loads later in the
	//      same traversal rewrite every flag latch (from a load command, or from the hold term fed by
	//      latch2/avr_latch/vset_latch/the BRK6E-gated I latch2, which differs from latch1 after a
	//      public setter or an interrupt sequence), so the second traversal would push other flags;
	//   2. SB/ADH connects SB to ADH and the constant generator then zeroes ADH bits (0/ADH0, 0/ADH17);
	//   3. SB/DB and SB/ADH are both active (the three-bus chain converges over two traversals);
	//   4. SB/ADD together with 0/ADD clears SB after the muxes (the power-up special case).
	// The register feedbacks Y/X/AC -> SB -> Y/X/AC are absorbed by the AND rule (the reloaded value
	// is the bus value itself); DL, S_out, ADD, PC, the ALU result and all PHI2 latches are not
	// written in PHI1; DOR and AB are rewritten with the same values; the external data bus is not
	// driven in PHI1 (RD is high). With the four patterns excluded and the controls unchanged by the
	// second Top traversal, the second Bottom traversal rewrites every bus, register, latch, output
	// and external bus with the value it already holds.
	bool M6502::Phi1BottomIdempotent(uint64_t c)
	{
		constexpr uint64_t P_DB = UINT64_C(1) << 47;
		constexpr uint64_t SB_DB = UINT64_C(1) << 42, SB_ADH = UINT64_C(1) << 43;
		constexpr uint64_t Z_ADH = (UINT64_C(1) << 40) | (UINT64_C(1) << 41);
		constexpr uint64_t SB_ADD = UINT64_C(1) << 11, Z_ADD = UINT64_C(1) << 10;
		if (c & P_DB) return false;
		if ((c & SB_ADH) && (c & Z_ADH)) return false;
		if ((c & SB_DB) && (c & SB_ADH)) return false;
		if ((c & SB_ADD) && (c & Z_ADD)) return false;
		return true;
	}

	// The second Top traversal of a PHI2 call. Its inputs are the pads (constant within the call), the
	// PHI1-enabled latches (frozen while PHI2 is high), the PHI2-enabled latches as the first traversal
	// left them, the decoder row (IR is PHI1-latched and the T-state cone is a fixed point of its own
	// recurrence: n_T0 = NOR(A, NOR(t0_latch, t1x)) gives the same value when fed back; n_T2..n_T5 are
	// the PHI1 shift register gated by the PHI1 tres2_latch) and what the Bottom traversal in between
	// changed in Top's view: ACR/AVR (the ALU recomputed), and the external data byte on a write cycle.
	// Walking the first traversal's writes: every PHI2 latch is rewritten with the value it already
	// holds (its inputs are frozen or settled by the first write: ready_latch1, rdydelay_latch2,
	// acr_latch2, t0_latch, fetch_latch, the BRK latches - ff1/ff2 are fixed points of their own
	// recurrence -, t67/t6/t7, br_latch1/2, step_latch1, ends_latch1/2, tresx_latch1, wr_latch,
	// br2/brfw_latch2, b_latch1, the five command evaluators - round 1's replay proof -, the
	// ExtraCounter HLE words, the pad flip-flops behind their change caches), and every PHI1 latch
	// enable is low. The exceptions, all reads of state that the first traversal rewrote AFTER reading
	// it, or of ACR:
	//   - sim_BeforeDecoder: ACRL1 (reads ACR and acr_latch2 before acr_latch2's rewrite), and the
	//     local BRK6E -> B_OUT -> Z_IR cone (reads brk6_latch2 before BRKProcessing rewrites it);
	//   - PreDecode: pd_latch re-latches the data byte (driven by the first Bottom traversal on a
	//     write cycle) and PD/n_PD/n_TWOCYCLE/n_IMPLIED follow Z_IR;
	//   - sim_AfterRandomLogic: ENDS (reads ends_latch1/2 before their rewrite), ipc_temp (ACR), hence
	//     n_TRES1/TRES1, tresx_latch2 (the one latch written with a possibly different value) and
	//     n_TRESX (ACRL1, tresx_latch2).
	// Everything else, wires included (n_1PC, WR, ENDX, T0, FETCH, BRK6E, B_OUT, BRFW, n_BRTAKEN,
	// RMW_T6/T7, the command word), is unchanged. FASTSIM_SKIP_ORACLE runs the full second traversal
	// from a snapshot of the same state and compares every byte of the core (sim_Phi2TopOracle).
	void M6502::sim_Phi2TopSettle(uint8_t* data_bus)
	{
		disp->sim_SettlePhi2BeforeDecoder();
		predecode->simImpl<2>(data_bus);
		disp->sim_SettlePhi2AfterRandomLogic();
	}

	// The second Top traversal of a PHI1 call. Its inputs are the pads, the PHI2-enabled latches
	// (frozen while PHI1 is high), the PHI1-enabled latches as the first traversal left them, and
	// what the Bottom traversal in between changed in Top's view: the flags' latch1 bank and the DB
	// bus (ACR is not recomputed in PHI1). The first traversal's late writes feed the T-state cone -
	// comp_latch1..3 and tres2_latch (n_T0, n_T2..n_T5), t1_latch (T1) - and, through DORES
	// (res_latch2), Z_IR, so the decoder row can re-select; everything downstream of the row re-runs
	// in full: the predecoder, IR, the ExtraCounter, the row lookup, the branch logic (which also
	// reads the new flags and DB7) and sim_AfterRandomLogic; the five command evaluators are round
	// 1's oracle-checked replay. The blocks that cannot differ are omitted:
	//   - the pads block: prdy_latch2, irqp_latch and resp_latch copy PHI2 state (prdy_latch1, the
	//     interrupt flip-flops change only while PHI2 is high);
	//   - sim_BeforeDecoder except its n_T0 and Z_IR cones (Dispatcher::sim_SettlePhi1BeforeDecoder):
	//     ready_latch2 <- WR with the settled DORES, which is what the first traversal's
	//     sim_AfterRandomLogic already wrote; rdydelay_latch1 <- n_ready; acr_latch1 <- ACRL1
	//     (ACR, rdydelay_latch2, acr_latch2 are unchanged); t1x_latch <- NOR(t0_latch, n_ready);
	//   - sim_BeforeRandom except BRK5_RDY/BRK7, donmi_latch and n_DONMI
	//     (BRKProcessing::sim_SettlePhi1BeforeRandom): brk6_latch1 and res_latch2 are fixed points
	//     of their recurrences over PHI2 state, brk6e_latch, nmip_latch and delay_latch2 copy
	//     unchanged signals, BRK6E/DORES/Z_ADL are unchanged;
	//   - sim_BeforeRandomLogic: t6_latch1 and t7_latch2 are functions of PHI2 latches and n_ready;
	//   - BRKProcessing::sim_AfterRandom: b_latch2 <- NOR(b_latch1, BRK6E) over unchanged inputs, and
	//     B_OUT is unchanged (int_set feeds only the PHI2-enabled b_latch1).
	// FASTSIM_SKIP_ORACLE runs the full second traversal from a snapshot of the same state and
	// compares every byte of the core (sim_Phi1TopOracle).
	void M6502::sim_Phi1TopSettle(uint8_t* data_bus)
	{
		disp->sim_SettlePhi1BeforeDecoder();

		predecode->simImpl<1>(data_bus);

		ir->simImpl<1>();

		ext->sim_HLEImpl<1>();

		TxBits = 0;
		TxBits |= ((size_t)wire.n_T0 << 0);
		TxBits |= ((size_t)wire.n_T1X << 1);
		TxBits |= ((size_t)wire.n_T2 << 2);
		TxBits |= ((size_t)wire.n_T3 << 3);
		TxBits |= ((size_t)wire.n_T4 << 4);
		TxBits |= ((size_t)wire.n_T5 << 5);

		wire.n_IR5 = (ir->IROut & 32) ? Zero : One;
		packed_decoder = &PackedDecoder::Lookup(ir->IROut, static_cast<uint8_t>(TxBits));

		brk->sim_SettlePhi1BeforeRandom();

		random->sim_Impl<true, 1>(true);

		disp->sim_AfterRandomLogicImpl<true, 1>();
	}

	// The second Bottom traversal of a PHI2 call. The Bottom steps are, in order: DL = ~data; the
	// register, ADD, flag (P_DB), stack, PC drivers onto the four buses; the PC increment; the two
	// connect muxes; the constant generator; the ALU compute; the flag latch2 copies; DOR/the external
	// data drive; the address output; the pads. In the second traversal the PC increment reads PCLS/PCHS
	// (written in PHI1 only) and n_1PC (unchanged by the second Top traversal) and so rewrites the
	// PackedPCL/PCH the first traversal wrote; the ALU compute reads AI/BI (loaded in PHI1 only) and the
	// unchanged command word and so rewrites n_ADD and the ACR/AVR latches it wrote (the ADD drivers of
	// the second traversal therefore see the same n_ADD whether or not the compute re-runs after them);
	// the flag latch2 copies read the PHI1 latch1 bank, so_latch1/3, BRK6E and AVR_V, all unchanged;
	// the external data drive rewrites ~DOR (PHI1 state) where the first traversal already put it, the
	// address output ABH:ABL (PHI1 state), and the pads rw_latch.nget() and t1_latch.nget() (PHI1
	// latches). Those steps are the identity in the second traversal and are omitted; every bus-driver
	// step and the DL re-latch (the data byte may now carry the first traversal's write) run as before,
	// in the original order. FASTSIM_SKIP_ORACLE runs the full traversal from a snapshot of the same
	// state and compares every byte of the core, the pads, the address and the data (sim_Phi2BottomOracle).
	void M6502::sim_Phi2BottomSettle(uint8_t* ext_data_bus)
	{
		data_bus->sim_GetExternalBusImpl<2>(ext_data_bus);
		regs->sim_StoreSBImpl<2>();
		alu->sim_StoreADD();
		random->flags->sim_Store();
		regs->sim_StoreOldS();
		pc->sim_StoreHLE();
		alu->sim_BusMux();
		addr_bus->sim_ConstGen();
	}

#ifdef FASTSIM_SKIP_ORACLE
	namespace
	{
		// Byte image of the core and every sub-object (the Top traversal touches only these).
		struct CoreImage
		{
			static constexpr size_t Regions = 19;
			const void* ptr[Regions]; size_t size[Regions]; std::vector<uint8_t> bytes[Regions];
			void Bind(const void* const* p, const size_t* s) { for (size_t n = 0; n < Regions; n++) { ptr[n] = p[n]; size[n] = s[n]; } }
			void Capture() { for (size_t n = 0; n < Regions; n++) bytes[n].assign(static_cast<const uint8_t*>(ptr[n]), static_cast<const uint8_t*>(ptr[n]) + size[n]); }
			void Restore() const { for (size_t n = 0; n < Regions; n++) std::memcpy(const_cast<void*>(ptr[n]), bytes[n].data(), size[n]); }
			bool Equals(const CoreImage& o, size_t& region, size_t& offset) const
			{
				for (size_t n = 0; n < Regions; n++) for (size_t b = 0; b < size[n]; b++) if (bytes[n][b] != o.bytes[n][b]) { region = n; offset = b; return false; }
				return true;
			}
		};
	}

	// Oracle for sim_Phi2TopSettle: from a snapshot of the state before the second traversal, run the
	// settle path and the full traversal separately and abort if any byte of the core differs.
	void M6502::sim_Phi2TopOracle(TriState inputs[], uint8_t* data_bus)
	{
		const void* const regions[CoreImage::Regions] = { this, predecode, ir, ext, brk, disp, random,
			random->regs_control, random->alu_control, random->pc_control, random->bus_control, random->flags_control,
			random->flags, random->branch_logic, addr_bus, regs, alu, pc, data_bus };
		const size_t sizes[CoreImage::Regions] = { sizeof(*this), sizeof(*predecode), sizeof(*ir), sizeof(*ext), sizeof(*brk), sizeof(*disp), sizeof(*random),
			sizeof(*random->regs_control), sizeof(*random->alu_control), sizeof(*random->pc_control), sizeof(*random->bus_control), sizeof(*random->flags_control),
			sizeof(*random->flags), sizeof(*random->branch_logic), sizeof(*addr_bus), sizeof(*regs), sizeof(*alu), sizeof(*pc), sizeof(*data_bus) };
		CoreImage before{}, settled{}, full{};
		before.Bind(regions, sizes); settled.Bind(regions, sizes); full.Bind(regions, sizes);
		before.Capture();
		sim_Phi2TopSettle(data_bus);
		settled.Capture();
		before.Restore();
		sim_TopImpl<true, 2>(inputs, data_bus, true);
		full.Capture();
		size_t region = 0, offset = 0;
		if (!settled.Equals(full, region, offset))
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: M6502 PHI2 Top settle violated (region %zu offset %zu: settle %02x, full traversal %02x; cmd %016llx)\n",
				region, offset, settled.bytes[region][offset], full.bytes[region][offset], (unsigned long long)cmd.raw);
			std::abort();
		}
	}

	// Oracle for sim_Phi1TopSettle: as sim_Phi2TopOracle, for the PHI1 second traversal.
	void M6502::sim_Phi1TopOracle(TriState inputs[], uint8_t* data_bus)
	{
		const void* const regions[CoreImage::Regions] = { this, predecode, ir, ext, brk, disp, random,
			random->regs_control, random->alu_control, random->pc_control, random->bus_control, random->flags_control,
			random->flags, random->branch_logic, addr_bus, regs, alu, pc, this->data_bus };
		const size_t sizes[CoreImage::Regions] = { sizeof(*this), sizeof(*predecode), sizeof(*ir), sizeof(*ext), sizeof(*brk), sizeof(*disp), sizeof(*random),
			sizeof(*random->regs_control), sizeof(*random->alu_control), sizeof(*random->pc_control), sizeof(*random->bus_control), sizeof(*random->flags_control),
			sizeof(*random->flags), sizeof(*random->branch_logic), sizeof(*addr_bus), sizeof(*regs), sizeof(*alu), sizeof(*pc), sizeof(*this->data_bus) };
		CoreImage before{}, settled{}, full{};
		before.Bind(regions, sizes); settled.Bind(regions, sizes); full.Bind(regions, sizes);
		before.Capture();
		sim_Phi1TopSettle(data_bus);
		settled.Capture();
		before.Restore();
		sim_TopImpl<true, 1>(inputs, data_bus, true);
		full.Capture();
		size_t region = 0, offset = 0;
		if (!settled.Equals(full, region, offset))
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: M6502 PHI1 Top settle violated (region %zu offset %zu: settle %02x, full traversal %02x; cmd %016llx)\n",
				region, offset, settled.bytes[region][offset], full.bytes[region][offset], (unsigned long long)cmd.raw);
			std::abort();
		}
	}

	// Oracle for sim_Phi2BottomSettle: from a snapshot of the state before the second Bottom traversal,
	// run the settle path and the full traversal separately and abort if any byte of the core, the
	// pads, the address or the data differs.
	void M6502::sim_Phi2BottomOracle(TriState inputs[], TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus)
	{
		const void* const regions[CoreImage::Regions] = { this, predecode, ir, ext, brk, disp, random,
			random->regs_control, random->alu_control, random->pc_control, random->bus_control, random->flags_control,
			random->flags, random->branch_logic, this->addr_bus, regs, alu, pc, this->data_bus };
		const size_t sizes[CoreImage::Regions] = { sizeof(*this), sizeof(*predecode), sizeof(*ir), sizeof(*ext), sizeof(*brk), sizeof(*disp), sizeof(*random),
			sizeof(*random->regs_control), sizeof(*random->alu_control), sizeof(*random->pc_control), sizeof(*random->bus_control), sizeof(*random->flags_control),
			sizeof(*random->flags), sizeof(*random->branch_logic), sizeof(*this->addr_bus), sizeof(*regs), sizeof(*alu), sizeof(*pc), sizeof(*this->data_bus) };
		CoreImage before{}, settled{}, full{};
		before.Bind(regions, sizes); settled.Bind(regions, sizes); full.Bind(regions, sizes);
		TriState outs_before[(size_t)OutputPad::Max], outs_settled[(size_t)OutputPad::Max];
		std::memcpy(outs_before, outputs, sizeof outs_before);
		const uint16_t addr_before = *addr_bus; const uint8_t data_before = *data_bus;
		before.Capture();
		sim_Phi2BottomSettle(data_bus);
		settled.Capture();
		std::memcpy(outs_settled, outputs, sizeof outs_settled);
		const uint16_t addr_settled = *addr_bus; const uint8_t data_settled = *data_bus;
		before.Restore();
		std::memcpy(outputs, outs_before, sizeof outs_before); *addr_bus = addr_before; *data_bus = data_before;
		sim_BottomImpl<true, 2>(inputs, outputs, addr_bus, data_bus);
		full.Capture();
		size_t region = 0, offset = 0;
		const bool same = settled.Equals(full, region, offset) && std::memcmp(outs_settled, outputs, sizeof outs_settled) == 0 &&
			addr_settled == *addr_bus && data_settled == *data_bus;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: M6502 PHI2 Bottom settle violated (region %zu offset %zu: settle %02x, full traversal %02x; addr %04x/%04x data %02x/%02x; cmd %016llx)\n",
				region, offset, settled.bytes[region][offset], full.bytes[region][offset], addr_settled, *addr_bus, data_settled, *data_bus, (unsigned long long)cmd.raw);
			std::abort();
		}
	}

	// Run the skipped PHI1 second Bottom traversal from the same state and abort if it changes
	// anything: buses and dirty flags, rw_latch, the bottom-owned objects, pads, address, data.
	void M6502::sim_Phi1BottomOracle(TriState inputs[], TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus)
	{
		uint8_t ab[sizeof(AddressBus)], rg[sizeof(Regs)], al[sizeof(ALU)], pcb[sizeof(ProgramCounter)], db[sizeof(DataBus)], fl[sizeof(Flags)];
		std::memcpy(ab, static_cast<const void*>(this->addr_bus), sizeof ab); std::memcpy(rg, static_cast<const void*>(regs), sizeof rg);
		std::memcpy(al, static_cast<const void*>(alu), sizeof al); std::memcpy(pcb, static_cast<const void*>(pc), sizeof pcb);
		std::memcpy(db, static_cast<const void*>(this->data_bus), sizeof db); std::memcpy(fl, static_cast<const void*>(random->flags), sizeof fl);
		const uint8_t sb = SB, dbv = DB, adl = ADL, adh = ADH;
		const bool d0 = SB_Dirty, d1 = DB_Dirty, d2 = ADL_Dirty, d3 = ADH_Dirty;
		const TriState rw = rw_latch.get();
		TriState outs[(size_t)OutputPad::Max]; std::memcpy(outs, outputs, sizeof outs);
		const uint16_t addr = *addr_bus; const uint8_t data = *data_bus;
		sim_BottomImpl<true, 1>(inputs, outputs, addr_bus, data_bus);
		const bool same = std::memcmp(ab, static_cast<const void*>(this->addr_bus), sizeof ab) == 0 && std::memcmp(rg, static_cast<const void*>(regs), sizeof rg) == 0 &&
			std::memcmp(al, static_cast<const void*>(alu), sizeof al) == 0 && std::memcmp(pcb, static_cast<const void*>(pc), sizeof pcb) == 0 &&
			std::memcmp(db, static_cast<const void*>(this->data_bus), sizeof db) == 0 && std::memcmp(fl, static_cast<const void*>(random->flags), sizeof fl) == 0 &&
			sb == SB && dbv == DB && adl == ADL && adh == ADH && d0 == SB_Dirty && d1 == DB_Dirty && d2 == ADL_Dirty && d3 == ADH_Dirty &&
			rw == rw_latch.get() && std::memcmp(outs, outputs, sizeof outs) == 0 && addr == *addr_bus && data == *data_bus;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: M6502 PHI1 Bottom replay violated (cmd %016llx SB %02x->%02x DB %02x->%02x ADL %02x->%02x ADH %02x->%02x)\n",
				(unsigned long long)cmd.raw, sb, SB, dbv, DB, adl, ADL, adh, ADH);
			std::abort();
		}
	}
#endif

	void M6502::sim(TriState inputs[], TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus)
	{
		TriState PHI0 = inputs[(size_t)InputPad::PHI0];
		TriState PHI2 = PHI0;

		// Precharge internal buses.
		// This operation is critical because it is used to form the interrupt address and the stack pointer (and many other cases).

		if (PHI2 == TriState::One)
		{
			SB = 0xff;
			DB = 0xff;
			ADL = 0xff;
			ADH = 0xff;
		}

		// These variables are used to mark the "dirty" bits of the internal buses. This is used to resolve conflicts, according to the "Ground wins" rule.

		SB_Dirty = false;
		DB_Dirty = false;
		ADL_Dirty = false;
		ADH_Dirty = false;

		// To stabilize latches, both parts are simulated twice.

		// Both settling passes remain. Packed phase scheduling only removes
		// calls proved inert after the phase-specific control outputs above.
#ifdef M6502_PASS2_STATS
		const unsigned stats_phi = PHI2 == TriState::One ? 1 : 0;
		pass2_stats.calls[stats_phi]++;
		std::vector<std::vector<uint8_t>> pass1_state, top2_state, bottom1_state;
		TriState pass1_outputs[(size_t)OutputPad::Max]{};
		uint16_t pass1_addr = 0; uint8_t pass1_data = 0;
		// Everything the Bottom traversal reads besides bottom-owned state: the command word, the
		// wires it consumes (PHI1/PHI2/WR/n_1PC/BRK6E/SO/n_IR5, T1 from the dispatcher, B_OUT from BRK),
		// the four internal buses with their dirty flags, and the external data byte.
		struct BottomInputs { uint64_t cmd; uint8_t wires[9]; uint8_t buses[4]; bool dirty[4]; uint8_t ext; };
		BottomInputs bottom1_in{}, bottom2_in{};
		auto capture_inputs = [&](BottomInputs& b) {
			b.cmd = cmd.raw;
			b.wires[0] = wire.PHI1; b.wires[1] = wire.PHI2; b.wires[2] = wire.WR; b.wires[3] = wire.n_1PC; b.wires[4] = wire.BRK6E;
			b.wires[5] = wire.SO; b.wires[6] = wire.n_IR5; b.wires[7] = disp->getT1(); b.wires[8] = brk->getB_OUT(wire.BRK6E);
			b.buses[0] = SB; b.buses[1] = DB; b.buses[2] = ADL; b.buses[3] = ADH;
			b.dirty[0] = SB_Dirty; b.dirty[1] = DB_Dirty; b.dirty[2] = ADL_Dirty; b.dirty[3] = ADH_Dirty;
			b.ext = *data_bus;
		};
#define M6502_PASS2_AFTER_TOP1() do { capture_inputs(bottom1_in); } while (0)
#define M6502_PASS2_BOTTOM2_SKIPPED() do { pass2_stats.bottom2_skipped[stats_phi]++; } while (0)
#define M6502_PASS2_AFTER_PASS1() do { pass1_state = SnapshotState(); std::memcpy(pass1_outputs, outputs, sizeof(pass1_outputs)); \
			pass1_addr = *addr_bus; pass1_data = *data_bus; bottom1_state = pass1_state; } while (0)
#define M6502_PASS2_AFTER_TOP2() do { top2_state = SnapshotState(); capture_inputs(bottom2_in); \
			bool any = false; for (size_t n = 0; n < top2_state.size(); n++) if (bottom1_state[n] != top2_state[n]) any = true; \
			if (any) pass2_stats.top2_changed[stats_phi]++; \
			pass2_stats.txbits_hist[TxBits & 63]++; pass2_stats.keys_seen[(ir->IROut | ((TxBits & 63) << 8)) >> 6] |= UINT64_C(1) << ((ir->IROut | ((TxBits & 63) << 8)) & 63); } while (0)
#define M6502_PASS2_AFTER_PASS2() do { const auto pass2_state = SnapshotState(); bool any = false; \
			pass2_stats.wire_offset = size_t(reinterpret_cast<const uint8_t*>(&wire) - reinterpret_cast<const uint8_t*>(this)); \
			pass2_stats.wire_size = sizeof(wire); pass2_stats.core_size = sizeof(*this); pass2_stats.disp_size = sizeof(*disp); \
			for (size_t n = 0; n < pass2_state.size(); n++) if (pass1_state[n] != pass2_state[n]) { pass2_stats.objects[stats_phi][n]++; any = true; \
				if (n == 0) for (size_t b = 0; b < pass2_state[n].size() && b < 1024; b++) if (pass1_state[n][b] != pass2_state[n][b]) pass2_stats.core_bytes[stats_phi][b]++; \
				if (n == 5) for (size_t b = 0; b < pass2_state[n].size() && b < 256; b++) if (pass1_state[n][b] != pass2_state[n][b]) pass2_stats.disp_bytes[stats_phi][b]++; } \
			if (any) pass2_stats.changed[stats_phi]++; \
			if (std::memcmp(pass1_outputs, outputs, sizeof(pass1_outputs)) != 0 || pass1_addr != *addr_bus || pass1_data != *data_bus) \
				pass2_stats.external_changed[stats_phi]++; \
			bool bottom2_any = false; \
			for (size_t n = 0; n < pass2_state.size(); n++) if (top2_state[n] != pass2_state[n]) { pass2_stats.bottom2_objects[stats_phi][n]++; bottom2_any = true; \
				if (n == 0) for (size_t b = 0; b < pass2_state[n].size() && b < 1024; b++) if (top2_state[n][b] != pass2_state[n][b]) pass2_stats.bottom2_core_bytes[stats_phi][b]++; } \
			if (bottom2_any) pass2_stats.bottom2_changed[stats_phi]++; \
			const bool eq_ex = bottom1_in.cmd == bottom2_in.cmd && std::memcmp(bottom1_in.wires, bottom2_in.wires, sizeof(bottom1_in.wires)) == 0 && bottom1_in.ext == bottom2_in.ext; \
			const bool eq_incl = eq_ex && std::memcmp(bottom1_in.buses, bottom2_in.buses, sizeof(bottom1_in.buses)) == 0 && std::memcmp(bottom1_in.dirty, bottom2_in.dirty, sizeof(bottom1_in.dirty)) == 0; \
			if (eq_ex) pass2_stats.inputs_equal_ex_buses[stats_phi]++; \
			if (eq_incl) pass2_stats.inputs_equal_incl_buses[stats_phi]++; \
			if (eq_ex && bottom2_any) pass2_stats.bottom2_changed_inputs_equal_ex_buses[stats_phi]++; \
			if (eq_incl && bottom2_any) pass2_stats.bottom2_changed_inputs_equal_incl_buses[stats_phi]++; } while (0)
#else
#define M6502_PASS2_AFTER_TOP1() ((void)0)
#define M6502_PASS2_BOTTOM2_SKIPPED() ((void)0)
#define M6502_PASS2_AFTER_PASS1() ((void)0)
#define M6502_PASS2_AFTER_TOP2() ((void)0)
#define M6502_PASS2_AFTER_PASS2() ((void)0)
#endif
		if (execution_backend == ExecutionBackend::ExactPacked)
		{
			// Phase-specialized kernels: PHI0 Zero -> Phase 1, One -> Phase 2 (every latch enable and
			// phase test is then a compile-time constant); any other pad value -> the generic Phase 0,
			// which keeps the reference's run-time phase expressions. The second Top traversal re-runs
			// only the blocks that can settle (dispatcher, BRK, branch logic, pads/counters); the five
			// command evaluators are a replay for a binary PHI0 (see RandomLogic::sim_Impl).
			const auto packed_call = [&](auto phase_tag) {
				constexpr int Phase = decltype(phase_tag)::value;
				sim_TopImpl<true, Phase>(inputs,data_bus);
				M6502_PASS2_AFTER_TOP1();
				BottomControls controls1{};
				if constexpr (Phase == 1) controls1 = CaptureBottomControls();
				sim_BottomImpl<true, Phase>(inputs,outputs,addr_bus,data_bus);
				M6502_PASS2_AFTER_PASS1();
				// PHI2: the second Top traversal settles only ACRL1, the Z_IR cone, the predecoder and the
				// ENDS/TRES1/tresx_latch2/n_TRESX group (see sim_Phi2TopSettle); PHI1 and the generic
				// phase run the full traversal (in PHI1 the T-state latches are transparent and the row
				// re-selects).
				if constexpr (Phase == 2)
				{
#ifdef FASTSIM_SKIP_ORACLE
					sim_Phi2TopOracle(inputs, data_bus);
#else
					sim_Phi2TopSettle(data_bus);
#endif
				}
				else if constexpr (Phase == 1)
				{
					// PHI1: the second Top traversal re-runs the row and everything downstream of it and
					// settles the n_T0/Z_IR cones and the BRK5_RDY/BRK7/donmi chain (see sim_Phi1TopSettle).
#ifdef FASTSIM_SKIP_ORACLE
					sim_Phi1TopOracle(inputs, data_bus);
#else
					sim_Phi1TopSettle(data_bus);
#endif
				}
				else sim_TopImpl<true, Phase>(inputs,data_bus,Phase != 0);
				M6502_PASS2_AFTER_TOP2();
				if constexpr (Phase == 1)
				{
					// PHI1: skip the second Bottom traversal when it is provably the identity (see
					// Phi1BottomIdempotent). AVR_V is the only command bit the second Top traversal may
					// change, and Bottom does not consume it in PHI1 (avr_latch is PHI2-enabled).
					const BottomControls controls2 = CaptureBottomControls();
					const bool replay = ((controls1.cmd ^ controls2.cmd) & ~(UINT64_C(1) << 57)) == 0 &&
						controls1.WR == controls2.WR && controls1.T1 == controls2.T1 && controls1.BRK6E == controls2.BRK6E &&
						controls1.B_OUT == controls2.B_OUT && Phi1BottomIdempotent(controls2.cmd);
					if (!replay) sim_BottomImpl<true, Phase>(inputs,outputs,addr_bus,data_bus);
					else M6502_PASS2_BOTTOM2_SKIPPED();
#ifdef FASTSIM_SKIP_ORACLE
					if (replay) sim_Phi1BottomOracle(inputs,outputs,addr_bus,data_bus);
#endif
				}
				else if constexpr (Phase == 2)
				{
					// PHI2: the second Bottom traversal runs only its bus-driver steps (see sim_Phi2BottomSettle).
#ifdef FASTSIM_SKIP_ORACLE
					sim_Phi2BottomOracle(inputs,outputs,addr_bus,data_bus);
#else
					sim_Phi2BottomSettle(data_bus);
#endif
				}
				else sim_BottomImpl<true, Phase>(inputs,outputs,addr_bus,data_bus);
				M6502_PASS2_AFTER_PASS2();
			};
			if (PHI0 == TriState::Zero) packed_call(std::integral_constant<int, 1>{});
			else if (PHI0 == TriState::One) packed_call(std::integral_constant<int, 2>{});
			else packed_call(std::integral_constant<int, 0>{});
		}
		else
		{
			sim_TopImpl<false>(inputs,data_bus);
			M6502_PASS2_AFTER_TOP1();
			sim_BottomImpl<false>(inputs,outputs,addr_bus,data_bus);
			M6502_PASS2_AFTER_PASS1();
			sim_TopImpl<false>(inputs,data_bus);
			M6502_PASS2_AFTER_TOP2();
			sim_BottomImpl<false>(inputs,outputs,addr_bus,data_bus);
			M6502_PASS2_AFTER_PASS2();
		}
#undef M6502_PASS2_AFTER_TOP1
#undef M6502_PASS2_BOTTOM2_SKIPPED
#undef M6502_PASS2_AFTER_PASS1
#undef M6502_PASS2_AFTER_TOP2
#undef M6502_PASS2_AFTER_PASS2

		// External bus write cycle trace: the core drives the address and data
		// buses during the PHI2 phase of a write cycle. One line per write cycle.

		if (wire.PHI2 == TriState::One &&
			outputs[(size_t)OutputPad::RnW] == TriState::Zero)
		{
			LOG_CORE(Cat_Bus, "Write $%04X = %02X", (int)*addr_bus, (int)*data_bus);
		}
	}
}
