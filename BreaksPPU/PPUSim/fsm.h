// H/V Control Logic

#pragma once

namespace PPUSim
{
	class FSM
	{
		PPU* ppu = nullptr;

	public:
		FSM(PPU *parent);
		~FSM();

		void sim();

		BaseLogic::TriState get_RESCL();
	};
}
