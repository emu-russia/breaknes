#pragma once

namespace M6502CoreUnitTest
{
	class UnitTest
	{
		enum class ALU_Operation
		{
			None = 0,
			ANDS,
			EORS,
			ORS,
			SRS,
			SUMS,
		};

		void ResetPcInputs();
		bool PC_Test(uint16_t initial_pc, uint16_t expected_pc, bool inc, const char* test_name);

		void ResetALUInputs(ALU_Operation op);
		int TestCompute(uint8_t a, uint8_t b, uint8_t expected, ALU_Operation op, bool bcd, bool carry);
		size_t BCD_Add(size_t a, size_t b, bool carry_in);

		M6502Core::M6502 *core;

	public:
		UnitTest();
		~UnitTest();

		/// <summary>
		/// Checking Program Counter simulator functionality.
		/// </summary>
		bool PC_UnitTest();

		/// <summary>
		/// Checking the operability of the ALU 6502 in different modes.
		/// </summary>
		/// <returns></returns>
		bool ALU_UnitTest();

		/// <summary>
		/// Go through all 6502 decoder values and output them to CSV.
		/// </summary>
		/// <returns></returns>
		bool DumpDecoder();

		/// <summary>
		/// Execute some million cycles and check that their execution time is faster or equal to the real chip.
		/// The chip in this test is in "pumpkin" mode: it lives, but it does nothing useful.
		/// </summary>
		/// <param name="desired_clk">Desired cycle rate per second (Hz)</param>
		/// <returns></returns>
		bool MegaCyclesTest(size_t desired_clk);

		/// <summary>
		/// Checking the decoder (PLA) simulator.
		/// </summary>
		bool Decoder_Test();

		/// <summary>
		/// Checking the instruction register (IR) simulator.
		/// </summary>
		bool IR_Test();

		/// <summary>
		/// Checking the predecoder simulator.
		/// </summary>
		bool PreDecode_Test();

		/// <summary>
		/// Checking the flags register (P) simulator: loading, storing, getters and setters.
		/// </summary>
		bool Flags_Test();

		/// <summary>
		/// Checking the Y/X/S registers simulator.
		/// </summary>
		bool Regs_Test();

		/// <summary>
		/// Checking the ALU extras: sim_HLE, sim_BusMux, getters/setters, BCD hack.
		/// </summary>
		bool ALU_Extras_Test();

		/// <summary>
		/// Checking the HLE program counter simulator (load/store/increment).
		/// </summary>
		bool PC_HLE_Test();

		/// <summary>
		/// Checking the data bus simulator (external bus exchange).
		/// </summary>
		bool DataBus_Test();

		/// <summary>
		/// Checking the address bus simulator (constant generator and output).
		/// </summary>
		bool AddressBus_Test();

		/// <summary>
		/// Checking the interrupt (BRK) processing logic: DORES, B_OUT, BRK6E latches.
		/// </summary>
		bool BRK_Test();

		/// <summary>
		/// Checking the register control logic (Y_SB, SB_Y, X_SB, SB_X, S_ADL, S_SB, SB_S, S_S).
		/// </summary>
		bool RegsControl_Test();

		/// <summary>
		/// Checking the bus control logic (ADL_ABL, ADH_ABH, SB_DB, SB_ADH, DL_ADL, DL_ADH, DL_DB, Z_*).
		/// </summary>
		bool BusControl_Test();

		/// <summary>
		/// Checking the program counter control logic (ADL_PCL, PCL_PCL, PCL_ADL, PCL_DB, ADH_PCH, PCH_PCH, PCH_ADH, PCH_DB).
		/// </summary>
		bool PCControl_Test();

		/// <summary>
		/// Checking the flags control logic (P_DB, DB_P, DBZ_Z, DB_N, IR5_C, DB_C, ACR_C, IR5_D, IR5_I, DB_V, AVR_V, Z_V).
		/// </summary>
		bool FlagsControl_Test();

		/// <summary>
		/// Checking the ALU control logic (n_ACIN, n_DAA, n_DSA, ANDS, EORS, ORS, SRS, SUMS, ADD_SB7, ADD_SB06, ADD_ADL).
		/// </summary>
		bool ALUControl_Test();

		/// <summary>
		/// Checking the branch logic (BRFW, n_BRTAKEN).
		/// </summary>
		bool BranchLogic_Test();

		/// <summary>
		/// Checking the dispatcher (T0, n_T0, n_T1X, FETCH, n_ready, ACRL1, ACRL2, TRES1, n_TRESX, ENDS, ENDX, WR, RMW_T6, RMW_T7, n_1PC).
		/// </summary>
		bool Dispatcher_Test();

		/// <summary>
		/// Checking the random logic as a whole: run a known program and verify the control commands.
		/// </summary>
		bool RandomLogic_Test();

		/// <summary>
		/// Checking the extra cycle counter (T2..T5), both HLE and non-HLE modes.
		/// </summary>
		bool ExtraCounter_Test();

		/// <summary>
		/// Running known programs and checking the internal signals (commands and wires) at specific cycles.
		/// </summary>
		bool CoreInternals_Integration_Test();

		/// <summary>
		/// Checking the public debugging API (getDebug, getUserRegs, get/setDebugSingle, get/setUserRegSingle).
		/// </summary>
		bool M6502API_Test();
	};
}
