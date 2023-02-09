#pragma once

/// <summary>
/// Basic logic primitives used in N-MOS chips.
/// Combinational primitives are implemented using ordinary methods.
/// Sequential primitives are implemented using classes.
/// </summary>
namespace BaseLogic
{
	enum TriState : uint8_t
	{
		Zero = 0,
		One = 1,
		Z = (uint8_t)-1,
		X = (uint8_t)-2,
	};

	/// <summary>
	/// The simplest element, implemented with a single N-MOS FET.
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	TriState NOT(TriState a);

	/// <summary>
	/// 2-nor
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	TriState NOR(TriState a, TriState b);

	/// <summary>
	/// 3-nor
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="c"></param>
	/// <returns></returns>
	TriState NOR3(TriState a, TriState b, TriState c);

	/// <summary>
	/// 4-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR4(TriState in[4]);
	TriState NOR4(TriState in0, TriState in1, TriState in2, TriState in3);

	/// <summary>
	/// 5-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR5(TriState in[5]);
	TriState NOR5(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4);

	/// <summary>
	/// 6-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR6(TriState in[6]);
	TriState NOR6(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5);

	/// <summary>
	/// 7-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR7(TriState in[7]);
	TriState NOR7(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6);

	/// <summary>
	/// 8-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR8(TriState in[8]);
	TriState NOR8(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7);

	/// <summary>
	/// 9-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR9(TriState in[9]);
	TriState NOR9(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8);

	/// <summary>
	/// 10-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR10(TriState in[10]);
	TriState NOR10(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9);

	/// <summary>
	/// 11-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR11(TriState in[11]);
	TriState NOR11(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10);

	/// <summary>
	/// 12-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR12(TriState in[12]);
	TriState NOR12(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11);

	/// <summary>
	/// 13-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR13(TriState in[13]);
	TriState NOR13(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12);

	/// <summary>
	/// 15-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR15(TriState in[15]);
	TriState NOR15(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14);

	/// <summary>
	/// 16-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR16(TriState in[16]);
	TriState NOR16(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15);

	/// <summary>
	/// 25-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR25(TriState in[25]);

	/// <summary>
	/// 27-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR27(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26);

	/// <summary>
	/// 28-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR28(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26, TriState in27);

	/// <summary>
	/// 29-nor
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState NOR29(TriState in0, TriState in1, TriState in2, TriState in3, TriState in4, TriState in5, TriState in6, TriState in7, TriState in8, TriState in9, TriState in10, TriState in11, TriState in12, TriState in13, TriState in14, TriState in15, TriState in16, TriState in17, TriState in18, TriState in19, TriState in20, TriState in21, TriState in22, TriState in23, TriState in24, TriState in25, TriState in26, TriState in27, TriState in28);

	/// <summary>
	/// 2-nand
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	TriState NAND(TriState a, TriState b);

	/// <summary>
	/// 3-nand
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="c"></param>
	/// <returns></returns>
	TriState NAND3(TriState a, TriState b, TriState c);

	/// <summary>
	/// 2-and
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	TriState AND(TriState a, TriState b);

	/// <summary>
	/// 3-and
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="c"></param>
	/// <returns></returns>
	TriState AND3(TriState a, TriState b, TriState c);

	/// <summary>
	/// 4-and
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState AND4(TriState in[4]);

	/// <summary>
	/// 2-or
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	TriState OR(TriState a, TriState b);

	/// <summary>
	/// 3-or
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="c"></param>
	/// <returns></returns>
	TriState OR3(TriState a, TriState b, TriState c);

	/// <summary>
	/// 2-xor
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	TriState XOR(TriState a, TriState b);

	/// <summary>
	/// The real latch works as a pair of N-MOS transistors.
	/// The first transistor is the tri-state (`d`). It opens the input to the gate of the second transistor, where the value is stored.
	/// After closing the tri-state the value is stored as a `floating` value on the gate of the second transistor.
	/// The output from the second transistor is the DLatch output.
	/// Since the second transistor is essentially an inverter, the output will also be in inverse logic (`/out`)
	/// </summary>
	class DLatch
	{
		TriState g = TriState::Zero;

	public:

		void set(TriState val, TriState en);
		TriState get();
		TriState nget();
	};

	/// <summary>
	/// Flip/Flop.
	/// Instead of using ordinary variables, a class is used to emphasize the circuitry nature.
	/// </summary>
	class FF
	{
		TriState g = TriState::Zero;

	public:

		void set(TriState val);
		TriState get();
		TriState nget();
	};

	/// <summary>
	/// 1-mux
	/// </summary>
	/// <param name="sel"></param>
	/// <param name="in0"></param>
	/// <param name="in1"></param>
	/// <returns></returns>
	TriState MUX(TriState sel, TriState in0, TriState in1);

	/// <summary>
	/// 2-mux
	/// </summary>
	/// <param name="sel"></param>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState MUX2(TriState sel[2], TriState in[4]);

	/// <summary>
	/// 3-mux
	/// </summary>
	/// <param name="sel"></param>
	/// <param name="in"></param>
	/// <returns></returns>
	TriState MUX3(TriState sel[3], TriState in[8]);

	/// <summary>
	/// DMX 2-to-4
	/// </summary>
	/// <param name="in"></param>
	/// <param name="out"></param>
	void DMX2(TriState in[2], TriState out[4]);

	/// <summary>
	/// DMX 3-to-8
	/// </summary>
	/// <param name="in"></param>
	/// <param name="out"></param>
	void DMX3(TriState in[3], TriState out[8]);

	/// <summary>
	/// DMX 4-to-16
	/// </summary>
	/// <param name="in"></param>
	/// <param name="out"></param>
	void DMX4(TriState in[4], TriState out[16]);

	/// <summary>
	/// Basic decoder 2-to-4. The input takes a value (bitwise) - the output is the number of the active output.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	size_t Decoder2(TriState in[2]);

	/// <summary>
	/// Basic decoder 3-to-8. The input takes a value (bitwise) - the output is the number of the active output.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	size_t Decoder3(TriState in[3]);

	/// <summary>
	/// Generalized PLA matrix emulator.
	/// Although PLA is a combinatorial element, it is made as a class because of its complexity.
	/// </summary>
	class PLA
	{
		uint8_t* rom = nullptr;			// ROM Matrix. We do not use `TriState` to define transistors in the `1` places.
		size_t romSize = 0;				// ROM matrix size in bytes
		size_t romInputs = 0;			// Saved number of decoder inputs (set in the constructor)
		size_t romOutputs = 0;			// Saved number of decoder outputs (set in the constructor)

		TriState* outs = nullptr;
		TriState* unomptimized_out = nullptr;

		void sim_Unomptimized(size_t input_bits, TriState** outputs);

		bool Optimize = true;
		char fname[0x100] = { 0 };

	public:
		PLA(size_t inputs, size_t outputs, char *filename);
		~PLA();

		/// <summary>
		/// Set the decoder matrix.
		/// </summary>
		/// <param name="bitmask">An array of bit values. msb corresponds to input `0`. lsb corresponds to input `romInputs-1`.</param>
		void SetMatrix(size_t bitmask[]);

		/// <summary>
		/// Simulate decoder.
		/// </summary>
		/// <param name="inputs">Input values (packed bits)</param>
		/// <param name="outputs">Output values. The number of outputs must correspond to the value defined in the constructor.</param>
		void sim(size_t input_bits, TriState** outputs);
	};

	/// <summary>
	/// Pack a bit vector into a byte.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	uint8_t Pack(TriState in[8]);

	/// <summary>
	/// Pack a bit vector into a byte.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	uint8_t Pack3(TriState in[3]);

	/// <summary>
	/// Pack a bit vector into a byte.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	uint8_t Pack5(TriState in[5]);

	/// <summary>
	/// Pack a bit vector into a nipple.
	/// </summary>
	/// <param name="in"></param>
	/// <returns></returns>
	uint8_t PackNibble(TriState in[4]);

	/// <summary>
	/// Unpack a byte into a bit vector.
	/// </summary>
	/// <param name="val"></param>
	/// <param name="out"></param>
	void Unpack(uint8_t val, TriState out[8]);

	/// <summary>
	/// Unpack a nibble into a bit vector.
	/// </summary>
	/// <param name="val"></param>
	/// <param name="out"></param>
	void UnpackNibble(uint8_t val, TriState out[4]);

	/// <summary>
	/// Dump vector.
	/// </summary>
	/// <param name="in"></param>
	void Dump(TriState in[8], const char* name);

	/// <summary>
	/// Connect the two buses using the "Ground Wins" rule.
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	void BusConnect(TriState& a, TriState& b);

	/// <summary>
	/// Reverse the bit order.
	/// </summary>
	/// <param name="in"></param>
	void BitRev(TriState in[8]);

	/// <summary>
	/// Convert TriState to byte.
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	uint8_t ToByte(TriState val);

	/// <summary>
	/// Convert the byte to the TriState type. This and the previous call can be used in TriState serialization.
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	TriState FromByte(uint8_t val);

	/// <summary>
	/// Check that the CLK is in the posedge state.
	/// </summary>
	/// <param name="prev_CLK">Previous CLK level</param>
	/// <param name="CLK">Current CLK level</param>
	/// <returns></returns>
	bool IsPosedge(TriState prev_CLK, TriState CLK);

	/// <summary>
	/// Check that the CLK is in the negedge state.
	/// </summary>
	/// <param name="prev_CLK">Previous CLK level</param>
	/// <param name="CLK">Current CLK level</param>
	/// <returns></returns>
	bool IsNegedge(TriState prev_CLK, TriState CLK);
}
