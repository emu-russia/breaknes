// Quad 2-Input OR Gate

#pragma once

namespace BaseBoard
{
	class LS32
	{
	public:
		static void sim(
			BaseLogic::TriState A[4],
			BaseLogic::TriState B[4],
			BaseLogic::TriState Y[4] );
	};
}
