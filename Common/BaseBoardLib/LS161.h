// 74LS161: presettable synchronous 4-bit binary counter with asynchronous reset.
//
// Pins (standard 74LS161 pinout, as wired on NES cartridge PCBs):
//   CLK   - clock (loads / counts on the rising edge)
//   nRST  - asynchronous reset (active low)
//   nLD   - parallel load enable (active low)
//   EN_T, EN_P - count enable (both high to count)
//   P[0..3] - parallel data inputs (P0 = LSB)
//   RCO   - ripple carry out (EN_T high and count == 15)
//   Q[0..3] - counter outputs (Q0 = LSB)
//
// The state advances on the *rising* edge of CLK: with nLD low the parallel
// data is loaded, otherwise (EN_T & EN_P high) the counter increments. This is
// how the chip is used on UxROM/AxROM boards: CLK = /ROMSEL, nLD = R/W, so a
// write to $8000-$FFFF loads the data bus into the bank register on the
// /ROMSEL rising edge at the end of the access.

#pragma once

#include <cstdint>

#include "../BaseLogicLib/BaseLogic.h"

namespace BaseBoard
{
	class LS161
	{
		uint8_t val = 0;
		BaseLogic::TriState prev_CLK = BaseLogic::TriState::One;	// power-up: clock idle (high)

	public:
		void sim(
			BaseLogic::TriState CLK,
			BaseLogic::TriState nRST,
			BaseLogic::TriState nLD,
			BaseLogic::TriState EN_T,
			BaseLogic::TriState EN_P,
			BaseLogic::TriState P[4],
			BaseLogic::TriState& RCO,
			BaseLogic::TriState Q[4] );

		/// <summary>
		/// Get the current value of the counter without simulating it.
		/// </summary>
		uint8_t getVal() { return val; }
	};
}
