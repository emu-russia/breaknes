#pragma once

namespace M6502Core
{
	class BranchLogic
	{
		friend M6502;
		friend RandomLogic;
		BaseLogic::DLatch br2_latch{};
		BaseLogic::DLatch brfw_latch1{};
		BaseLogic::DLatch brfw_latch2{};

		M6502* core = nullptr;

		template<bool Packed, int Phase = 0> void simImpl();

	public:

		BranchLogic(M6502* parent) { core = parent; }

		void sim();

		BaseLogic::TriState getBRFW();
	};
}
