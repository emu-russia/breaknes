#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	M6502::M6502(bool HLE, bool BCD_Hack)
	{
		HLE_Mode = HLE;

		decoder = new Decoder;
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
		wire.n_NMI = inputs[(size_t)InputPad::n_NMI];
		wire.n_IRQ = inputs[(size_t)InputPad::n_IRQ];
		wire.n_RES = inputs[(size_t)InputPad::n_RES];
		wire.PHI0 = inputs[(size_t)InputPad::PHI0];
		wire.RDY = inputs[(size_t)InputPad::RDY];
		wire.SO = inputs[(size_t)InputPad::SO];

		// Logic associated with external input terminals

		wire.PHI1 = NOT(wire.PHI0);
		wire.PHI2 = wire.PHI0;

		prdy_latch1.set(NOT(wire.RDY), wire.PHI2);
		prdy_latch2.set(prdy_latch1.nget(), wire.PHI1);
		wire.n_PRDY = prdy_latch2.nget();

		if (wire.PHI2 == TriState::One && nNMI_Cache != wire.n_NMI)
		{
			nmip_ff.set(NOR(NOR(nmip_ff.get(), AND(NOT(wire.n_NMI), wire.PHI2)), AND(wire.n_NMI, wire.PHI2)));
			nNMI_Cache = wire.n_NMI;
		}
		wire.n_NMIP = NOT(nmip_ff.get());

		if (wire.PHI2 == TriState::One && nIRQ_Cache != wire.n_IRQ)
		{
			irqp_ff.set(NOR(NOR(irqp_ff.get(), AND(NOT(wire.n_IRQ), wire.PHI2)), AND(wire.n_IRQ, wire.PHI2)));
			nIRQ_Cache = wire.n_IRQ;
		}
		irqp_latch.set(irqp_ff.get(), wire.PHI1);
		wire.n_IRQP = irqp_latch.nget();

		if (wire.PHI2 == TriState::One && nRES_Cache != wire.n_RES)
		{
			resp_ff.set(NOR(NOR(resp_ff.get(), AND(wire.n_RES, wire.PHI2)), AND(NOT(wire.n_RES), wire.PHI2)));
			nRES_Cache = wire.n_RES;
		}
		resp_latch.set(resp_ff.get(), wire.PHI1);
		wire.RESP = resp_latch.nget();

		// Dispatcher and other auxiliary logic

		disp->sim_BeforeDecoder();

		predecode->sim(data_bus);

		ir->sim();

		if (HLE_Mode)
		{
			ext->sim_HLE();
		}
		else
		{
			ext->sim();
		}

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

		TxBits = 0;
		TxBits |= ((size_t)wire.n_T0 << 0);
		TxBits |= ((size_t)wire.n_T1X << 1);
		TxBits |= ((size_t)wire.n_T2 << 2);
		TxBits |= ((size_t)wire.n_T3 << 3);
		TxBits |= ((size_t)wire.n_T4 << 4);
		TxBits |= ((size_t)wire.n_T5 << 5);

		decoder->sim(decoder_in.packed_bits, &decoder_out);

		// Interrupt handling

		brk->sim_BeforeRandom();

		// Random Logic

		disp->sim_BeforeRandomLogic();

		random->sim();

		brk->sim_AfterRandom();

		disp->sim_AfterRandomLogic();
	}

	void M6502::sim_Bottom(TriState inputs[], TriState outputs[], uint16_t* ext_addr_bus, uint8_t* ext_data_bus)
	{
		// Bottom Part

		// When you simulate the lower part, you have to turn on the man in you to the fullest and imagine that you are possessed by Chuck Peddle.
		// From the development history of the 6502, we know that one developer did the upper part and another (Chuck) did the lower part. 
		// So to simulate the lower part you have to abstract as much as possible and work only with what you have (registers, buses and set of control commands from the upper part).

		// Bus load from DL: DL_DB, DL_ADL, DL_ADH

		data_bus->sim_GetExternalBus(ext_data_bus);

		// Registers to the SB bus: Y_SB, X_SB, S_SB

		regs->sim_StoreSB();

		// ADD saving on SB/ADL: ADD_SB7, ADD_SB06, ADD_ADL

		alu->sim_StoreADD();

		// Saving AC: AC_SB, AC_DB

		alu->sim_StoreAC();

		// Saving flags on DB bus: P_DB

		random->flags->sim_Store();

		// Stack pointer saving on ADL bus: S_ADL

		regs->sim_StoreOldS();

		// Increment PC: n_1PC

		pc->sim();

		// Saving PC to buses: PCL_ADL, PCH_ADH, PCL_DB, PCH_DB

		pc->sim_Store();

		// Bus multiplexing: SB_DB, SB_ADH

		alu->sim_BusMux();

		// Constant generator: Z_ADL0, Z_ADL1, Z_ADL2, Z_ADH0, Z_ADH17

		addr_bus->sim_ConstGen();

		// Loading ALU operands: NDB_ADD, DB_ADD, Z_ADD, SB_ADD, ADL_ADD

		alu->sim_Load();

		// ALU operation: ANDS, EORS, ORS, SRS, SUMS, n_ACIN, n_DAA, n_DSA
		// BCD correction via SB bus: SB_AC

		if (HLE_Mode)
		{
			alu->sim_HLE();
		}
		else
		{
			alu->sim();
		}

		// Load flags: DB_P, DBZ_Z, DB_N, IR5_C, DB_C, IR5_D, IR5_I, DB_V, Z_V, ACR_C, AVR_V

		random->flags->sim_Load();

		// Load registers: SB_X, SB_Y, SB_S / S_S

		regs->sim_LoadSB();

		// PC loading from buses / keep: ADH_PCH/PCH_PCH, ADL_PCL/PCL_PCL

		pc->sim_Load();

		// Saving DB to DOR

		data_bus->sim_SetExternalBus(ext_data_bus);

		// Set external address bus: ADH_ABH, ADL_ABL

		addr_bus->sim_Output(ext_addr_bus);

		// Outputs

		rw_latch.set(wire.WR, wire.PHI1);

		outputs[(size_t)OutputPad::PHI1] = wire.PHI1;
		outputs[(size_t)OutputPad::PHI2] = wire.PHI2;
		outputs[(size_t)OutputPad::RnW] = rw_latch.nget();
		outputs[(size_t)OutputPad::SYNC] = disp->getT1();
	}

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

		for (size_t n = 0; n < 8; n++)
		{
			SB_Dirty = false;
			DB_Dirty = false;
			ADL_Dirty = false;
			ADH_Dirty = false;
		}

		// To stabilize latches, both parts are simulated twice.

		sim_Top(inputs, data_bus);
		sim_Bottom(inputs, outputs, addr_bus, data_bus);
		sim_Top(inputs, data_bus);
		sim_Bottom(inputs, outputs, addr_bus, data_bus);
	}
}
