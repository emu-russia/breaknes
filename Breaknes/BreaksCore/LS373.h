// LS373
// Octal D-type transparent latch; 3-state

#pragma once

namespace LS
{
	class LS373
	{
		uint8_t val = 0;

	public:

		/// <summary>
		/// Latch simulation.
		/// </summary>
		/// <param name="LE">1: Enable input</param>
		/// <param name="n_OE">0: Enable output</param>
		/// <param name="d">Input value for storage on the latch</param>
		/// <param name="q">Output value. Set only if n_OE = `0`.</param>
		/// <param name="qz">true: Output valid; false; Output has value `z` (disconnected)</param>
		void sim(BaseLogic::TriState LE, BaseLogic::TriState n_OE, uint8_t d, uint8_t *q, bool& qz);
	};
}
