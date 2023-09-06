// 6502 Core Binding

#pragma once

namespace FastAPU
{
	class CoreBinding
	{
		FastAPU* apu = nullptr;

		int CLK_FF{};
		uint8_t div{};

		void sim_DividerBeforeCore();
		void sim_DividerAfterCore();

	public:
		CoreBinding(FastAPU* parent);
		~CoreBinding();

		void sim();
	};
}
