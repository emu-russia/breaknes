#pragma once

namespace UnitTest
{
	class LS139_Test
	{
	public:
		LS139_Test();
		~LS139_Test();

		// Both halves decode correctly (active-low 1-of-4) while enabled.
		bool TestHalfTruthTable();

		// All outputs are '1' when a half is disabled.
		bool TestAllDisabled();

		// Floating (Z) inputs are resolved to '1' by the internal pull-ups of the chip
		// and the actual reference is modified on the net.
		bool TestInternalPullUp();

		// NES "north bridge" select mapping (nROMSEL / WRAM_nCE / PPU_nCE).
		bool TestNesDecode();

		// Famicom "north bridge" select mapping (stages are mixed up, same map).
		bool TestFamicomDecode();
	};
}
