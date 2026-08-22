// Full unit test coverage of the BaseLogicLib primitives.
// https://github.com/emu-russia/breaknes/issues/500

#include "pch.h"

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(BaseLogicLibUnitTest)
	{
	public:

		TEST_METHOD(TestNOT)
		{
			Assert::IsTrue(NOT(TriState::Zero) == TriState::One);
			Assert::IsTrue(NOT(TriState::One) == TriState::Zero);
		}

		TEST_METHOD(TestNOR)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					TriState res = NOR(FromByte((uint8_t)a), FromByte((uint8_t)b));
					uint8_t expected = (uint8_t)((~(a | b)) & 1);
					if (ToByte(res) != expected)
					{
						Logger::WriteMessage(("NOR failed: " + std::to_string(a) + ", " + std::to_string(b)).c_str());
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestNOR3)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					for (size_t c = 0; c <= 1; c++)
					{
						TriState res = NOR3(FromByte((uint8_t)a), FromByte((uint8_t)b), FromByte((uint8_t)c));
						uint8_t expected = (uint8_t)((~(a | b | c)) & 1);
						if (ToByte(res) != expected)
						{
							Logger::WriteMessage(("NOR3 failed: " + std::to_string(a) + ", " + std::to_string(b) + ", " + std::to_string(c)).c_str());
							Assert::Fail();
						}
					}
				}
			}
		}

		/// <summary>
		/// Array + argument based NOR overloads (NOR4..NOR16, NOR25, NOR27, NOR28, NOR29).
		/// </summary>
		TEST_METHOD(TestNORn)
		{
			const size_t counts[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 25, 27, 28, 29 };

			for (size_t c : counts)
			{
				// All zeros -> One
				TriState* arr = new TriState[c];
				for (size_t i = 0; i < c; i++)
				{
					arr[i] = TriState::Zero;
				}

				TriState res = NOR_Array(c, arr);
				if (res != TriState::One)
				{
					Logger::WriteMessage(("NOR" + std::to_string(c) + " all-zeros failed").c_str());
					delete[] arr;
					Assert::Fail();
				}

				// Single one -> Zero
				for (size_t hot = 0; hot < c; hot++)
				{
					for (size_t i = 0; i < c; i++)
					{
						arr[i] = TriState::Zero;
					}
					arr[hot] = TriState::One;

					res = NOR_Array(c, arr);
					if (res != TriState::Zero)
					{
						Logger::WriteMessage(("NOR" + std::to_string(c) + " single-one failed at " + std::to_string(hot)).c_str());
						delete[] arr;
						Assert::Fail();
					}
				}

				delete[] arr;
			}
		}

		TEST_METHOD(TestNAND)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					TriState res = NAND(FromByte((uint8_t)a), FromByte((uint8_t)b));
					uint8_t expected = (uint8_t)((~(a & b)) & 1);
					if (ToByte(res) != expected)
					{
						Logger::WriteMessage(("NAND failed: " + std::to_string(a) + ", " + std::to_string(b)).c_str());
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestNAND3)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					for (size_t c = 0; c <= 1; c++)
					{
						TriState res = NAND3(FromByte((uint8_t)a), FromByte((uint8_t)b), FromByte((uint8_t)c));
						uint8_t expected = (uint8_t)((~((a & b) & c)) & 1);
						if (ToByte(res) != expected)
						{
							Logger::WriteMessage("NAND3 failed");
							Assert::Fail();
						}
					}
				}
			}
		}

		TEST_METHOD(TestAND)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					TriState res = AND(FromByte((uint8_t)a), FromByte((uint8_t)b));
					uint8_t expected = (uint8_t)(a & b);
					if (ToByte(res) != expected)
					{
						Logger::WriteMessage("AND failed");
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestAND3)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					for (size_t c = 0; c <= 1; c++)
					{
						TriState res = AND3(FromByte((uint8_t)a), FromByte((uint8_t)b), FromByte((uint8_t)c));
						uint8_t expected = (uint8_t)(((a & b) & c) & 1);
						if (ToByte(res) != expected)
						{
							Logger::WriteMessage("AND3 failed");
							Assert::Fail();
						}
					}
				}
			}
		}

		TEST_METHOD(TestAND4)
		{
			for (size_t v = 0; v < 0x10; v++)
			{
				TriState in[4];
				UnpackNibble((uint8_t)v, in);
				TriState res = AND4(in);
				uint8_t expected = (uint8_t)(((in[0] & in[1] & in[2] & in[3])) & 1);
				if (ToByte(res) != expected)
				{
					Logger::WriteMessage(("AND4 failed for value " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(TestOR)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					TriState res = OR(FromByte((uint8_t)a), FromByte((uint8_t)b));
					uint8_t expected = (uint8_t)((a | b) & 1);
					if (ToByte(res) != expected)
					{
						Logger::WriteMessage("OR failed");
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestOR3)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					for (size_t c = 0; c <= 1; c++)
					{
						TriState res = OR3(FromByte((uint8_t)a), FromByte((uint8_t)b), FromByte((uint8_t)c));
						uint8_t expected = (uint8_t)((a | b | c) & 1);
						if (ToByte(res) != expected)
						{
							Logger::WriteMessage("OR3 failed");
							Assert::Fail();
						}
					}
				}
			}
		}

		TEST_METHOD(TestXOR)
		{
			for (size_t a = 0; a <= 1; a++)
			{
				for (size_t b = 0; b <= 1; b++)
				{
					TriState res = XOR(FromByte((uint8_t)a), FromByte((uint8_t)b));
					uint8_t expected = (uint8_t)((a ^ b) & 1);
					if (ToByte(res) != expected)
					{
						Logger::WriteMessage("XOR failed");
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestDLatch)
		{
			DLatch l;

			// Initial state is Zero
			Assert::IsTrue(l.get() == TriState::Zero);

			// Latched on enable=1
			l.set(TriState::One, TriState::One);
			Assert::IsTrue(l.get() == TriState::One);
			Assert::IsTrue(l.nget() == TriState::Zero);

			// Disabled enable does not change state
			l.set(TriState::Zero, TriState::Zero);
			Assert::IsTrue(l.get() == TriState::One);

			// Enabled writes
			l.set(TriState::Zero, TriState::One);
			Assert::IsTrue(l.get() == TriState::Zero);

			// Floating input does not change state
			l.set(TriState::Z, TriState::One);
			Assert::IsTrue(l.get() == TriState::Zero);
			l.set(TriState::One, TriState::One);
			Assert::IsTrue(l.get() == TriState::One);
			l.set(TriState::Z, TriState::One);
			Assert::IsTrue(l.get() == TriState::One);
		}

		TEST_METHOD(TestFF)
		{
			FF f;

			Assert::IsTrue(f.get() == TriState::Zero);

			f.set(TriState::One);
			Assert::IsTrue(f.get() == TriState::One);
			Assert::IsTrue(f.nget() == TriState::Zero);

			f.set(TriState::Zero);
			Assert::IsTrue(f.get() == TriState::Zero);

			// Floating input does not change state
			f.set(TriState::One);
			f.set(TriState::Z);
			Assert::IsTrue(f.get() == TriState::One);
		}

		TEST_METHOD(TestMUX)
		{
			Assert::IsTrue(MUX(TriState::Zero, TriState::Zero, TriState::One) == TriState::Zero);
			Assert::IsTrue(MUX(TriState::One, TriState::Zero, TriState::One) == TriState::One);
		}

		TEST_METHOD(TestMUX2)
		{
			TriState in[4]{};
			TriState sel[2]{};

			for (size_t n = 0; n < _countof(in); n++)
			{
				sel[0] = n & 1 ? TriState::One : TriState::Zero;
				sel[1] = n & 2 ? TriState::One : TriState::Zero;

				for (size_t i = 0; i < _countof(in); i++)
				{
					in[i] = TriState::Zero;
				}

				in[n] = TriState::One;

				Assert::IsTrue(MUX2(sel, in) == TriState::One);
			}
		}

		TEST_METHOD(TestMUX3)
		{
			TriState in[8]{};
			TriState sel[3]{};

			for (size_t n = 0; n < _countof(in); n++)
			{
				sel[0] = n & 1 ? TriState::One : TriState::Zero;
				sel[1] = n & 2 ? TriState::One : TriState::Zero;
				sel[2] = n & 4 ? TriState::One : TriState::Zero;

				for (size_t i = 0; i < _countof(in); i++)
				{
					in[i] = TriState::Zero;
				}

				in[n] = TriState::One;

				Assert::IsTrue(MUX3(sel, in) == TriState::One);
			}
		}

		TEST_METHOD(TestDMX2)
		{
			for (size_t v = 0; v < 4; v++)
			{
				TriState in[2]{};
				in[0] = v & 1 ? TriState::One : TriState::Zero;
				in[1] = v & 2 ? TriState::One : TriState::Zero;

				TriState out[4]{};
				DMX2(in, out);

				for (size_t n = 0; n < 4; n++)
				{
					TriState expected = n == v ? TriState::One : TriState::Zero;
					if (out[n] != expected)
					{
						Logger::WriteMessage(("DMX2 failed for input " + std::to_string(v) + ", out " + std::to_string(n)).c_str());
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestDMX3)
		{
			for (size_t v = 0; v < 8; v++)
			{
				TriState in[3]{};
				in[0] = v & 1 ? TriState::One : TriState::Zero;
				in[1] = v & 2 ? TriState::One : TriState::Zero;
				in[2] = v & 4 ? TriState::One : TriState::Zero;

				TriState out[8]{};
				DMX3(in, out);

				for (size_t n = 0; n < 8; n++)
				{
					TriState expected = n == v ? TriState::One : TriState::Zero;
					if (out[n] != expected)
					{
						Logger::WriteMessage(("DMX3 failed for input " + std::to_string(v) + ", out " + std::to_string(n)).c_str());
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestDMX4)
		{
			for (size_t v = 0; v < 16; v++)
			{
				TriState in[4]{};
				in[0] = v & 1 ? TriState::One : TriState::Zero;
				in[1] = v & 2 ? TriState::One : TriState::Zero;
				in[2] = v & 4 ? TriState::One : TriState::Zero;
				in[3] = v & 8 ? TriState::One : TriState::Zero;

				TriState out[16]{};
				DMX4(in, out);

				for (size_t n = 0; n < 16; n++)
				{
					TriState expected = n == v ? TriState::One : TriState::Zero;
					if (out[n] != expected)
					{
						Logger::WriteMessage(("DMX4 failed for input " + std::to_string(v) + ", out " + std::to_string(n)).c_str());
						Assert::Fail();
					}
				}
			}
		}

		TEST_METHOD(TestDecoder2)
		{
			for (size_t v = 0; v < 4; v++)
			{
				TriState in[2]{};
				in[0] = v & 1 ? TriState::One : TriState::Zero;
				in[1] = v & 2 ? TriState::One : TriState::Zero;
				Assert::IsTrue(Decoder2(in) == v);
			}
		}

		TEST_METHOD(TestDecoder3)
		{
			for (size_t v = 0; v < 8; v++)
			{
				TriState in[3]{};
				in[0] = v & 1 ? TriState::One : TriState::Zero;
				in[1] = v & 2 ? TriState::One : TriState::Zero;
				in[2] = v & 4 ? TriState::One : TriState::Zero;
				Assert::IsTrue(Decoder3(in) == v);
			}
		}

		/// <summary>
		/// PLA 3-input/4-output test.
		/// out0 = NOT(in0), out1 = NOT(in1), out2 = NOT(in2), out3 = 1
		/// </summary>
		TEST_METHOD(TestPLA)
		{
			const size_t inputs = 3;
			const size_t outputs = 4;

			char fname[0x100];
			sprintf_s(fname, sizeof(fname), "UnitTest_pla_3x4.bin");
			remove(fname);		// Ensure a deterministic simulation

			PLA pla(inputs, outputs, fname);

			size_t bitmask[outputs] = {
				0b001,	// out0: transistor at input2 (lsb of bitmask = highest input index)
				0b010,	// out1: transistor at input1
				0b100,	// out2: transistor at input0 (msb of bitmask = input 0)
				0b000,	// out3: always 1
			};

			pla.SetMatrix(bitmask);

			TriState* out;
			for (size_t v = 0; v < (1 << inputs); v++)
			{
				pla.sim(v, &out);

				uint8_t expected0 = (uint8_t)((~((v >> 2) & 1)) & 1);	// NOT(in2)
				uint8_t expected1 = (uint8_t)((~((v >> 1) & 1)) & 1);	// NOT(in1)
				uint8_t expected2 = (uint8_t)((~(v & 1)) & 1);			// NOT(in0)

				if (ToByte(out[0]) != expected0 || ToByte(out[1]) != expected1 ||
					ToByte(out[2]) != expected2 || ToByte(out[3]) != 1)
				{
					Logger::WriteMessage(("PLA failed for input " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}

			remove(fname);
		}

		/// <summary>
		/// PLA with a bigger matrix (8 inputs / 2 outputs) to exercise the optimized table.
		/// </summary>
		TEST_METHOD(TestPLA_Big)
		{
			const size_t inputs = 8;
			const size_t outputs = 2;

			char fname[0x100];
			sprintf_s(fname, sizeof(fname), "UnitTest_pla_8x2.bin");
			remove(fname);

			PLA pla(inputs, outputs, fname);

			// out0 = NOR of the four low inputs (in0..in3)
			// out1 = NOR of the four high inputs (in4..in7)
			size_t bitmask[outputs] = {
				0b11110000,	// out0: transistors at inputs 0..3
				0b00001111,	// out1: transistors at inputs 4..7
			};

			pla.SetMatrix(bitmask);

			TriState* out;
			for (size_t v = 0; v < (1 << inputs); v++)
			{
				pla.sim(v, &out);

				size_t hi = (v >> 4) & 0xf;
				size_t lo = v & 0xf;

				uint8_t expected0 = (uint8_t)(lo == 0 ? 1 : 0);
				uint8_t expected1 = (uint8_t)(hi == 0 ? 1 : 0);

				if (ToByte(out[0]) != expected0 || ToByte(out[1]) != expected1)
				{
					Logger::WriteMessage(("PLA_Big failed for input " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}

			remove(fname);
		}

		TEST_METHOD(TestPack)
		{
			TriState in[8];
			for (size_t v = 0; v < 0x100; v++)
			{
				Unpack((uint8_t)v, in);
				if (Pack(in) != v)
				{
					Logger::WriteMessage(("Pack/Unpack roundtrip failed for value " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(TestPack3)
		{
			TriState in[3];
			for (size_t v = 0; v < 8; v++)
			{
				Unpack3((uint8_t)v, in);
				if (Pack3(in) != v)
				{
					Logger::WriteMessage(("Pack3/Unpack3 roundtrip failed for value " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(TestPack5)
		{
			TriState in[5];
			for (size_t v = 0; v < 32; v++)
			{
				Unpack5((uint8_t)v, in);
				if (Pack5(in) != v)
				{
					Logger::WriteMessage(("Pack5/Unpack5 roundtrip failed for value " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(TestPackNibble)
		{
			TriState in[4];
			for (size_t v = 0; v < 16; v++)
			{
				UnpackNibble((uint8_t)v, in);
				if (PackNibble(in) != v)
				{
					Logger::WriteMessage(("PackNibble/UnpackNibble roundtrip failed for value " + std::to_string(v)).c_str());
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(TestDump)
		{
			// Dump is a pure printf helper; just ensure it does not crash on all states.
			TriState in[8]{ TriState::Zero, TriState::One, TriState::Z, TriState::X, TriState::Zero, TriState::One, TriState::Z, TriState::X };
			Dump(in, "UnitTestDump");
		}

		TEST_METHOD(TestBusConnect)
		{
			// "Ground Wins" rule

			TriState a = TriState::One;
			TriState b = TriState::Zero;
			BusConnect(a, b);
			Assert::IsTrue(a == TriState::Zero && b == TriState::Zero);

			a = TriState::Zero;
			b = TriState::One;
			BusConnect(a, b);
			Assert::IsTrue(a == TriState::Zero && b == TriState::Zero);

			a = TriState::One;
			b = TriState::One;
			BusConnect(a, b);
			Assert::IsTrue(a == TriState::One && b == TriState::One);

			a = TriState::Zero;
			b = TriState::Z;
			BusConnect(a, b);
			Assert::IsTrue(a == TriState::Zero && b == TriState::Zero);

			a = TriState::One;
			b = TriState::Z;
			BusConnect(a, b);
			Assert::IsTrue(a == TriState::One && b == TriState::Z);
		}

		TEST_METHOD(TestBitRev)
		{
			TriState bits[8];

			Unpack(0b10000110, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0b01100001);

			Unpack(0x00, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0x00);

			Unpack(0xFF, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0xFF);

			Unpack(0x01, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0x80);

			Unpack(0x80, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0x01);

			Unpack(0xAA, bits);
			BitRev(bits);
			Assert::IsTrue(Pack(bits) == 0x55);
		}

		TEST_METHOD(TestToByteFromByte)
		{
			Assert::IsTrue(ToByte(TriState::Zero) == 0);
			Assert::IsTrue(ToByte(TriState::One) == 1);
			Assert::IsTrue(ToByte(TriState::Z) == 0xFF);
			Assert::IsTrue(ToByte(TriState::X) == 0xFE);

			Assert::IsTrue(FromByte(0) == TriState::Zero);
			Assert::IsTrue(FromByte(1) == TriState::One);
			Assert::IsTrue(FromByte(0xFF) == TriState::Z);
			Assert::IsTrue(FromByte(0xFE) == TriState::X);
		}

		TEST_METHOD(TestIsPosedge)
		{
			Assert::IsTrue(IsPosedge(TriState::Zero, TriState::One));
			Assert::IsFalse(IsPosedge(TriState::Zero, TriState::Zero));
			Assert::IsFalse(IsPosedge(TriState::One, TriState::One));
			Assert::IsFalse(IsPosedge(TriState::One, TriState::Zero));
		}

		TEST_METHOD(TestIsNegedge)
		{
			Assert::IsTrue(IsNegedge(TriState::One, TriState::Zero));
			Assert::IsFalse(IsNegedge(TriState::Zero, TriState::One));
			Assert::IsFalse(IsNegedge(TriState::Zero, TriState::Zero));
			Assert::IsFalse(IsNegedge(TriState::One, TriState::One));
		}

		TEST_METHOD(TestPullup)
		{
			TriState val = TriState::Z;
			Pullup(val);
			Assert::IsTrue(val == TriState::One);

			val = TriState::Zero;
			Pullup(val);
			Assert::IsTrue(val == TriState::Zero);

			val = TriState::One;
			Pullup(val);
			Assert::IsTrue(val == TriState::One);
		}

		TEST_METHOD(TestPulldown)
		{
			TriState val = TriState::Z;
			Pulldown(val);
			Assert::IsTrue(val == TriState::Zero);

			val = TriState::Zero;
			Pulldown(val);
			Assert::IsTrue(val == TriState::Zero);

			val = TriState::One;
			Pulldown(val);
			Assert::IsTrue(val == TriState::One);
		}

	private:

		/// <summary>
		/// Helper that calls the right NOR overload by arity.
		/// </summary>
		static TriState NOR_Array(size_t count, TriState in[])
		{
			switch (count)
			{
				case 4: return NOR4(in);
				case 5: return NOR5(in);
				case 6: return NOR6(in);
				case 7: return NOR7(in);
				case 8: return NOR8(in);
				case 9: return NOR9(in);
				case 10: return NOR10(in);
				case 11: return NOR11(in);
				case 12: return NOR12(in);
				case 13: return NOR13(in);
				case 15: return NOR15(in);
				case 16: return NOR16(in);
				case 25: return NOR25(in);
				case 27: return NOR27(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], in[19], in[20], in[21], in[22], in[23], in[24], in[25], in[26]);
				case 28: return NOR28(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], in[19], in[20], in[21], in[22], in[23], in[24], in[25], in[26], in[27]);
				case 29: return NOR29(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], in[19], in[20], in[21], in[22], in[23], in[24], in[25], in[26], in[27], in[28]);
			}
			Assert::Fail();
			return TriState::X;
		}
	};
}
