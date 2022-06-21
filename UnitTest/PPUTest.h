#pragma once

namespace PPUSimUnitTest
{
	/// <summary>
	/// Call one public test at a time, after creating the class instance (only this way will guarantee clean conditions between tests)
	/// </summary>
	class UnitTest
	{
		PPUSim::PPU* ppu = nullptr;

		void pclk();
		void n_pclk();
		void hv_count();

		PPUSim::PPU_Interconnects wires_open{};
		PPUSim::PPU_FSMStates fsm_open{};
		PPUSim::OAMEvalWires eval_open{};

		void LogWires(Debug::EventLog* log, PPUSim::PPU_Interconnects& wires, size_t ts);
		void LogFSMState(Debug::EventLog *log, PPUSim::PPU_FSMStates &fsm, size_t ts);
		void LogOAMEval(Debug::EventLog* log, PPUSim::OAMEvalWires& eval_wires, size_t ts);
		void CloseWires(Debug::EventLog* log, size_t ts);
		void CloseFSMState(Debug::EventLog* log, size_t ts);
		void CloseOAMEval(Debug::EventLog* log, size_t ts);
		void DumpPhaseShifter(PPUSim::VideoOut& vout);

	public:
		UnitTest(PPUSim::Revision rev);
		~UnitTest();

		bool HVCounterTestRES();
		bool HVCounterTest();
		bool RunSingleHalfCLK(BaseLogic::TriState CLK, BaseLogic::TriState n_RES);
		bool RunSinglePCLK();
		bool RunLines(size_t n);
		bool RunHalfCyclesWithChipScope(size_t half_cycles, const char *filename);
		uint8_t TestComparator(uint8_t a, uint8_t b);
		bool TestBGC_SR8();
		bool TestOAMCounter();
		bool TestFIFODownCounter(size_t v);
		bool TestFIFOPairedSR(uint8_t val);
		bool TestNtscPpuChromaDecoderOutputs();
		void DumpNtscPpuPhaseShifter(bool reset);
		void DumpNtscPpuColorSpace(bool emphasis);
		void Dump_2C04_0003_ColorSpace();
	};
}
