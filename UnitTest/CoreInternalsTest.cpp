#include "pch.h"

#include <cstddef>

using namespace BaseLogic;
using namespace M6502Core;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{
	/// <summary>
	/// Runs the standard reset sequence (32 half-cycles with n_RES=0 and data bus = 0),
	/// then `cycles` half-cycles of the given program, capturing DebugInfo after each half-cycle.
	/// This is the same harness that was used to produce the ground-truth traces.
	/// </summary>
	void RunProgramCapture(M6502* core, uint8_t* prog, size_t prog_len, size_t cycles, DebugInfo* out)
	{
		TriState inputs[(size_t)InputPad::Max]{};
		TriState outputs[(size_t)OutputPad::Max]{};
		uint16_t addr_bus = 0;
		uint8_t data_bus = 0;

		inputs[(size_t)InputPad::n_IRQ] = TriState::One;
		inputs[(size_t)InputPad::n_NMI] = TriState::One;
		inputs[(size_t)InputPad::PHI0] = TriState::One;
		inputs[(size_t)InputPad::RDY] = TriState::One;
		inputs[(size_t)InputPad::SO] = TriState::One;
		inputs[(size_t)InputPad::n_RES] = TriState::Zero;

		TriState PHI = TriState::Zero;

		for (size_t n = 0; n < 16; n++)
		{
			data_bus = 0;
			inputs[(size_t)InputPad::PHI0] = PHI;
			core->sim(inputs, outputs, &addr_bus, &data_bus);
			PHI = NOT(PHI);
			data_bus = 0;
			inputs[(size_t)InputPad::PHI0] = PHI;
			core->sim(inputs, outputs, &addr_bus, &data_bus);
			PHI = NOT(PHI);
		}

		inputs[(size_t)InputPad::n_RES] = TriState::One;

		for (size_t n = 0; n < cycles; n++)
		{
			data_bus = (addr_bus < prog_len) ? prog[addr_bus] : 0;
			inputs[(size_t)InputPad::PHI0] = PHI;
			core->sim(inputs, outputs, &addr_bus, &data_bus);
			PHI = NOT(PHI);
			core->getDebug(&out[n]);
		}
	}
}

namespace M6502CoreUnitTest
{
	bool UnitTest::Decoder_Test()
	{
		auto decoder = new Decoder;

		TriState* outputs = nullptr;

		// Input 0: no term matches, so all 130 outputs are One.

		decoder->sim(0, &outputs);
		if (!outputs)
		{
			Logger::WriteMessage("Decoder_Test failed! outputs == nullptr for input 0\n");
			delete decoder;
			return false;
		}

		for (size_t n = 0; n < Decoder::outputs_count; n++)
		{
			uint8_t v = ToByte(outputs[n]);
			if (v != 1)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "Decoder_Test failed! output[%zu] is %d, expected 1 for input 0\n", n, v);
				Logger::WriteMessage(msg);
				delete decoder;
				return false;
			}
		}

		// Several inputs: the outputs pointer must be non-null and all outputs must be Zero/One.

		size_t inputs[] = { 0, (1ULL << Decoder::inputs_count) - 1, 0x12345, 0x155555, 0x1FFFFF, 0x55555, 0xAAAAA };
		for (size_t i = 0; i < _countof(inputs); i++)
		{
			decoder->sim(inputs[i], &outputs);
			if (!outputs)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "Decoder_Test failed! outputs == nullptr for input 0x%zX\n", inputs[i]);
				Logger::WriteMessage(msg);
				delete decoder;
				return false;
			}

			for (size_t n = 0; n < Decoder::outputs_count; n++)
			{
				uint8_t v = ToByte(outputs[n]);
				if (v != 0 && v != 1)
				{
					char msg[0x100];
					sprintf_s(msg, sizeof(msg), "Decoder_Test failed! output[%zu] is %d (not Zero/One) for input 0x%zX\n", n, v, inputs[i]);
					Logger::WriteMessage(msg);
					delete decoder;
					return false;
				}
			}
		}

		// The all-ones input grounds every output line (every decoder output has at
		// least one transistor), while the zero input leaves all outputs at One.
		// So the decoder must respond to the input pattern: output[0] = 1 for input 0
		// and 0 for the all-ones input.

		decoder->sim((1ULL << Decoder::inputs_count) - 1, &outputs);
		size_t zeros = 0;
		for (size_t n = 0; n < Decoder::outputs_count; n++)
		{
			uint8_t v = ToByte(outputs[n]);
			if (v != 0)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "Decoder_Test failed! output[%zu] is %d for the all-ones input, expected 0\n", n, v);
				Logger::WriteMessage(msg);
				delete decoder;
				return false;
			}
			zeros++;
		}
		if (zeros != Decoder::outputs_count)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Decoder_Test failed! all-ones input gives zeros=%zu\n", zeros);
			Logger::WriteMessage(msg);
			delete decoder;
			return false;
		}

		delete decoder;
		Logger::WriteMessage("Decoder_Test OK!\n");
		return true;
	}

	bool UnitTest::IR_Test()
	{
		// PHI1 & FETCH: the IR latches n_PD, IROut = ~n_PD.

		core->wire.PHI1 = TriState::One;
		core->wire.FETCH = TriState::One;
		core->predecode->n_PD = 0xA5;
		core->ir->IROut = 0;
		core->ir->sim();
		if (core->ir->IROut != 0x5A)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "IR_Test failed! IROut=0x%02X, expected 0x5A\n", core->ir->IROut);
			Logger::WriteMessage(msg);
			return false;
		}

		// n_PD = 0 -> IROut = 0xFF.

		core->predecode->n_PD = 0;
		core->ir->sim();
		if (core->ir->IROut != 0xFF)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "IR_Test failed! IROut=0x%02X, expected 0xFF\n", core->ir->IROut);
			Logger::WriteMessage(msg);
			return false;
		}

		// FETCH = 0 -> the IR does not change.

		core->wire.FETCH = TriState::Zero;
		core->predecode->n_PD = 0x01;
		core->ir->sim();
		if (core->ir->IROut != 0xFF)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "IR_Test failed! IROut changed to 0x%02X while FETCH=0\n", core->ir->IROut);
			Logger::WriteMessage(msg);
			return false;
		}

		// PHI1 = 0 -> the IR does not change.

		core->wire.PHI1 = TriState::Zero;
		core->wire.FETCH = TriState::One;
		core->predecode->n_PD = 0x10;
		core->ir->sim();
		if (core->ir->IROut != 0xFF)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "IR_Test failed! IROut changed to 0x%02X while PHI1=0\n", core->ir->IROut);
			Logger::WriteMessage(msg);
			return false;
		}

		Logger::WriteMessage("IR_Test OK!\n");
		return true;
	}

	bool UnitTest::PreDecode_Test()
	{
		// PHI2 = 1: PD = ~pd_latch = data bus value, n_PD = ~PD.

		core->wire.PHI2 = TriState::One;
		core->wire.Z_IR = TriState::Zero;
		uint8_t db = 0x5A;
		core->predecode->sim(&db);
		if (core->predecode->PD != 0x5A)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "PreDecode_Test failed! PD=0x%02X, expected 0x5A\n", core->predecode->PD);
			Logger::WriteMessage(msg);
			return false;
		}
		if (core->predecode->n_PD != 0xA5)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "PreDecode_Test failed! n_PD=0x%02X, expected 0xA5\n", core->predecode->n_PD);
			Logger::WriteMessage(msg);
			return false;
		}

		// Z_IR = 1: PD is forced to zero.

		core->wire.Z_IR = TriState::One;
		db = 0x33;
		core->predecode->sim(&db);
		if (core->predecode->PD != 0)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "PreDecode_Test failed! PD=0x%02X with Z_IR=1, expected 0\n", core->predecode->PD);
			Logger::WriteMessage(msg);
			return false;
		}
		if (core->predecode->n_PD != 0xFF)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "PreDecode_Test failed! n_PD=0x%02X with Z_IR=1, expected 0xFF\n", core->predecode->n_PD);
			Logger::WriteMessage(msg);
			return false;
		}

		// PHI2 = 0: the latch holds the previous value (pd_latch was updated with 0x33
		// during the Z_IR test above, so PD = ~pd_latch = 0x33).

		core->wire.PHI2 = TriState::Zero;
		core->wire.Z_IR = TriState::Zero;
		db = 0xFF;
		core->predecode->sim(&db);
		if (core->predecode->PD != 0x33)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "PreDecode_Test failed! PD=0x%02X after PHI2=0, expected 0x33 (latched)\n", core->predecode->PD);
			Logger::WriteMessage(msg);
			return false;
		}

		// The precomputed n_TWOCYCLE / n_IMPLIED values for known opcodes.

		core->wire.PHI2 = TriState::One;
		core->wire.Z_IR = TriState::Zero;

		db = 0xEA;	// NOP: 2-cycle, implied
		core->predecode->sim(&db);
		if (core->wire.n_TWOCYCLE != TriState::Zero || core->wire.n_IMPLIED != TriState::Zero)
		{
			Logger::WriteMessage("PreDecode_Test failed! NOP (0xEA) n_TWOCYCLE/n_IMPLIED mismatch\n");
			return false;
		}

		db = 0xA9;	// LDA #imm: 2-cycle, not implied
		core->predecode->sim(&db);
		if (core->wire.n_TWOCYCLE != TriState::Zero || core->wire.n_IMPLIED != TriState::One)
		{
			Logger::WriteMessage("PreDecode_Test failed! LDA #imm (0xA9) n_TWOCYCLE/n_IMPLIED mismatch\n");
			return false;
		}

		db = 0x6C;	// JMP (abs): 5-cycle, not implied
		core->predecode->sim(&db);
		if (core->wire.n_TWOCYCLE != TriState::One || core->wire.n_IMPLIED != TriState::One)
		{
			Logger::WriteMessage("PreDecode_Test failed! JMP (0x6C) n_TWOCYCLE/n_IMPLIED mismatch\n");
			return false;
		}

		Logger::WriteMessage("PreDecode_Test OK!\n");
		return true;
	}

	bool UnitTest::Flags_Test()
	{
		// --- Z flag ---

		// DBZ_Z with DB == 0 sets the Z flag.

		core->cmd.DBZ_Z = 1;
		core->cmd.DB_P = 0;
		core->DB = 0;
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_Z_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! Z not set with DBZ_Z=1 and DB=0\n");
			return false;
		}

		// DBZ_Z with DB != 0 clears the Z flag.

		core->DB = 0x02;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_Z_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! Z not cleared with DBZ_Z=1 and DB=2\n");
			return false;
		}

		// DB_P with DB bit1 = 1 sets the Z flag.

		core->cmd.DBZ_Z = 0;
		core->cmd.DB_P = 1;
		core->DB = 0x02;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_Z_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! Z not set with DB_P=1 and DB bit1=1\n");
			return false;
		}

		// DB_P with DB bit1 = 0 clears the Z flag.

		core->DB = 0;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_Z_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! Z not cleared with DB_P=1 and DB bit1=0\n");
			return false;
		}

		// --- N flag ---

		core->cmd.DB_N = 1;
		core->DB = 0x80;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_N_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! N not set with DB_N=1 and DB bit7=1\n");
			return false;
		}

		core->DB = 0x7F;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_N_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! N not cleared with DB_N=1 and DB bit7=0\n");
			return false;
		}

		// --- C flag ---

		// IR5_C with n_IR5=1 (CLC, IR5=0) clears the C flag.

		core->cmd.IR5_C = 1;
		core->cmd.DB_C = 0;
		core->cmd.ACR_C = 0;
		core->wire.n_IR5 = TriState::One;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! C not cleared with IR5_C=1 and n_IR5=1 (CLC)\n");
			return false;
		}

		// IR5_C with n_IR5=0 (SEC, IR5=1) sets the C flag.

		core->wire.n_IR5 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! C not set with IR5_C=1 and n_IR5=0 (SEC)\n");
			return false;
		}

		// DB_C with DB bit0 = 0 clears the C flag.

		core->cmd.IR5_C = 0;
		core->cmd.DB_C = 1;
		core->DB = 0x00;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! C not cleared with DB_C=1 and DB bit0=0\n");
			return false;
		}

		// DB_C with DB bit0 = 1 sets the C flag.

		core->DB = 0x01;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! C not set with DB_C=1 and DB bit0=1\n");
			return false;
		}

		// ACR_C with ACR=0 clears the C flag.

		core->cmd.DB_C = 0;
		core->cmd.ACR_C = 1;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! C not cleared with ACR_C=1 and ACR=0\n");
			return false;
		}

		// ACR_C with ACR=1 sets the C flag. First produce a carry with the ALU.

		core->cmd.SUMS = 1;
		core->cmd.n_ACIN = 1;
		core->cmd.ANDS = 0;
		core->cmd.EORS = 0;
		core->cmd.ORS = 0;
		core->cmd.SRS = 0;
		core->cmd.SB_AC = 0;
		core->wire.PHI2 = TriState::One;
		core->alu->setAI(0xFF);
		core->alu->setBI(0x01);
		core->alu->sim();
		if (core->alu->getACR() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! could not produce ACR=1\n");
			return false;
		}

		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_C_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! C not set with ACR_C=1 and ACR=1\n");
			return false;
		}

		// --- D flag ---

		// IR5_D with n_IR5=0 (SED, IR5=1) sets the D flag.

		core->cmd.IR5_D = 1;
		core->cmd.DB_P = 0;
		core->wire.n_IR5 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_D_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! D not set with IR5_D=1 and n_IR5=0 (SED)\n");
			return false;
		}

		// IR5_D with n_IR5=1 (CLD, IR5=0) clears the D flag.

		core->wire.n_IR5 = TriState::One;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_D_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! D not cleared with IR5_D=1 and n_IR5=1 (CLD)\n");
			return false;
		}

		// DB_P with DB bit3 = 1 sets the D flag.

		core->cmd.IR5_D = 0;
		core->cmd.DB_P = 1;
		core->DB = 0x08;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_D_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! D not set with DB_P=1 and DB bit3=1\n");
			return false;
		}

		// DB_P with DB bit3 = 0 clears the D flag.

		core->DB = 0;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_D_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! D not cleared with DB_P=1 and DB bit3=0\n");
			return false;
		}

		// --- I flag ---

		// IR5_I with n_IR5=0 (SEI, IR5=1) sets the I flag.

		core->cmd.IR5_I = 1;
		core->cmd.DB_P = 0;
		core->wire.n_IR5 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_I_OUT(TriState::Zero) != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! I not set with IR5_I=1 and n_IR5=0 (SEI)\n");
			return false;
		}

		// IR5_I with n_IR5=1 (CLI, IR5=0) clears the I flag.

		core->wire.n_IR5 = TriState::One;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_I_OUT(TriState::Zero) != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! I not cleared with IR5_I=1 and n_IR5=1 (CLI)\n");
			return false;
		}

		// DB_P with DB bit2 = 1 sets the I flag.

		core->cmd.IR5_I = 0;
		core->cmd.DB_P = 1;
		core->DB = 0x04;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_I_OUT(TriState::Zero) != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! I not set with DB_P=1 and DB bit2=1\n");
			return false;
		}

		// BRK6E=1 forces the I flag set (n_I_OUT=0) regardless of the latch.

		if (core->random->flags->getn_I_OUT(TriState::One) != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! getn_I_OUT(BRK6E=1) != 0\n");
			return false;
		}

		// --- V flag ---

		// DB_V with DB bit6 = 1 sets the V flag.

		core->cmd.DB_V = 1;
		core->cmd.Z_V = 0;
		core->cmd.AVR_V = 0;
		core->DB = 0x40;
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_V_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! V not set with DB_V=1 and DB bit6=1\n");
			return false;
		}

		// DB_V with DB bit6 = 0 clears the V flag.

		core->DB = 0;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_V_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! V not cleared with DB_V=1 and DB bit6=0\n");
			return false;
		}

		// Z_V clears the V flag.

		core->cmd.DB_V = 0;
		core->cmd.Z_V = 1;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_V_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! V not cleared with Z_V=1\n");
			return false;
		}

		// AVR_V with AVR=0 clears the V flag, with AVR=1 sets it.

		core->cmd.Z_V = 0;
		core->cmd.AVR_V = 1;
		core->alu->setAVR(TriState::Zero);
		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->random->flags->sim_Load();	// latches AVR_V into avr_latch
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->random->flags->sim_Load();	// computes v_latch1
		if (core->random->flags->getn_V_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! V not cleared with AVR_V=1 and AVR=0\n");
			return false;
		}

		core->alu->setAVR(TriState::One);
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->random->flags->sim_Load();
		if (core->random->flags->getn_V_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! V not set with AVR_V=1 and AVR=1\n");
			return false;
		}

		// --- Setters / getters ---

		core->random->flags->set_Z_OUT(TriState::One);
		if (core->random->flags->getn_Z_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_Z_OUT(One)/getn_Z_OUT broken\n");
			return false;
		}
		core->random->flags->set_Z_OUT(TriState::Zero);
		if (core->random->flags->getn_Z_OUT() != TriState::One)
		{
			Logger::WriteMessage("Flags_Test failed! set_Z_OUT(Zero)/getn_Z_OUT broken\n");
			return false;
		}

		core->random->flags->set_N_OUT(TriState::One);
		if (core->random->flags->getn_N_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_N_OUT/getn_N_OUT broken\n");
			return false;
		}
		core->random->flags->set_C_OUT(TriState::One);
		if (core->random->flags->getn_C_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_C_OUT/getn_C_OUT broken\n");
			return false;
		}
		core->random->flags->set_D_OUT(TriState::One);
		if (core->random->flags->getn_D_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_D_OUT/getn_D_OUT broken\n");
			return false;
		}
		core->random->flags->set_I_OUT(TriState::One);
		if (core->random->flags->getn_I_OUT(TriState::Zero) != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_I_OUT/getn_I_OUT broken\n");
			return false;
		}
		core->random->flags->set_V_OUT(TriState::One);
		if (core->random->flags->getn_V_OUT() != TriState::Zero)
		{
			Logger::WriteMessage("Flags_Test failed! set_V_OUT/getn_V_OUT broken\n");
			return false;
		}

		// --- sim_Store: P is placed on the DB bus ---

		// C=1 Z=1 I=1 D=0 V=1 N=1, B=0 -> bits 7654 3210 = 1 1 1 0 | 0 1 1 1 -> 0xE7 (bit5 stays 1)

		core->random->flags->set_C_OUT(TriState::One);
		core->random->flags->set_Z_OUT(TriState::One);
		core->random->flags->set_I_OUT(TriState::One);
		core->random->flags->set_D_OUT(TriState::Zero);
		core->random->flags->set_V_OUT(TriState::One);
		core->random->flags->set_N_OUT(TriState::One);
		core->wire.BRK6E = TriState::Zero;
		core->cmd.P_DB = 1;
		core->DB = 0xFF;
		core->DB_Dirty = false;
		core->random->flags->sim_Store();
		if (core->DB != 0xE7 || !core->DB_Dirty)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Flags_Test failed! sim_Store DB=0x%02X, expected 0xE7\n", core->DB);
			Logger::WriteMessage(msg);
			return false;
		}

		Logger::WriteMessage("Flags_Test OK!\n");
		return true;
	}

	bool UnitTest::Regs_Test()
	{
		// Setters / getters

		core->regs->setY(0x12);
		core->regs->setX(0x34);
		core->regs->setS(0x56);
		if (core->regs->getY() != 0x12 || core->regs->getX() != 0x34 || core->regs->getS() != 0x56)
		{
			Logger::WriteMessage("Regs_Test failed! setY/setX/setS or getters broken\n");
			return false;
		}

		// sim_LoadSB: SB_Y, SB_X load Y and X from the SB bus.

		core->cmd.SB_Y = 1;
		core->cmd.SB_X = 1;
		core->cmd.SB_S = 0;
		core->cmd.S_S = 0;
		core->SB = 0x42;
		core->wire.PHI2 = TriState::Zero;
		core->regs->sim_LoadSB();
		if (core->regs->getY() != 0x42 || core->regs->getX() != 0x42)
		{
			Logger::WriteMessage("Regs_Test failed! Y/X not loaded from SB\n");
			return false;
		}

		// S_S refreshes S_in from S_out.

		core->regs->setS(0x50);		// S_out = ~0x50
		core->cmd.SB_Y = 0;
		core->cmd.SB_X = 0;
		core->cmd.SB_S = 0;
		core->cmd.S_S = 1;
		core->wire.PHI2 = TriState::Zero;
		core->regs->sim_LoadSB();	// S_in = ~S_out = 0x50
		core->wire.PHI2 = TriState::One;
		core->regs->sim_LoadSB();	// S_out = ~S_in = ~0x50
		if (core->regs->getS() != 0x50)
		{
			Logger::WriteMessage("Regs_Test failed! S_S refresh broken\n");
			return false;
		}

		// SB_S loads S_in directly from the SB bus.

		core->regs->setS(0x00);
		core->cmd.SB_S = 1;
		core->cmd.S_S = 0;
		core->SB = 0x77;
		core->wire.PHI2 = TriState::Zero;
		core->regs->sim_LoadSB();
		core->wire.PHI2 = TriState::One;
		core->regs->sim_LoadSB();
		if (core->regs->getS() != 0x77)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Regs_Test failed! SB_S load broken, S=0x%02X\n", core->regs->getS());
			Logger::WriteMessage(msg);
			return false;
		}

		// sim_StoreSB: S_SB, Y_SB, X_SB place the registers on the SB bus ("Ground wins" rule).
		// S=0x50, Y=0x55, X=0x33 -> SB = 0x50 & 0x55 & 0x33 = 0x10

		core->regs->setY(0x55);
		core->regs->setX(0x33);
		core->regs->setS(0x50);		// S_out = 0xAF
		core->cmd.S_S = 1;
		core->cmd.SB_Y = 0;
		core->cmd.SB_X = 0;
		core->cmd.SB_S = 0;
		core->wire.PHI2 = TriState::Zero;
		core->regs->sim_LoadSB();	// S_in = ~S_out = 0x50
		core->cmd.S_S = 0;
		core->wire.PHI2 = TriState::One;
		core->regs->sim_LoadSB();	// S_out = ~S_in = 0xAF

		core->SB = 0xFF;
		core->SB_Dirty = false;
		core->cmd.Y_SB = 1;
		core->cmd.X_SB = 1;
		core->cmd.S_SB = 1;
		core->wire.PHI2 = TriState::One;
		core->regs->sim_StoreSB();
		if (core->SB != 0x10 || !core->SB_Dirty)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Regs_Test failed! StoreSB SB=0x%02X, expected 0x10\n", core->SB);
			Logger::WriteMessage(msg);
			return false;
		}

		// sim_StoreOldS: S_ADL places ~S_out on the ADL bus.

		core->ADL = 0xFF;
		core->ADL_Dirty = false;
		core->cmd.S_ADL = 1;
		core->regs->sim_StoreOldS();
		if (core->ADL != 0x50 || !core->ADL_Dirty)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Regs_Test failed! StoreOldS ADL=0x%02X, expected 0x50\n", core->ADL);
			Logger::WriteMessage(msg);
			return false;
		}

		Logger::WriteMessage("Regs_Test OK!\n");
		return true;
	}

	bool UnitTest::ALU_Extras_Test()
	{
		// Getters / setters

		core->alu->setAI(0x11);
		core->alu->setBI(0x22);
		core->alu->setADD(0x33);
		core->alu->setAC(0x44);
		if (core->alu->getAI() != 0x11 || core->alu->getBI() != 0x22 ||
			core->alu->getADD() != 0x33 || core->alu->getAC() != 0x44)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! getters/setters broken\n");
			return false;
		}

		core->alu->setAVR(TriState::One);
		if (core->alu->getAVR() != TriState::One)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! setAVR(One)/getAVR broken\n");
			return false;
		}
		core->alu->setAVR(TriState::Zero);
		if (core->alu->getAVR() != TriState::Zero)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! setAVR(Zero)/getAVR broken\n");
			return false;
		}

		// sim_Load: NDB_ADD, ADL_ADD, SB_ADD, Z_ADD, and the SB_ADD+Z_ADD special case.

		core->cmd.NDB_ADD = 1;
		core->DB = 0x5A;
		core->DB_Dirty = false;
		core->alu->sim_Load();
		if (core->alu->getBI() != 0xA5)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "ALU_Extras_Test failed! NDB_ADD BI=0x%02X, expected 0xA5\n", core->alu->getBI());
			Logger::WriteMessage(msg);
			return false;
		}

		core->cmd.NDB_ADD = 0;
		core->cmd.ADL_ADD = 1;
		core->ADL = 0x3C;
		core->alu->sim_Load();
		if (core->alu->getBI() != 0x3C)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! ADL_ADD load broken\n");
			return false;
		}

		core->cmd.ADL_ADD = 0;
		core->cmd.SB_ADD = 1;
		core->SB = 0x27;
		core->SB_Dirty = false;
		core->alu->sim_Load();
		if (core->alu->getAI() != 0x27)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! SB_ADD load broken\n");
			return false;
		}

		core->cmd.Z_ADD = 1;
		core->SB = 0xAB;
		core->SB_Dirty = false;
		core->alu->sim_Load();
		if (core->SB != 0 || !core->SB_Dirty || core->alu->getAI() != 0)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! SB_ADD+Z_ADD special case broken\n");
			return false;
		}

		// sim_BusMux: SB_DB, SB_ADH with the "Ground wins" rule.

		core->cmd.SB_DB = 1;
		core->cmd.SB_ADH = 0;
		core->SB = 0;
		core->DB = 0x33;
		core->SB_Dirty = false;
		core->DB_Dirty = true;
		core->alu->sim_BusMux();
		if (core->SB != 0x33 || !core->SB_Dirty)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! SB_DB (DB->SB) broken\n");
			return false;
		}

		core->SB = 0x55;
		core->DB = 0x0F;
		core->SB_Dirty = true;
		core->DB_Dirty = true;
		core->alu->sim_BusMux();
		if (core->SB != 0x05 || core->DB != 0x05)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! SB_DB (AND) broken\n");
			return false;
		}

		core->cmd.SB_DB = 0;
		core->cmd.SB_ADH = 1;
		core->SB = 0x3C;
		core->ADH = 0;
		core->SB_Dirty = true;
		core->ADH_Dirty = false;
		core->alu->sim_BusMux();
		if (core->ADH != 0x3C || !core->ADH_Dirty)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! SB_ADH (SB->ADH) broken\n");
			return false;
		}

		// sim_HLE: SUMS. The test core has BCD_Hack=false, so sim_HLE would normally
		// fall back to sim(). Enable the BCD hack to force the fast HLE path.

		core->alu->SetBCDHack(true);
		core->cmd.SUMS = 1;
		core->cmd.SB_AC = 0;
		core->cmd.n_ACIN = 1;	// no carry in
		core->wire.PHI2 = TriState::One;
		core->alu->setAI(0x11);
		core->alu->setBI(0x22);
		core->alu->sim_HLE();
		if (core->alu->getADD() != 0x33)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "ALU_Extras_Test failed! sim_HLE SUMS ADD=0x%02X, expected 0x33\n", core->alu->getADD());
			Logger::WriteMessage(msg);
			return false;
		}

		// sim_HLE: SB_AC loads the accumulator from the SB bus.

		core->cmd.SB_AC = 1;
		core->SB = 0x33;
		core->SB_Dirty = false;
		core->wire.PHI2 = TriState::Zero;
		core->alu->sim_HLE();
		if (core->alu->getAC() != 0x33)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! sim_HLE SB_AC load broken\n");
			return false;
		}

		// sim_HLE: carry out (0xFF + 0x01 = 0x100).

		core->cmd.SUMS = 1;
		core->cmd.n_ACIN = 1;
		core->wire.PHI2 = TriState::One;
		core->alu->setAI(0xFF);
		core->alu->setBI(0x01);
		core->alu->sim_HLE();
		if (core->alu->getACR() != TriState::One)
		{
			Logger::WriteMessage("ALU_Extras_Test failed! sim_HLE ACR (carry out) broken\n");
			return false;
		}
		core->alu->SetBCDHack(false);

		// SetBCDHack(true): the BCD correction is disabled in the regular sim() as well.

		core->alu->SetBCDHack(true);
		core->cmd.SUMS = 1;
		core->cmd.n_ACIN = 1;
		core->cmd.n_DAA = 0;	// would enable BCD correction, but the hack overrides it
		core->cmd.n_DSA = 0;
		core->wire.PHI2 = TriState::One;
		core->alu->setAI(0x19);
		core->alu->setBI(0x01);
		core->alu->sim();
		if (core->alu->getADD() != 0x1A)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "ALU_Extras_Test failed! sim() with BCD hack ADD=0x%02X, expected 0x1A\n", core->alu->getADD());
			Logger::WriteMessage(msg);
			return false;
		}
		core->alu->SetBCDHack(false);

		// SetBCDHack(true) also selects the fast HLE path in sim_HLE.

		core->alu->SetBCDHack(true);
		core->wire.PHI2 = TriState::One;
		core->alu->setAI(0x19);
		core->alu->setBI(0x01);
		core->alu->sim_HLE();
		if (core->alu->getADD() != 0x1A)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "ALU_Extras_Test failed! sim_HLE with BCD hack ADD=0x%02X, expected 0x1A\n", core->alu->getADD());
			Logger::WriteMessage(msg);
			return false;
		}
		core->alu->SetBCDHack(false);

		Logger::WriteMessage("ALU_Extras_Test OK!\n");
		return true;
	}

	bool UnitTest::PC_HLE_Test()
	{
		// The test core is in HLE mode, so the ProgramCounter uses the HLE paths.

		// Setters / getters

		core->pc->setPCL(0x34);
		core->pc->setPCH(0x12);
		core->pc->setPCLS(0xAB);
		core->pc->setPCHS(0xCD);
		if (core->pc->getPCL() != 0x34 || core->pc->getPCH() != 0x12 ||
			core->pc->getPCLS() != 0xAB || core->pc->getPCHS() != 0xCD)
		{
			Logger::WriteMessage("PC_HLE_Test failed! setters/getters broken\n");
			return false;
		}

		// sim_LoadHLE: ADL_PCL, ADH_PCH, PCL_PCL, PCH_PCH

		core->pc->setPCL(0x44);
		core->pc->setPCH(0x22);

		core->cmd.ADL_PCL = 1;
		core->cmd.PCL_PCL = 0;
		core->ADL = 0x55;
		core->pc->sim_LoadHLE();
		if (core->pc->getPCLS() != 0x55)
		{
			Logger::WriteMessage("PC_HLE_Test failed! ADL_PCL load broken\n");
			return false;
		}

		core->cmd.ADL_PCL = 0;
		core->cmd.PCL_PCL = 1;
		core->pc->sim_LoadHLE();
		if (core->pc->getPCLS() != 0x44)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCL_PCL load broken\n");
			return false;
		}

		core->cmd.ADH_PCH = 1;
		core->cmd.PCH_PCH = 0;
		core->ADH = 0x66;
		core->pc->sim_LoadHLE();
		if (core->pc->getPCHS() != 0x66)
		{
			Logger::WriteMessage("PC_HLE_Test failed! ADH_PCH load broken\n");
			return false;
		}

		core->cmd.ADH_PCH = 0;
		core->cmd.PCH_PCH = 1;
		core->pc->sim_LoadHLE();
		if (core->pc->getPCHS() != 0x22)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCH_PCH load broken\n");
			return false;
		}

		// sim_HLE: increment (n_1PC = 0), 0x00FF -> 0x0100.

		core->pc->setPCLS(0xFF);
		core->pc->setPCHS(0x00);
		core->pc->setPCL(0x00);
		core->pc->setPCH(0x00);
		core->wire.PHI2 = TriState::One;
		core->wire.n_1PC = TriState::Zero;
		core->pc->sim_HLE();
		if (core->pc->getPCL() != 0x00 || core->pc->getPCH() != 0x01)
		{
			Logger::WriteMessage("PC_HLE_Test failed! sim_HLE increment (0x00FF->0x0100) broken\n");
			return false;
		}

		// sim_HLE: no increment (n_1PC = 1).

		core->pc->setPCLS(0x34);
		core->pc->setPCHS(0x12);
		core->wire.n_1PC = TriState::One;
		core->pc->sim_HLE();
		if (core->pc->getPCL() != 0x34 || core->pc->getPCH() != 0x12)
		{
			Logger::WriteMessage("PC_HLE_Test failed! sim_HLE no-increment broken\n");
			return false;
		}

		// sim_HLE: PHI2 = 0 does not change the PC.

		core->pc->setPCLS(0x78);
		core->pc->setPCHS(0x56);
		core->wire.PHI2 = TriState::Zero;
		core->wire.n_1PC = TriState::Zero;
		core->pc->sim_HLE();
		if (core->pc->getPCL() != 0x34 || core->pc->getPCH() != 0x12)
		{
			Logger::WriteMessage("PC_HLE_Test failed! sim_HLE PHI2=0 changed the PC\n");
			return false;
		}

		// sim_StoreHLE: PCL_ADL, PCL_DB, PCH_ADH, PCH_DB.

		core->pc->setPCL(0x77);
		core->pc->setPCH(0x88);

		core->cmd.PCL_ADL = 1;
		core->ADL = 0;
		core->ADL_Dirty = false;
		core->pc->sim_StoreHLE();
		if (core->ADL != 0x77 || !core->ADL_Dirty)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCL_ADL store broken\n");
			return false;
		}

		core->cmd.PCL_ADL = 0;
		core->cmd.PCL_DB = 1;
		core->DB = 0;
		core->DB_Dirty = false;
		core->pc->sim_StoreHLE();
		if (core->DB != 0x77 || !core->DB_Dirty)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCL_DB store broken\n");
			return false;
		}

		core->cmd.PCH_ADH = 1;
		core->ADH = 0;
		core->ADH_Dirty = false;
		core->pc->sim_StoreHLE();
		if (core->ADH != 0x88 || !core->ADH_Dirty)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCH_ADH store broken\n");
			return false;
		}

		core->cmd.PCL_ADL = 0;
		core->cmd.PCL_DB = 0;
		core->cmd.PCH_ADH = 0;
		core->cmd.PCH_DB = 1;
		core->DB = 0;
		core->DB_Dirty = false;
		core->pc->sim_StoreHLE();
		if (core->DB != 0x88 || !core->DB_Dirty)
		{
			Logger::WriteMessage("PC_HLE_Test failed! PCH_DB store broken\n");
			return false;
		}

		Logger::WriteMessage("PC_HLE_Test OK!\n");
		return true;
	}

	bool UnitTest::DataBus_Test()
	{
		// Setters / getters

		core->data_bus->setDL(0x12);
		core->data_bus->setDOR(0x34);
		if (core->data_bus->getDL() != 0x12 || core->data_bus->getDOR() != 0x34)
		{
			Logger::WriteMessage("DataBus_Test failed! setters/getters broken\n");
			return false;
		}

		// sim_GetExternalBus: DL latches the external bus during PHI2.

		uint8_t ext = 0x5A;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->data_bus->sim_GetExternalBus(&ext);
		if (core->data_bus->getDL() != 0x5A)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "DataBus_Test failed! getDL=0x%02X, expected 0x5A\n", core->data_bus->getDL());
			Logger::WriteMessage(msg);
			return false;
		}

		// DL_ADL, DL_ADH, DL_DB are applied to the internal buses during PHI1.

		core->cmd.DL_ADL = 1;
		core->ADL = 0xFF;
		core->ADL_Dirty = false;
		core->cmd.DL_ADH = 1;
		core->ADH = 0xFF;
		core->ADH_Dirty = false;
		core->cmd.DL_DB = 1;
		core->DB = 0xFF;
		core->DB_Dirty = false;
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->data_bus->sim_GetExternalBus(&ext);
		if (core->ADL != 0x5A || core->ADH != 0x5A || core->DB != 0x5A)
		{
			Logger::WriteMessage("DataBus_Test failed! DL_ADL/DL_ADH/DL_DB broken\n");
			return false;
		}

		// A dirty bus ANDs with ~DL.

		core->ADL = 0xF0;
		core->ADL_Dirty = true;
		core->data_bus->sim_GetExternalBus(&ext);
		if (core->ADL != 0x50)
		{
			Logger::WriteMessage("DataBus_Test failed! DL_ADL with dirty ADL broken\n");
			return false;
		}

		// sim_SetExternalBus: DOR is latched from DB during PHI1, and the bus is driven
		// with ~DOR during PHI2 of the write cycle (RD=0 when WR=1 and PHI2=1).

		core->DB = 0x34;
		core->DB_Dirty = false;
		core->wire.WR = TriState::One;
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		uint8_t out_bus = 0xFF;
		core->data_bus->sim_SetExternalBus(&out_bus);	// latches WR into rd_latch, DOR = ~DB
		if (core->data_bus->getDOR() != 0x34)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "DataBus_Test failed! getDOR=0x%02X, expected 0x34\n", core->data_bus->getDOR());
			Logger::WriteMessage(msg);
			return false;
		}
		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->data_bus->sim_SetExternalBus(&out_bus);	// RD=0 -> drive the bus with ~DOR
		if (out_bus != 0x34)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "DataBus_Test failed! external bus=0x%02X, expected 0x34 (write)\n", out_bus);
			Logger::WriteMessage(msg);
			return false;
		}

		Logger::WriteMessage("DataBus_Test OK!\n");
		return true;
	}

	bool UnitTest::AddressBus_Test()
	{
		// Setters / getters

		core->addr_bus->setABL(0x12);
		core->addr_bus->setABH(0x34);
		if (core->addr_bus->getABL() != 0x12 || core->addr_bus->getABH() != 0x34)
		{
			Logger::WriteMessage("AddressBus_Test failed! setters/getters broken\n");
			return false;
		}

		// sim_ConstGen: forced clearing of the ADL/ADH bits.

		core->ADL = 0xFF;
		core->ADH = 0xFF;
		core->cmd.Z_ADL0 = 1;
		core->cmd.Z_ADL1 = 1;
		core->cmd.Z_ADL2 = 1;
		core->cmd.Z_ADH0 = 1;
		core->cmd.Z_ADH17 = 1;
		core->addr_bus->sim_ConstGen();
		if (core->ADL != 0xF8)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "AddressBus_Test failed! ADL=0x%02X, expected 0xF8\n", core->ADL);
			Logger::WriteMessage(msg);
			return false;
		}
		if (core->ADH != 0x00)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "AddressBus_Test failed! ADH=0x%02X, expected 0x00\n", core->ADH);
			Logger::WriteMessage(msg);
			return false;
		}

		// sim_Output: the address bus is set during PHI1.

		uint16_t addr = 0;
		core->ADL = 0x56;
		core->ADH = 0x78;
		core->cmd.ADL_ABL = 1;
		core->cmd.ADH_ABH = 1;
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->addr_bus->sim_Output(&addr);
		if (addr != 0x7856 || core->addr_bus->getABL() != 0x56 || core->addr_bus->getABH() != 0x78)
		{
			Logger::WriteMessage("AddressBus_Test failed! sim_Output broken\n");
			return false;
		}

		// During PHI2 the bus output does not change.

		core->ADL = 0x11;
		core->ADH = 0x22;
		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->addr_bus->sim_Output(&addr);
		if (addr != 0x7856)
		{
			Logger::WriteMessage("AddressBus_Test failed! sim_Output changed during PHI2\n");
			return false;
		}

		Logger::WriteMessage("AddressBus_Test OK!\n");
		return true;
	}

	bool UnitTest::BRK_Test()
	{
		// Initial states of the interrupt latches.

		if (core->brk->getDORES() != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! initial DORES != 0\n");
			return false;
		}
		if (core->brk->getn_BRK6_LATCH2() != TriState::One)
		{
			Logger::WriteMessage("BRK_Test failed! initial n_BRK6_LATCH2 != 1\n");
			return false;
		}
		if (core->brk->getB_OUT(TriState::Zero) != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! initial B_OUT != 0\n");
			return false;
		}

		// BRK5 = 1 (decoder output 22) with n_ready = 0 starts the BRK6 sequence:
		// after two full cycles BRK6E becomes 1 and BRK7 becomes 0.

		TriState d[130]{};
		d[22] = TriState::One;
		core->decoder_out = d;
		core->wire.n_ready = TriState::Zero;
		core->wire.RESP = TriState::Zero;
		core->wire.n_NMIP = TriState::One;

		// Phase 1: PHI1

		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->brk->sim_BeforeRandom();
		if (core->wire.BRK6E != TriState::Zero || core->wire.BRK7 != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! phase 1 BRK6E/BRK7 mismatch\n");
			return false;
		}

		// Phase 2: PHI2

		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->brk->sim_BeforeRandom();
		if (core->wire.BRK6E != TriState::Zero || core->wire.BRK7 != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! phase 2 BRK6E/BRK7 mismatch\n");
			return false;
		}

		// Phase 3: PHI1

		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->brk->sim_BeforeRandom();
		if (core->wire.BRK6E != TriState::Zero || core->wire.BRK7 != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! phase 3 BRK6E/BRK7 mismatch\n");
			return false;
		}

		// Phase 4: PHI2 -> BRK6E = 1

		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->brk->sim_BeforeRandom();
		if (core->wire.BRK6E != TriState::One)
		{
			Logger::WriteMessage("BRK_Test failed! BRK6E != 1 after the BRK5 sequence\n");
			return false;
		}
		if (core->wire.BRK7 != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! BRK7 != 0 after the BRK5 sequence\n");
			return false;
		}
		if (core->brk->getn_BRK6_LATCH2() != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! n_BRK6_LATCH2 != 0 after the BRK5 sequence\n");
			return false;
		}

		// DORES: with RESP=1 latched, DORES=1.

		core->wire.RESP = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->brk->sim_BeforeRandom();	// res_latch1.set(RESP=1, PHI2)
		if (core->brk->getDORES() != TriState::One)
		{
			Logger::WriteMessage("BRK_Test failed! DORES != 1 with RESP latched\n");
			return false;
		}

		// DORES: releasing RESP while BRK6E=1 clears DORES.

		core->wire.RESP = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->wire.PHI2 = TriState::Zero;
		core->brk->sim_BeforeRandom();	// res_latch2.set(0, PHI1) because BRK6E=1
		core->wire.PHI1 = TriState::Zero;
		core->wire.PHI2 = TriState::One;
		core->brk->sim_BeforeRandom();	// res_latch1.set(RESP=0, PHI2)
		if (core->brk->getDORES() != TriState::Zero)
		{
			Logger::WriteMessage("BRK_Test failed! DORES != 0 after releasing RESP\n");
			return false;
		}

		Logger::WriteMessage("BRK_Test OK!\n");
		return true;
	}

	bool UnitTest::RegsControl_Test()
	{
		// The register control logic is driven by a precomputed table indexed by
		// (IROut, TxBits, n_ready, nready_latch). We drive it directly with the real
		// decoder state for known opcodes and T-states and check the resulting commands.
		// The expected values were verified against the full-chip simulation.
		// T0 state: n_T0=0, all other n_Tx=1 -> TxBits = 0x3E.
		// T1X state: n_T1X=0, all other n_Tx=1 -> TxBits = 0x3D.
		// T2 state: n_T2=0, all other n_Tx=1 -> TxBits = 0x3B.
		// T3 state: n_T3=0, all other n_Tx=1 -> TxBits = 0x37.

		auto SetState = [this](uint8_t opcode, size_t TxBits) {
			DecoderInput di{};
			di.n_T1X = FromByte((TxBits >> 1) & 1);
			di.n_T0 = FromByte((TxBits >> 0) & 1);
			di.n_T2 = FromByte((TxBits >> 2) & 1);
			di.n_T3 = FromByte((TxBits >> 3) & 1);
			di.n_T4 = FromByte((TxBits >> 4) & 1);
			di.n_T5 = FromByte((TxBits >> 5) & 1);

			uint8_t ir = opcode;
			di.n_IR0 = NOT(FromByte(ir & 1));
			di.n_IR1 = NOT(FromByte((ir >> 1) & 1));
			di.IR01 = OR(FromByte(ir & 1), FromByte((ir >> 1) & 1));
			di.n_IR2 = NOT(FromByte((ir >> 2) & 1));
			di.IR2 = FromByte((ir >> 2) & 1);
			di.n_IR3 = NOT(FromByte((ir >> 3) & 1));
			di.IR3 = FromByte((ir >> 3) & 1);
			di.n_IR4 = NOT(FromByte((ir >> 4) & 1));
			di.IR4 = FromByte((ir >> 4) & 1);
			di.n_IR5 = NOT(FromByte((ir >> 5) & 1));
			di.IR5 = FromByte((ir >> 5) & 1);
			di.n_IR6 = NOT(FromByte((ir >> 6) & 1));
			di.IR6 = FromByte((ir >> 6) & 1);
			di.n_IR7 = NOT(FromByte((ir >> 7) & 1));
			di.IR7 = FromByte((ir >> 7) & 1);

			core->decoder->sim(di.packed_bits, &core->decoder_out);
			core->ir->IROut = opcode;
			core->TxBits = TxBits;
			core->wire.n_ready = TriState::Zero;
		};

		auto RunLatch = [this]() {
			core->wire.PHI1 = TriState::Zero;
			core->wire.PHI2 = TriState::One;
			core->random->regs_control->sim();	// latch phase
			core->wire.PHI1 = TriState::One;
			core->wire.PHI2 = TriState::Zero;
			core->random->regs_control->sim();	// output phase
		};

		// TXS (0x9A) @T0: X_SB=1, SB_S=1, S_S=0.

		SetState(0x9A, 0x3E);
		RunLatch();
		if (core->cmd.X_SB != 1 || core->cmd.SB_S != 1 || core->cmd.S_S != 0)
		{
			char msg[0x200];
			sprintf_s(msg, sizeof(msg), "RegsControl_Test failed! TXS@T0: X_SB=%d SB_S=%d S_S=%d\n", core->cmd.X_SB, core->cmd.SB_S, core->cmd.S_S);
			Logger::WriteMessage(msg);
			return false;
		}
		if (core->cmd.Y_SB != 0 || core->cmd.SB_Y != 0 || core->cmd.SB_X != 0 || core->cmd.S_SB != 0 || core->cmd.S_ADL != 0)
		{
			Logger::WriteMessage("RegsControl_Test failed! TXS@T0: unexpected extra commands\n");
			return false;
		}

		// TSX (0xBA) @T0: SB_X=1, S_SB=1, S_S=1.

		SetState(0xBA, 0x3E);
		RunLatch();
		if (core->cmd.SB_X != 1 || core->cmd.S_SB != 1 || core->cmd.S_S != 1)
		{
			char msg[0x200];
			sprintf_s(msg, sizeof(msg), "RegsControl_Test failed! TSX@T0: SB_X=%d S_SB=%d S_S=%d\n", core->cmd.SB_X, core->cmd.S_SB, core->cmd.S_S);
			Logger::WriteMessage(msg);
			return false;
		}
		if (core->cmd.Y_SB != 0 || core->cmd.SB_Y != 0 || core->cmd.X_SB != 0 || core->cmd.SB_S != 0 || core->cmd.S_ADL != 0)
		{
			Logger::WriteMessage("RegsControl_Test failed! TSX@T0: unexpected extra commands\n");
			return false;
		}

		// TAY (0xA8) @T0: SB_Y=1.

		SetState(0xA8, 0x3E);
		RunLatch();
		if (core->cmd.SB_Y != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! TAY@T0: SB_Y/S_S mismatch\n");
			return false;
		}

		// TYA (0x98) @T0: Y_SB=1.

		SetState(0x98, 0x3E);
		RunLatch();
		if (core->cmd.Y_SB != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! TYA@T0: Y_SB/S_S mismatch\n");
			return false;
		}

		// TAX (0xAA) @T0: SB_X=1.

		SetState(0xAA, 0x3E);
		RunLatch();
		if (core->cmd.SB_X != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! TAX@T0: SB_X/S_S mismatch\n");
			return false;
		}

		// LDX (0xA2) @T0: SB_X=1.

		SetState(0xA2, 0x3E);
		RunLatch();
		if (core->cmd.SB_X != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! LDX@T0: SB_X/S_S mismatch\n");
			return false;
		}

		// STY (0x84) @T2: Y_SB=1.

		SetState(0x84, 0x3B);
		RunLatch();
		if (core->cmd.Y_SB != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! STY@T2: Y_SB/S_S mismatch\n");
			return false;
		}

		// STX (0x86) @T2: X_SB=1.

		SetState(0x86, 0x3B);
		RunLatch();
		if (core->cmd.X_SB != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! STX@T2: X_SB/S_S mismatch\n");
			return false;
		}

		// PHA (0x48) @T0: SB_S=1 (the stack pointer is loaded from the SB bus).

		SetState(0x48, 0x3E);
		RunLatch();
		if (core->cmd.SB_S != 1 || core->cmd.S_S != 0)
		{
			Logger::WriteMessage("RegsControl_Test failed! PHA@T0: SB_S/S_S mismatch\n");
			return false;
		}

		// PHA (0x48) @T2: S_ADL=1 (the stack pointer goes to the ADL bus).

		SetState(0x48, 0x3B);
		RunLatch();
		if (core->cmd.S_ADL != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! PHA@T2: S_ADL/S_S mismatch\n");
			return false;
		}

		// PLA (0x68) @T3: SB_S=1.

		SetState(0x68, 0x37);
		RunLatch();
		if (core->cmd.SB_S != 1 || core->cmd.S_S != 0)
		{
			Logger::WriteMessage("RegsControl_Test failed! PLA@T3: SB_S/S_S mismatch\n");
			return false;
		}

		// PLA (0x68) @T2: S_ADL=1.

		SetState(0x68, 0x3B);
		RunLatch();
		if (core->cmd.S_ADL != 1 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! PLA@T2: S_ADL/S_S mismatch\n");
			return false;
		}

		// NOP (0xEA) @T0 and @T1X: no register commands, only the S refresh (S_S=1).

		SetState(0xEA, 0x3E);
		RunLatch();
		if (core->cmd.Y_SB != 0 || core->cmd.SB_Y != 0 || core->cmd.X_SB != 0 || core->cmd.SB_X != 0 ||
			core->cmd.S_ADL != 0 || core->cmd.S_SB != 0 || core->cmd.SB_S != 0 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! NOP@T0: unexpected commands\n");
			return false;
		}

		SetState(0xEA, 0x3D);
		RunLatch();
		if (core->cmd.Y_SB != 0 || core->cmd.SB_Y != 0 || core->cmd.X_SB != 0 || core->cmd.SB_X != 0 ||
			core->cmd.S_ADL != 0 || core->cmd.S_SB != 0 || core->cmd.SB_S != 0 || core->cmd.S_S != 1)
		{
			Logger::WriteMessage("RegsControl_Test failed! NOP@T1X: unexpected commands\n");
			return false;
		}

		Logger::WriteMessage("RegsControl_Test OK!\n");
		return true;
	}

	bool UnitTest::BusControl_Test()
	{
		// Direct smoke test: with a fully zero decoder output the bus control must still
		// produce valid commands and must not crash. The exact values are covered by the
		// full-chip integration test.

		TriState d[130]{};
		core->decoder_out = d;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->wire.n_ready = TriState::Zero;
		core->wire.RMW_T6 = TriState::Zero;
		core->wire.RMW_T7 = TriState::Zero;
		core->wire.n_PRDY = TriState::One;
		core->wire.T0 = TriState::Zero;
		core->wire.BRK6E = TriState::Zero;
		core->wire.ACRL2 = TriState::Zero;
		core->cmd.Z_ADL0 = 0;
		core->ir->IROut = 0;
		core->random->bus_control->sim();
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->random->bus_control->sim();

		if (core->cmd.ADL_ABL > 1 || core->cmd.ADH_ABH > 1 || core->cmd.SB_DB > 1 || core->cmd.SB_ADH > 1 ||
			core->cmd.DL_ADL > 1 || core->cmd.DL_ADH > 1 || core->cmd.DL_DB > 1 ||
			core->cmd.Z_ADL0 > 1 || core->cmd.Z_ADL1 > 1 || core->cmd.Z_ADL2 > 1 ||
			core->cmd.Z_ADH0 > 1 || core->cmd.Z_ADH17 > 1 ||
			core->cmd.SB_AC > 1 || core->cmd.AC_SB > 1 || core->cmd.AC_DB > 1)
		{
			Logger::WriteMessage("BusControl_Test failed! invalid command values with zero decoder output\n");
			return false;
		}

		// Integration spot checks are done in CoreInternals_Integration_Test.

		Logger::WriteMessage("BusControl_Test OK!\n");
		return true;
	}

	bool UnitTest::PCControl_Test()
	{
		// Direct smoke test: the PC control produces valid commands and does not crash.
		// The exact values are covered by the full-chip integration test.
		// Note: the dispatcher is never run here, so T1 = getT1() keeps its initial value (1).

		TriState d[130]{};
		core->decoder_out = d;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->wire.n_ready = TriState::Zero;
		core->wire.T0 = TriState::Zero;
		core->wire.n_PRDY = TriState::One;
		core->random->pc_control->sim();
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->random->pc_control->sim();

		if (core->cmd.PCL_DB > 1 || core->cmd.PCH_DB > 1 || core->cmd.PCL_ADL > 1 || core->cmd.PCH_ADH > 1 ||
			core->cmd.ADL_PCL > 1 || core->cmd.PCL_PCL > 1 || core->cmd.ADH_PCH > 1 || core->cmd.PCH_PCH > 1)
		{
			Logger::WriteMessage("PCControl_Test failed! invalid command values with zero decoder output\n");
			return false;
		}

		Logger::WriteMessage("PCControl_Test OK!\n");
		return true;
	}

	bool UnitTest::FlagsControl_Test()
	{
		// The IR5_I/IR5_C/IR5_D/Z_V/AVR_V commands are driven directly by decoder
		// outputs: IR5_I = d[108], IR5_C = d[110], IR5_D = d[120], Z_V = d[127],
		// AVR_V = d[112]. Check them with a synthetic decoder output.

		TriState d[130]{};
		d[108] = TriState::One;
		d[110] = TriState::One;
		d[120] = TriState::One;
		d[127] = TriState::One;
		d[112] = TriState::One;
		core->decoder_out = d;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->wire.n_ready = TriState::Zero;
		core->wire.RMW_T6 = TriState::Zero;
		core->wire.RMW_T7 = TriState::Zero;
		core->ir->IROut = 0;
		core->TxBits = 0x3F;
		core->random->flags_control->sim();

		if (core->cmd.IR5_I != 1 || core->cmd.IR5_C != 1 || core->cmd.IR5_D != 1 || core->cmd.Z_V != 1 || core->cmd.AVR_V != 1)
		{
			char msg[0x200];
			sprintf_s(msg, sizeof(msg), "FlagsControl_Test failed! IR5_I=%d IR5_C=%d IR5_D=%d Z_V=%d AVR_V=%d\n",
				core->cmd.IR5_I, core->cmd.IR5_C, core->cmd.IR5_D, core->cmd.Z_V, core->cmd.AVR_V);
			Logger::WriteMessage(msg);
			return false;
		}

		// With all-zero decoder output the commands must be valid and must not crash.

		TriState d2[130]{};
		core->decoder_out = d2;
		core->ir->IROut = 0;
		core->TxBits = 0x3F;
		core->random->flags_control->sim();
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->random->flags_control->sim();

		if (core->cmd.P_DB > 1 || core->cmd.DB_P > 1 || core->cmd.DBZ_Z > 1 || core->cmd.DB_N > 1 ||
			core->cmd.IR5_C > 1 || core->cmd.DB_C > 1 || core->cmd.ACR_C > 1 ||
			core->cmd.IR5_D > 1 || core->cmd.IR5_I > 1 || core->cmd.DB_V > 1 || core->cmd.AVR_V > 1 || core->cmd.Z_V > 1)
		{
			Logger::WriteMessage("FlagsControl_Test failed! invalid command values\n");
			return false;
		}

		Logger::WriteMessage("FlagsControl_Test OK!\n");
		return true;
	}

	bool UnitTest::ALUControl_Test()
	{
		// Direct smoke test: the ALU control produces valid commands and does not crash.
		// The exact values are covered by the full-chip integration test.

		TriState d[130]{};
		core->decoder_out = d;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->wire.n_ready = TriState::Zero;
		core->wire.T0 = TriState::One;
		core->wire.RMW_T6 = TriState::Zero;
		core->wire.RMW_T7 = TriState::Zero;
		core->wire.BRK6E = TriState::Zero;
		core->wire.n_PRDY = TriState::One;
		core->ir->IROut = 0;
		core->TxBits = 0x3E;
		core->random->flags->set_C_OUT(TriState::Zero);	// n_C_OUT = 1
		core->random->alu_control->sim();
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->random->alu_control->sim();

		if (core->cmd.NDB_ADD > 1 || core->cmd.DB_ADD > 1 || core->cmd.Z_ADD > 1 || core->cmd.SB_ADD > 1 || core->cmd.ADL_ADD > 1 ||
			core->cmd.ANDS > 1 || core->cmd.EORS > 1 || core->cmd.ORS > 1 || core->cmd.SRS > 1 || core->cmd.SUMS > 1 ||
			core->cmd.n_ACIN > 1 || core->cmd.n_DAA > 1 || core->cmd.n_DSA > 1 ||
			core->cmd.ADD_SB7 > 1 || core->cmd.ADD_SB06 > 1 || core->cmd.ADD_ADL > 1)
		{
			Logger::WriteMessage("ALUControl_Test failed! invalid command values\n");
			return false;
		}

		Logger::WriteMessage("ALUControl_Test OK!\n");
		return true;
	}

	bool UnitTest::BranchLogic_Test()
	{
		// Direct smoke test: the branch logic produces valid outputs and does not crash.
		// The exact values are covered by the full-chip integration test.

		TriState d[130]{};
		core->decoder_out = d;
		core->wire.PHI2 = TriState::One;
		core->wire.PHI1 = TriState::Zero;
		core->DB = 0;
		core->wire.n_IR5 = TriState::Zero;
		core->random->flags->set_Z_OUT(TriState::One);
		core->random->flags->set_C_OUT(TriState::Zero);
		core->random->flags->set_N_OUT(TriState::Zero);
		core->random->flags->set_V_OUT(TriState::Zero);
		core->random->branch_logic->sim();
		core->wire.PHI2 = TriState::Zero;
		core->wire.PHI1 = TriState::One;
		core->random->branch_logic->sim();

		if (core->wire.BRFW > 1 || (core->wire.n_BRTAKEN != TriState::Zero && core->wire.n_BRTAKEN != TriState::One))
		{
			Logger::WriteMessage("BranchLogic_Test failed! invalid outputs\n");
			return false;
		}

		Logger::WriteMessage("BranchLogic_Test OK!\n");
		return true;
	}

	bool UnitTest::Dispatcher_Test()
	{
		// Full-chip: the dispatcher controls the instruction cycle. We run an RMW
		// instruction (INC $10) and check the T6/T7, WR, ENDX and n_1PC signals,
		// plus the general validity of the dispatcher outputs.

		M6502 core_local(true, false);
		uint8_t prog[] = { 0xE6, 0x10, 0xEA, 0xEA };
		DebugInfo info[40]{};
		RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

		// RMW_T6 = 1 during the RMW read-modify-write cycles.

		if (info[22].RMW_T6 != 1)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "Dispatcher_Test failed! RMW_T6=%d at cycle 22, expected 1\n", info[22].RMW_T6);
			Logger::WriteMessage(msg);
			return false;
		}
		if (info[23].RMW_T6 != 1 || info[23].WR != 1)
		{
			Logger::WriteMessage("Dispatcher_Test failed! RMW_T6/WR mismatch at cycle 23\n");
			return false;
		}
		if (info[24].RMW_T7 != 1 || info[24].n_ACIN != 0 || info[24].WR != 1)
		{
			Logger::WriteMessage("Dispatcher_Test failed! RMW_T7/WR mismatch at cycle 24\n");
			return false;
		}
		if (info[25].RMW_T7 != 1 || info[25].ENDX != 0)
		{
			Logger::WriteMessage("Dispatcher_Test failed! RMW_T7/ENDX mismatch at cycle 25\n");
			return false;
		}

		// All dispatcher outputs must be valid bits at every cycle.

		for (size_t n = 0; n < _countof(info); n++)
		{
			if (info[n].T0 > 1 || info[n].n_T0 > 1 || info[n].n_T1X > 1 || info[n].FETCH > 1 || info[n].n_ready > 1 ||
				info[n].ACRL1 > 1 || info[n].ACRL2 > 1 || info[n].TRES1 > 1 || info[n].n_TRESX > 1 ||
				info[n].ENDS > 1 || info[n].ENDX > 1 || info[n].WR > 1 || info[n].n_1PC > 1)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "Dispatcher_Test failed! invalid dispatcher signal at cycle %zu\n", n);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// getTRES2()/getT1() smoke.

		if (core_local.disp->getTRES2() > 1 || core_local.disp->getT1() > 1)
		{
			Logger::WriteMessage("Dispatcher_Test failed! getTRES2/getT1 invalid\n");
			return false;
		}

		// getSTOR(d): STOR = NOR(n_MemOp, n_STORE).

		TriState d[130]{};
		d[97] = TriState::One;
		d[111] = TriState::One;	// n_MemOp = 0, n_STORE = 0 -> STOR = 1
		if (core_local.disp->getSTOR(d) != TriState::One)
		{
			Logger::WriteMessage("Dispatcher_Test failed! getSTOR with memop+store != 1\n");
			return false;
		}
		d[111] = TriState::Zero;	// n_MemOp = 1, n_STORE = 0 -> STOR = 0
		if (core_local.disp->getSTOR(d) != TriState::Zero)
		{
			Logger::WriteMessage("Dispatcher_Test failed! getSTOR without memop != 0\n");
			return false;
		}
		d[97] = TriState::Zero;		// n_MemOp = 1, n_STORE = 1 -> STOR = 0
		if (core_local.disp->getSTOR(d) != TriState::Zero)
		{
			Logger::WriteMessage("Dispatcher_Test failed! getSTOR without store != 0\n");
			return false;
		}

		Logger::WriteMessage("Dispatcher_Test OK!\n");
		return true;
	}

	bool UnitTest::RandomLogic_Test()
	{
		// Full-chip: run a program with transfers and ALU operations and check that the
		// random logic produces the expected commands, and that all commands stay valid.

		M6502 core_local(true, false);
		uint8_t prog[] = { 0xA9, 0x5A, 0xAA, 0xA8, 0x98, 0x9A, 0xBA, 0xEA, 0xEA, 0xEA };
		DebugInfo info[46]{};
		RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

		// LDA #0x5A: the accumulator loads from the SB bus, flags are updated.

		if (info[22].SB_AC != 1 || info[22].DBZ_Z != 1 || info[22].DB_N != 1)
		{
			char msg[0x100];
			sprintf_s(msg, sizeof(msg), "RandomLogic_Test failed! LDA cycle 22: SB_AC=%d DBZ_Z=%d DB_N=%d\n", info[22].SB_AC, info[22].DBZ_Z, info[22].DB_N);
			Logger::WriteMessage(msg);
			return false;
		}

		// TXS: X_SB and SB_S must be active at the transfer cycle.

		if (info[38].X_SB != 1 || info[38].SB_S != 1)
		{
			Logger::WriteMessage("RandomLogic_Test failed! TXS cycle 38: X_SB/SB_S mismatch\n");
			return false;
		}

		// All commands must be valid bits at every cycle.

		for (size_t n = 0; n < _countof(info); n++)
		{
			if (info[n].Y_SB > 1 || info[n].SB_Y > 1 || info[n].X_SB > 1 || info[n].SB_X > 1 ||
				info[n].S_ADL > 1 || info[n].S_SB > 1 || info[n].SB_S > 1 || info[n].S_S > 1 ||
				info[n].NDB_ADD > 1 || info[n].DB_ADD > 1 || info[n].Z_ADD > 1 || info[n].SB_ADD > 1 || info[n].ADL_ADD > 1 ||
				info[n].ANDS > 1 || info[n].EORS > 1 || info[n].ORS > 1 || info[n].SRS > 1 || info[n].SUMS > 1 ||
				info[n].n_ACIN > 1 || info[n].n_DAA > 1 || info[n].n_DSA > 1 ||
				info[n].ADD_SB7 > 1 || info[n].ADD_SB06 > 1 || info[n].ADD_ADL > 1 ||
				info[n].SB_AC > 1 || info[n].AC_SB > 1 || info[n].AC_DB > 1 ||
				info[n].P_DB > 1 || info[n].DB_P > 1 || info[n].DBZ_Z > 1 || info[n].DB_N > 1 ||
				info[n].IR5_C > 1 || info[n].DB_C > 1 || info[n].ACR_C > 1 || info[n].IR5_D > 1 || info[n].IR5_I > 1 ||
				info[n].DB_V > 1 || info[n].AVR_V > 1 || info[n].Z_V > 1)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "RandomLogic_Test failed! invalid command at cycle %zu\n", n);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		Logger::WriteMessage("RandomLogic_Test OK!\n");
		return true;
	}

	bool UnitTest::ExtraCounter_Test()
	{
		// The extra counter produces the T2..T5 states. Run a BRK loop (7-cycle
		// instruction, so T2..T5 all occur) and check the signals are valid and
		// all four states are visited. Do this for both HLE and non-HLE modes.

		uint8_t prog[] = { 0x00 };

		// HLE mode.

		{
			M6502 core_local(true, false);
			DebugInfo info[60]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			bool t2 = false, t3 = false, t4 = false, t5 = false;
			for (size_t n = 0; n < _countof(info); n++)
			{
				if (info[n].n_T2 > 1 || info[n].n_T3 > 1 || info[n].n_T4 > 1 || info[n].n_T5 > 1)
				{
					char msg[0x100];
					sprintf_s(msg, sizeof(msg), "ExtraCounter_Test (HLE) failed! invalid n_Tx at cycle %zu\n", n);
					Logger::WriteMessage(msg);
					return false;
				}
				if (info[n].n_T2 == 0) t2 = true;
				if (info[n].n_T3 == 0) t3 = true;
				if (info[n].n_T4 == 0) t4 = true;
				if (info[n].n_T5 == 0) t5 = true;
			}
			if (!t2 || !t3 || !t4 || !t5)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "ExtraCounter_Test (HLE) failed! not all T states visited (t2=%d t3=%d t4=%d t5=%d)\n", t2, t3, t4, t5);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// Non-HLE mode.

		{
			M6502 core_local(false, false);
			DebugInfo info[60]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			bool t2 = false, t3 = false, t4 = false, t5 = false;
			for (size_t n = 0; n < _countof(info); n++)
			{
				if (info[n].n_T2 > 1 || info[n].n_T3 > 1 || info[n].n_T4 > 1 || info[n].n_T5 > 1)
				{
					char msg[0x100];
					sprintf_s(msg, sizeof(msg), "ExtraCounter_Test (non-HLE) failed! invalid n_Tx at cycle %zu\n", n);
					Logger::WriteMessage(msg);
					return false;
				}
				if (info[n].n_T2 == 0) t2 = true;
				if (info[n].n_T3 == 0) t3 = true;
				if (info[n].n_T4 == 0) t4 = true;
				if (info[n].n_T5 == 0) t5 = true;
			}
			if (!t2 || !t3 || !t4 || !t5)
			{
				char msg[0x100];
				sprintf_s(msg, sizeof(msg), "ExtraCounter_Test (non-HLE) failed! not all T states visited (t2=%d t3=%d t4=%d t5=%d)\n", t2, t3, t4, t5);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		Logger::WriteMessage("ExtraCounter_Test OK!\n");
		return true;
	}

	bool UnitTest::CoreInternals_Integration_Test()
	{
		// This test runs known programs and checks the internal commands and wires at
		// specific cycles. The expected values were captured from the full-chip
		// simulation (ground truth) and are deterministic.

		// --- Program 1: LDA #0x5A, TAX, TAY, TYA, TXS, TSX, NOP ---

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0xA9, 0x5A, 0xAA, 0xA8, 0x98, 0x9A, 0xBA, 0xEA, 0xEA, 0xEA };
			DebugInfo info[46]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// LDA #0x5A load cycle.

			if (info[22].SB_AC != 1 || info[22].DBZ_Z != 1 || info[22].DB_N != 1 || info[22].AC != 0x5A)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! LDA cycle 22: SB_AC=%d DBZ_Z=%d DB_N=%d AC=0x%02X\n",
					info[22].SB_AC, info[22].DBZ_Z, info[22].DB_N, info[22].AC);
				Logger::WriteMessage(msg);
				return false;
			}

			// TAX execute cycle: SB_X and AC_SB, X becomes 0x5A.

			if (info[26].SB_X != 1 || info[26].AC_SB != 1 || info[26].X != 0x5A)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TAX cycle 26: SB_X=%d AC_SB=%d X=0x%02X\n",
					info[26].SB_X, info[26].AC_SB, info[26].X);
				Logger::WriteMessage(msg);
				return false;
			}

			// TAY execute cycle: SB_Y and AC_SB, Y becomes 0x5A.

			if (info[30].SB_Y != 1 || info[30].AC_SB != 1 || info[30].Y != 0x5A)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TAY cycle 30: SB_Y=%d AC_SB=%d Y=0x%02X\n",
					info[30].SB_Y, info[30].AC_SB, info[30].Y);
				Logger::WriteMessage(msg);
				return false;
			}

			// TYA execute cycle: Y_SB and SB_AC.

			if (info[34].Y_SB != 1 || info[34].SB_AC != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TYA cycle 34: Y_SB=%d SB_AC=%d\n", info[34].Y_SB, info[34].SB_AC);
				Logger::WriteMessage(msg);
				return false;
			}

			// TXS execute cycle: X_SB and SB_S are active, and S becomes 0x5A
			// (the register update is visible at the next cycle).

			if (info[38].X_SB != 1 || info[38].SB_S != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TXS cycle 38: X_SB=%d SB_S=%d\n", info[38].X_SB, info[38].SB_S);
				Logger::WriteMessage(msg);
				return false;
			}
			if (info[39].S != 0x5A)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TXS S=0x%02X at cycle 39, expected 0x5A\n", info[39].S);
				Logger::WriteMessage(msg);
				return false;
			}

			// TSX execute cycle: SB_X and S_SB.

			if (info[42].SB_X != 1 || info[42].S_SB != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! TSX cycle 42: SB_X=%d S_SB=%d\n", info[42].SB_X, info[42].S_SB);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// --- Program 2: LDA #0x11, SEC, ADC #0x22, PHP, PLP, PHA, PLA ---

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0xA9, 0x11, 0x38, 0x69, 0x22, 0x08, 0x28, 0x48, 0x68, 0xEA, 0xEA };
			DebugInfo info[130]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// LDA #0x11.

			if (info[22].SB_AC != 1 || info[22].AC != 0x11 || info[22].DBZ_Z != 1 || info[22].DB_N != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! LDA#11 cycle 22: SB_AC=%d AC=0x%02X DBZ_Z=%d DB_N=%d\n",
					info[22].SB_AC, info[22].AC, info[22].DBZ_Z, info[22].DB_N);
				Logger::WriteMessage(msg);
				return false;
			}

			// Opcode fetch cycle: the bus control drives the external address bus
			// (ADL_ABL, ADH_ABH) and connects the internal buses (SB_DB).

			if (info[20].ADL_ABL != 1 || info[20].ADH_ABH != 1 || info[20].SB_DB != 1 ||
				info[20].SB_ADH != 0 || info[20].DL_ADL != 0 || info[20].DL_ADH != 0 || info[20].DL_DB != 0 ||
				info[20].Z_ADL0 != 0 || info[20].Z_ADL1 != 0 || info[20].Z_ADL2 != 0 ||
				info[20].Z_ADH0 != 0 || info[20].Z_ADH17 != 0)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! bus control cycle 20: ADL_ABL=%d ADH_ABH=%d SB_DB=%d SB_ADH=%d DL_ADL=%d DL_ADH=%d DL_DB=%d\n",
					info[20].ADL_ABL, info[20].ADH_ABH, info[20].SB_DB, info[20].SB_ADH, info[20].DL_ADL, info[20].DL_ADH, info[20].DL_DB);
				Logger::WriteMessage(msg);
				return false;
			}

			// PC control at the fetch cycle: the PC is output to the address bus
			// (PCL_ADL, PCH_ADH) and is refreshed from ADL/ADH (ADL_PCL, ADH_PCH).

			if (info[20].PCL_ADL != 1 || info[20].PCH_ADH != 1 || info[20].ADL_PCL != 1 || info[20].ADH_PCH != 1 ||
				info[20].PCL_DB != 0 || info[20].PCH_DB != 0 || info[20].PCL_PCL != 0 || info[20].PCH_PCH != 0)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! PC control cycle 20: PCL_ADL=%d PCH_ADH=%d ADL_PCL=%d ADH_PCH=%d PCL_DB=%d PCH_DB=%d PCL_PCL=%d PCH_PCH=%d\n",
					info[20].PCL_ADL, info[20].PCH_ADH, info[20].ADL_PCL, info[20].ADH_PCH, info[20].PCL_DB, info[20].PCH_DB, info[20].PCL_PCL, info[20].PCH_PCH);
				Logger::WriteMessage(msg);
				return false;
			}

			// SEC: IR5_C is active, the C flag becomes 1.

			if (info[25].IR5_C != 1 || info[26].IR5_C != 1 || info[26].C_OUT != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! SEC: IR5_C(25)=%d IR5_C(26)=%d C(26)=%d\n", info[25].IR5_C, info[26].IR5_C, info[26].C_OUT);
				Logger::WriteMessage(msg);
				return false;
			}

			// ADC #0x22: the ALU adds with the carry in (n_ACIN=0), SUMS active.

			if (info[30].n_ACIN != 0 || info[30].SUMS != 1 || info[30].AC_SB != 1 ||
				info[30].ANDS != 0 || info[30].EORS != 0 || info[30].ORS != 0 || info[30].SRS != 0 ||
				info[30].n_DAA != 1 || info[30].n_DSA != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! ADC cycle 30: n_ACIN=%d SUMS=%d AC_SB=%d ANDS=%d EORS=%d ORS=%d SRS=%d n_DAA=%d n_DSA=%d\n",
					info[30].n_ACIN, info[30].SUMS, info[30].AC_SB, info[30].ANDS, info[30].EORS, info[30].ORS, info[30].SRS, info[30].n_DAA, info[30].n_DSA);
				Logger::WriteMessage(msg);
				return false;
			}

			// 0x11 + 0x22 + 1 = 0x34.

			if (info[32].AC != 0x34)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! ADC result AC=0x%02X, expected 0x34\n", info[32].AC);
				Logger::WriteMessage(msg);
				return false;
			}

			// PHP: P_DB, S_ADL and WR active (the status register is pushed).

			if (info[33].P_DB != 1 || info[33].S_ADL != 1 || info[33].WR != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! PHP cycle 33: P_DB=%d S_ADL=%d WR=%d\n", info[33].P_DB, info[33].S_ADL, info[33].WR);
				Logger::WriteMessage(msg);
				return false;
			}

			// PLP: the status register is loaded from the DB bus (DB_P, DB_N, DB_C, DB_V).

			if (info[105].DB_P != 1 || info[105].DB_N != 1 || info[105].DB_C != 1 || info[105].DB_V != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! PLP cycle 105: DB_P=%d DB_N=%d DB_C=%d DB_V=%d\n",
					info[105].DB_P, info[105].DB_N, info[105].DB_C, info[105].DB_V);
				Logger::WriteMessage(msg);
				return false;
			}

			// PHA: the accumulator goes to the DB bus (AC_DB), WR active.

			if (info[110].S_ADL != 1 || info[110].WR != 1 || info[110].AC_DB != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! PHA cycle 110: S_ADL=%d WR=%d AC_DB=%d\n", info[110].S_ADL, info[110].WR, info[110].AC_DB);
				Logger::WriteMessage(msg);
				return false;
			}

			// PLA: the accumulator loads from the SB bus (the stack holds 0 in this harness).

			if (info[120].SB_AC != 1 || info[120].AC != 0x00)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! PLA cycle 120: SB_AC=%d AC=0x%02X\n", info[120].SB_AC, info[120].AC);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// --- Program 3: BRK loop ---

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0x00 };
			DebugInfo info[46]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// BRK pushes PCH, PCL and P (P_DB=1) to the stack.

			if (info[22].WR != 1 || info[26].P_DB != 1 || info[26].WR != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! BRK push: WR(22)=%d P_DB(26)=%d WR(26)=%d\n", info[22].WR, info[26].P_DB, info[26].WR);
				Logger::WriteMessage(msg);
				return false;
			}

			// The interrupt vector address is forced to FFFE (Z_ADL0=1).

			if (info[27].Z_ADL0 != 1)
			{
				Logger::WriteMessage("Integration failed! BRK vector: Z_ADL0 not active at cycle 27\n");
				return false;
			}

			// BRK6E becomes active during the vector fetch.

			if (info[29].BRK6E != 1)
			{
				Logger::WriteMessage("Integration failed! BRK6E not active at cycle 29\n");
				return false;
			}

			// The stack pointer goes down.

			if (info[29].S != 0xFA)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! BRK S=0x%02X at cycle 29, expected 0xFA\n", info[29].S);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// --- Program 4: branch taken (BNE +2) ---

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0xA9, 0x01, 0xD0, 0x02, 0xEA, 0xEA, 0xEA, 0xEA };
			DebugInfo info[40]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// The BNE is fetched.

			if (info[24].IR != 0xD0)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! BNE IR=0x%02X at cycle 24, expected 0xD0\n", info[24].IR);
				Logger::WriteMessage(msg);
				return false;
			}

			// The branch is taken (BRFW=0, nBRTAKEN=0), the PC increment is disabled,
			// and the offset is added via the ALU (ADL_ADD, SB_ADH).

			if (info[26].BRFW != 0 || info[26].n_BRTAKEN != 0 || info[26].n_1PC != 1 || info[26].ADL_ADD != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! BNE cycle 26: BRFW=%d nBRTAKEN=%d n_1PC=%d ADL_ADD=%d\n",
					info[26].BRFW, info[26].n_BRTAKEN, info[26].n_1PC, info[26].ADL_ADD);
				Logger::WriteMessage(msg);
				return false;
			}
			if (info[27].SB_ADH != 1)
			{
				Logger::WriteMessage("Integration failed! BNE cycle 27: SB_ADH not active\n");
				return false;
			}

			// The PC reaches the branch target.

			if (info[29].PCL != 0x07)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! BNE PCL=0x%02X at cycle 29, expected 0x07\n", info[29].PCL);
				Logger::WriteMessage(msg);
				return false;
			}
		}

		// --- Program 5: INC $10 (RMW) ---

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0xE6, 0x10, 0xEA, 0xEA };
			DebugInfo info[30]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// RMW_T6 cycle.

			if (info[22].RMW_T6 != 1 || info[23].RMW_T6 != 1 || info[23].WR != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! INC T6: RMW_T6(22)=%d RMW_T6(23)=%d WR(23)=%d\n", info[22].RMW_T6, info[23].RMW_T6, info[23].WR);
				Logger::WriteMessage(msg);
				return false;
			}

			// RMW_T7 cycle: the increment is performed (n_ACIN=0) and the result is written back.

			if (info[24].RMW_T7 != 1 || info[24].n_ACIN != 0 || info[24].WR != 1)
			{
				char msg[0x200];
				sprintf_s(msg, sizeof(msg), "Integration failed! INC T7: RMW_T7=%d n_ACIN=%d WR=%d\n", info[24].RMW_T7, info[24].n_ACIN, info[24].WR);
				Logger::WriteMessage(msg);
				return false;
			}
			if (info[25].RMW_T7 != 1 || info[25].ENDX != 0)
			{
				Logger::WriteMessage("Integration failed! INC T7 cycle 25: RMW_T7/ENDX mismatch\n");
				return false;
			}
		}

		Logger::WriteMessage("CoreInternals_Integration_Test OK!\n");
		return true;
	}

	bool UnitTest::M6502API_Test()
	{
		// Run a few cycles first so the core is alive.

		{
			M6502 core_local(true, false);
			uint8_t prog[] = { 0xA9, 0x42, 0xEA, 0xEA };
			DebugInfo info[24]{};
			RunProgramCapture(&core_local, prog, sizeof(prog), _countof(info), info);

			// getDebug fills the structure without crashing and the values are readable.

			DebugInfo di{};
			core_local.getDebug(&di);
			if (di.IR > 0xFF || di.PD > 0xFF || di.AC > 0xFF)
			{
				Logger::WriteMessage("M6502API_Test failed! getDebug produced invalid values\n");
				return false;
			}

			// getDebugSingle / setDebugSingle round trip.

			core_local.setDebugSingle(offsetof(DebugInfo, AC), 0x77);
			if (core_local.getDebugSingle(offsetof(DebugInfo, AC)) != 0x77)
			{
				Logger::WriteMessage("M6502API_Test failed! setDebugSingle(AC)/getDebugSingle round trip broken\n");
				return false;
			}

			core_local.setDebugSingle(offsetof(DebugInfo, Y), 0x12);
			if (core_local.getDebugSingle(offsetof(DebugInfo, Y)) != 0x12)
			{
				Logger::WriteMessage("M6502API_Test failed! setDebugSingle(Y) round trip broken\n");
				return false;
			}

			core_local.setDebugSingle(offsetof(DebugInfo, PCL), 0x34);
			core_local.setDebugSingle(offsetof(DebugInfo, PCH), 0x12);
			if (core_local.getDebugSingle(offsetof(DebugInfo, PCL)) != 0x34 || core_local.getDebugSingle(offsetof(DebugInfo, PCH)) != 0x12)
			{
				Logger::WriteMessage("M6502API_Test failed! setDebugSingle(PCL/PCH) round trip broken\n");
				return false;
			}

			core_local.setDebugSingle(offsetof(DebugInfo, C_OUT), 1);
			if (core_local.getDebugSingle(offsetof(DebugInfo, C_OUT)) != 1)
			{
				Logger::WriteMessage("M6502API_Test failed! setDebugSingle(C_OUT) round trip broken\n");
				return false;
			}

			// Invalid offsets are ignored / return 0.

			core_local.setDebugSingle(0x7FFF, 5);
			if (core_local.getDebugSingle(0x7FFF) != 0)
			{
				Logger::WriteMessage("M6502API_Test failed! getDebugSingle(invalid) != 0\n");
				return false;
			}

			// getUserRegSingle / setUserRegSingle round trip.

			core_local.setUserRegSingle(offsetof(UserRegs, A), 0x42);
			if (core_local.getUserRegSingle(offsetof(UserRegs, A)) != 0x42)
			{
				Logger::WriteMessage("M6502API_Test failed! setUserRegSingle(A) round trip broken\n");
				return false;
			}
			if (core_local.getDebugSingle(offsetof(DebugInfo, AC)) != 0x42)
			{
				Logger::WriteMessage("M6502API_Test failed! A (set via UserRegs) not visible in getDebug\n");
				return false;
			}

			core_local.setUserRegSingle(offsetof(UserRegs, X), 0x21);
			core_local.setUserRegSingle(offsetof(UserRegs, Y), 0x43);
			core_local.setUserRegSingle(offsetof(UserRegs, S), 0x80);
			if (core_local.getUserRegSingle(offsetof(UserRegs, X)) != 0x21 ||
				core_local.getUserRegSingle(offsetof(UserRegs, Y)) != 0x43 ||
				core_local.getUserRegSingle(offsetof(UserRegs, S)) != 0x80)
			{
				Logger::WriteMessage("M6502API_Test failed! setUserRegSingle(X/Y/S) round trip broken\n");
				return false;
			}

			core_local.setUserRegSingle(offsetof(UserRegs, N_OUT), 1);
			if (core_local.getUserRegSingle(offsetof(UserRegs, N_OUT)) != 1)
			{
				Logger::WriteMessage("M6502API_Test failed! setUserRegSingle(N_OUT) round trip broken\n");
				return false;
			}

			// getUserRegs returns consistent values.

			UserRegs ur{};
			core_local.getUserRegs(&ur);
			if (ur.A != 0x42 || ur.X != 0x21 || ur.Y != 0x43 || ur.S != 0x80)
			{
				Logger::WriteMessage("M6502API_Test failed! getUserRegs inconsistent\n");
				return false;
			}
			if (ur.PCL != 0x34 || ur.PCH != 0x12)
			{
				Logger::WriteMessage("M6502API_Test failed! getUserRegs PCL/PCH inconsistent\n");
				return false;
			}
			if (ur.C_OUT != 1 || ur.N_OUT != 1)
			{
				Logger::WriteMessage("M6502API_Test failed! getUserRegs flags inconsistent\n");
				return false;
			}

			// getDebug agrees with getUserRegs.

			core_local.getDebug(&di);
			if (di.AC != 0x42 || di.X != 0x21 || di.Y != 0x43 || di.S != 0x80 ||
				di.PCL != 0x34 || di.PCH != 0x12)
			{
				Logger::WriteMessage("M6502API_Test failed! getDebug inconsistent with UserRegs\n");
				return false;
			}
		}

		Logger::WriteMessage("M6502API_Test OK!\n");
		return true;
	}
}

namespace UnitTest
{
	TEST_CLASS(CoreInternalsUnitTest)
	{
	public:

		TEST_METHOD(TestDecoder)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.Decoder_Test());
		}

		TEST_METHOD(TestIR)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.IR_Test());
		}

		TEST_METHOD(TestPreDecode)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.PreDecode_Test());
		}

		TEST_METHOD(TestFlags)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.Flags_Test());
		}

		TEST_METHOD(TestRegs)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.Regs_Test());
		}

		TEST_METHOD(TestALU_Extras)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.ALU_Extras_Test());
		}

		TEST_METHOD(TestPC_HLE)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.PC_HLE_Test());
		}

		TEST_METHOD(TestDataBus)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.DataBus_Test());
		}

		TEST_METHOD(TestAddressBus)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.AddressBus_Test());
		}

		TEST_METHOD(TestBRK)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.BRK_Test());
		}

		TEST_METHOD(TestRegsControl)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.RegsControl_Test());
		}

		TEST_METHOD(TestBusControl)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.BusControl_Test());
		}

		TEST_METHOD(TestPCControl)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.PCControl_Test());
		}

		TEST_METHOD(TestFlagsControl)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.FlagsControl_Test());
		}

		TEST_METHOD(TestALUControl)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.ALUControl_Test());
		}

		TEST_METHOD(TestBranchLogic)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.BranchLogic_Test());
		}

		TEST_METHOD(TestDispatcher)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.Dispatcher_Test());
		}

		TEST_METHOD(TestRandomLogic)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.RandomLogic_Test());
		}

		TEST_METHOD(TestExtraCounter)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.ExtraCounter_Test());
		}

		TEST_METHOD(TestCoreInternals_Integration)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.CoreInternals_Integration_Test());
		}

		TEST_METHOD(TestM6502API)
		{
			M6502CoreUnitTest::UnitTest ut;
			Assert::IsTrue(ut.M6502API_Test());
		}
	};
}
