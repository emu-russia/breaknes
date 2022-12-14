// Triangle Channel

#pragma once

namespace APUSim
{
	class TriangleChan
	{
		APU* apu = nullptr;

	public:
		TriangleChan(APU* parent);
		~TriangleChan();

		void sim();
	};
}
