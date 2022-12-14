// 6502 Core Binding

#pragma once

namespace APUSim
{
	class CoreBinding
	{
		APU* apu = nullptr;

	public:
		CoreBinding(APU* parent);
		~CoreBinding();

		void sim();
	};
}
