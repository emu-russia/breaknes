#pragma once

namespace M6502Core
{
	class Regs
	{
		uint8_t Y = 0;
		uint8_t X = 0;
		uint8_t S_in = 0;
		uint8_t S_out = 0;

		M6502* core = nullptr;

	public:

		Regs(M6502* parent) { core = parent; }

		void sim_LoadSB();

		void sim_StoreSB();

		void sim_StoreOldS();

		uint8_t getY();
		uint8_t getX();
		uint8_t getS();

		void setY(uint8_t val);
		void setX(uint8_t val);
		void setS(uint8_t val);
	};
}
