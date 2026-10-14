// Pixel Clock

#include "pch.h"
#include <array>

using namespace BaseLogic;

namespace PPUSimFast
{
	// The divider retains each transparent latch bank; this table does not
	// assume alternating clocks or an already-running ring.
	struct PixelClockStep { uint8_t state; TriState clock; };
	static constexpr PixelClockStep DividerStep(uint8_t state, bool high, bool reset, bool pal)
	{
		if (pal)
		{
			if (high) state=static_cast<uint8_t>((state & 0x2a) |
				(((state & 0x20)==0 && !reset) ? 1 : 0) |
				((state & 2)==0 ? 4 : 0) | ((state & 8)==0 ? 16 : 0));
			else state=static_cast<uint8_t>((state & 0x15) |
				((state & 1)==0 ? 2 : 0) | ((state & 4)==0 ? 8 : 0) |
				((state & 0x14)==0x14 ? 32 : 0));
			return {state,(state & 0x18)==0x10 ? One : Zero};
		}
		if (high) state=static_cast<uint8_t>((state & 5) | ((state & 1)==0 ? 2 : 0) | ((state & 4)==0 ? 8 : 0));
		else state=static_cast<uint8_t>((state & 10) | ((state & 8)!=0 && !reset ? 1 : 0) | ((state & 2)==0 ? 4 : 0));
		return {state,(state & 8)==0 ? One : Zero};
	}
	static const std::array<PixelClockStep,512>& DividerTable()
	{
		static constexpr auto table=[] {
			std::array<PixelClockStep,512> result{};
			for (unsigned key=0;key<512;key++) result[key]=DividerStep(key & 63,(key & 64)!=0,(key & 128)!=0,(key & 256)!=0);
			return result;
		}();
		return table;
	}
	void PPU::sim_PCLK()
	{
		const auto previous=wire.PCLK;
		const bool pal=rev==Revision::RP2C07_0 || rev==Revision::UMC_UA6538;
		if (wire.CLK!=One && wire.n_CLK!=One)
		{
			wire.PCLK=pal ? ((pclk_state & 0x18)==0x10 ? One : Zero) : ((pclk_state & 8)==0 ? One : Zero);
			wire.n_PCLK=NOT(wire.PCLK);
			if (IsNegedge(previous,wire.PCLK)) pclk_counter++;
			return;
		}
		const auto next=DividerTable()[pclk_state | (wire.CLK==One ? 64 : 0) | (wire.RES==One ? 128 : 0) | (pal ? 256 : 0)];
		pclk_state=next.state; wire.PCLK=next.clock; wire.n_PCLK=NOT(next.clock);
		if (IsNegedge(previous,next.clock)) pclk_counter++;
	}
}
