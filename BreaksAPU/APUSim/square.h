// Square Channels

#pragma once

namespace APUSim
{
	/// <summary>
	/// Input carry connection option for the adder
	/// </summary>
	enum SquareChanCarryIn
	{
		Unknown = 0,
		Vdd,
		Inc,
	};

	class SquareChan
	{
		APU* apu = nullptr;
		SquareChanCarryIn cin_type = SquareChanCarryIn::Unknown;

	public:
		SquareChan(APU* parent, SquareChanCarryIn carry_routing);
		~SquareChan();

		void sim();
	};
}
