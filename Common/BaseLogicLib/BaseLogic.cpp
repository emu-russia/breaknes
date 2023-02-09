#include "pch.h"

namespace BaseLogic
{

	TriState NOT (TriState a)
	{
#if _DEBUG
		if (!(a == TriState::Zero || a == TriState::One))
		{
			throw "The third state is not supported for this operation under normal conditions.";
		}
#endif

		return a == TriState::Zero ? TriState::One : TriState::Zero;
	}

	TriState NOR(TriState a, TriState b)
	{
		return (TriState)((~(a | b)) & 1);
	}

	TriState NOR3(TriState a, TriState b, TriState c)
	{
		return (TriState)((~(a | b | c)) & 1);
	}

	TriState NOR4(TriState in[4])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3])) & 1);
	}

	TriState NOR4(TriState in0, TriState in1, TriState in2, TriState in3)
	{
		return (TriState)((~(in0 | in1 | in2 | in3)) & 1);
	}

	TriState NOR5(TriState in[5])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4])) & 1);
	}

	TriState NOR5(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4)) & 1);
	}

	TriState NOR6(TriState in[6])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5])) & 1);
	}

	TriState NOR6(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5)) & 1);
	}

	TriState NOR7(TriState in[7])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6])) & 1);
	}

	TriState NOR7(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6)) & 1);
	}

	TriState NOR8(TriState in[8])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7])) & 1);
	}

	TriState NOR8(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7)) & 1);
	}

	TriState NOR9(TriState in[9])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8])) & 1);
	}

	TriState NOR9(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8)) & 1);
	}

	TriState NOR10(TriState in[10])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9])) & 1);
	}

	TriState NOR10(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9)) & 1);
	}

	TriState NOR11(TriState in[11])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10])) & 1);
	}

	TriState NOR11(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10)) & 1);
	}

	TriState NOR12(TriState in[12])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10] | in[11])) & 1);
	}

	TriState NOR12(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11)) & 1);
	}

	TriState NOR13(TriState in[13])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10] | in[11] | in[12])) & 1);
	}

	TriState NOR13(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12)) & 1);
	}

	TriState NOR15(TriState in[15])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10] | in[11] | in[12] | in[13] | in[14])) & 1);
	}

	TriState NOR15(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12 | in13 | in14)) & 1);
	}

	TriState NOR16(TriState in[16])
	{
		return (TriState)((~(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10] | in[11] | in[12] | in[13] | in[14] | in[15])) & 1);
	}

	TriState NOR16(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12 | in13 | in14 | in15)) & 1);
	}

	TriState NOR25(TriState in[25])
	{
		return (TriState)((~(
			in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7] | in[8] | in[9] | in[10] | in[11] |
			in[12] | in[13] | in[14] | in[15] | in[16] | in[17] | in[18] | in[19] | in[20] | in[21] | in[22] | in[23] |
			in[24] )) & 1);
	}

	TriState NOR27(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12 | in13 | in14 | in15 | in16 | in17 | in18 | in19 | in20 | in21 | in22 | in23 | in24 | in25 | in26)) & 1);
	}

	TriState NOR28(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26, TriState in27)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12 | in13 | in14 | in15 | in16 | in17 | in18 | in19 | in20 | in21 | in22 | in23 | in24 | in25 | in26 | in27)) & 1);
	}

	TriState NOR29(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26, TriState in27, TriState in28)
	{
		return (TriState)((~(in0 | in1 | in2 | in3 | in4 | in5 | in6 | in7 | in8 | in9 | in10 | in11 | in12 | in13 | in14 | in15 | in16 | in17 | in18 | in19 | in20 | in21 | in22 | in23 | in24 | in25 | in26 | in27 | in28)) & 1);
	}

	TriState NAND(TriState a, TriState b)
	{
		return (TriState)((~(a & b)) & 1);
	}

	TriState NAND3(TriState a, TriState b, TriState c)
	{
		return (TriState)((~((a & b) & c)) & 1);
	}

	TriState AND(TriState a, TriState b)
	{
		return (TriState)(a & b);
	}

	TriState AND3(TriState a, TriState b, TriState c)
	{
		return (TriState)(((a & b) & c) & 1);
	}

	TriState AND4(TriState in[4])
	{
		return (TriState)(((in[0] & in[1] & in[2] & in[3])) & 1);
	}

	TriState OR(TriState a, TriState b)
	{
		return (TriState)((a | b) & 1);
	}

	TriState OR3(TriState a, TriState b, TriState c)
	{
		return (TriState)((a | b | c) & 1);
	}

	TriState XOR(TriState a, TriState b)
	{
		return (TriState)((a ^ b) & 1);
	}

	void DLatch::set(TriState val, TriState en)
	{
		if (en == TriState::One)
		{
			if (val == TriState::Z)
			{
				// The floating input does not change the state of the latch.
				return;
			}

			g = val;
		}
	}

	TriState DLatch::get()
	{
		return g;
	}

	TriState DLatch::nget()
	{
		return NOT(g);
	}

	void FF::set(TriState val)
	{
		if (val == TriState::Z)
		{
			// The floating input does not change the state of the FF.
			return;
		}

		g = val;
	}

	TriState FF::get()
	{
		return g;
	}

	TriState FF::nget()
	{
		return NOT(g);
	}

	TriState MUX(TriState sel, TriState in0, TriState in1)
	{
		return ((sel & 1) == 0) ? in0 : in1;
	}

	TriState MUX2(TriState sel[2], TriState in[4])
	{
		size_t numOut = 0;

		for (size_t n = 0; n < 2; n++)
		{
			numOut |= (sel[n] == TriState::One ? 1ULL : 0) << n;
		}

		return in[numOut];
	}

	TriState MUX3(TriState sel[3], TriState in[8])
	{
		size_t numOut = 0;

		for (size_t n = 0; n < 3; n++)
		{
			numOut |= (sel[n] == TriState::One ? 1ULL : 0) << n;
		}

		return in[numOut];
	}

	void DMX2(TriState in[2], TriState out[4])
	{
		TriState nibble[4];
		
		nibble[0] = in[0];
		nibble[1] = in[1];
		nibble[2] = TriState::Zero;
		nibble[3] = TriState::Zero;

		size_t fireInput = PackNibble(nibble);

		for (size_t n = 0; n < 4; n++)
		{
			out[n] = n == fireInput ? TriState::One : TriState::Zero;
		}
	}

	void DMX3(TriState in[3], TriState out[8])
	{
		TriState nibble[4];

		nibble[0] = in[0];
		nibble[1] = in[1];
		nibble[2] = in[2];
		nibble[3] = TriState::Zero;

		size_t fireInput = PackNibble(nibble);

		for (size_t n = 0; n < 8; n++)
		{
			out[n] = n == fireInput ? TriState::One : TriState::Zero;
		}
	}

	void DMX4(TriState in[4], TriState out[16])
	{
		size_t fireInput = PackNibble(in);

		for (size_t n = 0; n < 16; n++)
		{
			out[n] = n == fireInput ? TriState::One : TriState::Zero;
		}
	}

	size_t Decoder2(TriState in[2])
	{
		// Pack2
		size_t n = ((in[1] == TriState::One ? 1ULL : 0) << 1) |
			(in[0] == TriState::One ? 1ULL : 0);
		return n;
	}

	size_t Decoder3(TriState in[3])
	{
		// Pack3
		size_t n = ((in[2] == TriState::One ? 1ULL : 0) << 2) |
			((in[1] == TriState::One ? 1ULL : 0) << 1) |
			(in[0] == TriState::One ? 1ULL : 0);
		return n;
	}

	PLA::PLA(size_t inputs, size_t outputs, char* filename)
	{
		romInputs = inputs;
		romOutputs = outputs;
		romSize = inputs * outputs;
		rom = new uint8_t[romSize];
		memset(rom, 0, romSize);
		unomptimized_out = new TriState[romOutputs];
		strcpy_s(fname, sizeof(fname), filename);
	}

	PLA::~PLA()
	{
		if (rom)
			delete[] rom;

		if (outs)
			delete[] outs;

		if (unomptimized_out)
			delete[] unomptimized_out;
	}

	void PLA::SetMatrix(size_t bitmask[])
	{
		for (size_t out = 0; out < romOutputs; out++)
		{
			size_t val = bitmask[out];

			for (size_t bit = 0; bit < romInputs; bit++)
			{
				rom[out * romInputs + (romInputs - bit - 1)] = val & 1;
				val >>= 1;
			}
		}

		// Optimization for PLA

		if (Optimize)
		{
			if (outs)
			{
				delete[] outs;
				outs = nullptr;
			}

			size_t maxLane = (1ULL << romInputs);

			FILE* f;
			fopen_s(&f, fname, "rb");
			if (f)
			{
				outs = new TriState[maxLane * romOutputs];
				fread(outs, sizeof(TriState), maxLane * romOutputs, f);
				fclose(f);
				return;
			}

			outs = new TriState[maxLane * romOutputs];
			TriState* outputs;

			for (size_t n = 0; n < maxLane; n++)
			{
				sim_Unomptimized(n, &outputs);

				TriState* lane = &outs[n * romOutputs];
				memcpy(lane, outputs, romOutputs * sizeof(TriState));
			}

			fopen_s(&f, fname, "wb");
			if (f)
			{
				fwrite(outs, sizeof(TriState), maxLane * romOutputs, f);
				fclose(f);
			}
			else
			{
				throw "fopen_s failed!";
			}
		}
	}

	void PLA::sim(size_t input_bits, TriState** outputs)
	{
		if (!outs)
		{
			sim_Unomptimized(input_bits, outputs);
			return;
		}

		TriState* lane = &outs[input_bits * romOutputs];
		*outputs = lane;
	}

	void PLA::sim_Unomptimized(size_t input_bits, TriState** outputs)
	{
		for (size_t out = 0; out < romOutputs; out++)
		{
			// Since the decoder lines are multi-input NORs - the default output is `1`.
			unomptimized_out[out] = TriState::One;

			for (size_t bit = 0; bit < romInputs; bit++)
			{
				// If there is a transistor at the crossing point and the corresponding input is `1` - then ground the output and abort the term.
				if (rom[out * romInputs + bit] && (input_bits & (1ULL << bit)))
				{
					unomptimized_out[out] = TriState::Zero;
					break;
				}
			}
		}
		*outputs = unomptimized_out;
	}

	uint8_t Pack(TriState in[8])
	{
		uint8_t val = 0;
		for (size_t i = 0; i < 8; i++)
		{
			val |= (in[i] == TriState::One ? 1 : 0) << i;
		}
		return val;
	}

	uint8_t Pack3(TriState in[3])
	{
		uint8_t val = 0;
		for (size_t i = 0; i < 3; i++)
		{
			val |= (in[i] == TriState::One ? 1 : 0) << i;
		}
		return val;
	}

	uint8_t Pack5(TriState in[5])
	{
		uint8_t val = 0;
		for (size_t i = 0; i < 5; i++)
		{
			val |= (in[i] == TriState::One ? 1 : 0) << i;
		}
		return val;
	}

	uint8_t PackNibble(TriState in[4])
	{
		uint8_t val = 0;
		for (size_t i = 0; i < 4; i++)
		{
			val |= (in[i] == TriState::One ? 1 : 0) << i;
		}
		return val;
	}

	void Unpack(uint8_t val, TriState out[8])
	{
		for (size_t i = 0; i < 8; i++)
		{
			out[i] = ((val >> i) & 1) ? TriState::One : TriState::Zero;
		}
	}

	void UnpackNibble(uint8_t val, TriState out[4])
	{
		for (size_t i = 0; i < 4; i++)
		{
			out[i] = ((val >> i) & 1) ? TriState::One : TriState::Zero;
		}
	}

	void Dump(TriState in[8], const char* name)
	{
		printf("%s: ", name);
		for (size_t i = 0; i < 8; i++)
		{
			switch (in[7 - i])
			{
				case TriState::Zero:
					printf("0 ");
					break;
				case TriState::One:
					printf("1 ");
					break;
				case TriState::Z:
					printf("z ");
					break;
				case TriState::X:
					printf("x ");
					break;
			}
		}
		printf("\n");
	}

	void BusConnect(TriState& a, TriState& b)
	{
		if (a != b)
		{
			if (a == TriState::Zero)
			{
				b = TriState::Zero;
			}
			if (b == TriState::Zero)
			{
				a = TriState::Zero;
			}
		}
	}

	void BitRev(TriState in[8])
	{
		for (size_t n = 0; n < 4; n++)
		{
			auto old = in[n];
			in[n] = in[7 - n];
			in[7 - n] = old;
		}
	}

	uint8_t ToByte(TriState val)
	{
		return (uint8_t)val;
	}

	TriState FromByte(uint8_t val)
	{
		return (TriState)val;
	}

	bool IsPosedge(TriState prev_CLK, TriState CLK)
	{
		return (prev_CLK == TriState::Zero && CLK == TriState::One);
	}

	bool IsNegedge(TriState prev_CLK, TriState CLK)
	{
		return (prev_CLK == TriState::One && CLK == TriState::Zero);
	}
}
