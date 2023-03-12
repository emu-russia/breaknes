// Quad 2-Input OR Gate

#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	void LS32::sim(
		TriState A[4],
		TriState B[4],
		TriState Y[4])
	{
		for (size_t n = 0; n < 4; n++)
		{
			Y[n] = OR(A[n], B[n]);
		}
	}
}
