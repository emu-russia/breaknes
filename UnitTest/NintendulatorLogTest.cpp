// Nintendulator log format unit tests.

#include "pch.h"

#include "../Breaknes/BreaksCore/NintendulatorLog.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NintendulatorLogUnitTest
{
	static uint8_t test_mem[0x10000];

	uint8_t ReadMem(void* opaque, uint16_t addr)
	{
		return test_mem[addr];
	}

	TEST_CLASS(NintendulatorLogTest)
	{
	public:

		/// <summary>
		/// Write one trace entry and return the exact line that was written to the log file.
		/// </summary>
		static std::string LogOneLine(const NintendulatorLog::Entry& entry)
		{
			// Make sure the file starts empty.
			remove("Nintendulator.log");

			NintendulatorLog log;
			log.SetReadByteCallback(ReadMem, nullptr);
			log.Enable();
			Assert::IsTrue(log.Enabled());

			log.LogInstruction(entry);
			log.Disable();
			Assert::IsFalse(log.Enabled());

			FILE* f = fopen("Nintendulator.log", "rb");
			Assert::IsNotNull(f);

			std::string line;
			char buf[256];
			size_t n = fread(buf, 1, sizeof(buf) - 1, f);
			fclose(f);
			remove("Nintendulator.log");

			buf[n] = 0;
			line = buf;
			return line;
		}

		static std::string HexDump(const std::string& s)
		{
			std::string out;
			char tmp[8];
			for (size_t i = 0; i < s.size(); i++)
			{
				sprintf_s(tmp, "%02X ", (unsigned char)s[i]);
				out += tmp;
			}
			return out;
		}

		static void AssertLine(const std::string& expected, const NintendulatorLog::Entry& entry)
		{
			std::string actual = LogOneLine(entry);
			if (actual != expected)
			{
				Logger::WriteMessage(("Expected len=" + std::to_string(expected.size()) + ": " + HexDump(expected)).c_str());
				Logger::WriteMessage(("Actual   len=" + std::to_string(actual.size()) + ": " + HexDump(actual)).c_str());
			}
			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(TestImplied)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC000] = 0x78;	// SEI

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC000;
			entry.a = 0x12;
			entry.x = 0x34;
			entry.y = 0x56;
			entry.s = 0xFD;
			entry.p = 0xA5;
			entry.phi_counter = 30;
			entry.h_counter = 10;
			entry.v_counter = 20;

			std::string expected =
				"C000  78        SEI                             A:12 X:34 Y:56 P:A5 SP:FD PPU: 10, 20 CYC:30\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestImmediate)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC001] = 0xA9;	// LDA #$00
			test_mem[0xC002] = 0x00;

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC001;
			entry.a = 0x12;
			entry.x = 0x34;
			entry.y = 0x56;
			entry.s = 0xFD;
			entry.p = 0xA5;
			entry.phi_counter = 31;
			entry.h_counter = 10;
			entry.v_counter = 20;

			std::string expected =
				"C001  A9 00     LDA #$00                        A:12 X:34 Y:56 P:A5 SP:FD PPU: 10, 20 CYC:31\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestAbsolute)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC003] = 0x8D;	// STA $2000
			test_mem[0xC004] = 0x00;
			test_mem[0xC005] = 0x20;
			test_mem[0x2000] = 0xAB;	// value shown in the hint

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC003;
			entry.a = 0xAB;
			entry.x = 0x34;
			entry.y = 0x56;
			entry.s = 0xFD;
			entry.p = 0xA5;
			entry.phi_counter = 32;
			entry.h_counter = 10;
			entry.v_counter = 20;

			std::string expected =
				"C003  8D 00 20  STA $2000 = AB                  A:AB X:34 Y:56 P:A5 SP:FD PPU: 10, 20 CYC:32\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestAbsoluteIndexed)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC010] = 0x9D;	// STA $4000,X
			test_mem[0xC011] = 0x00;
			test_mem[0xC012] = 0x40;
			test_mem[0x4017] = 0xFF;	// effective address = $4000 + X = $4017

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC010;
			entry.a = 0x00;
			entry.x = 0x17;
			entry.y = 0x00;
			entry.s = 0xFF;
			entry.p = 0x24;
			entry.phi_counter = 57189;
			entry.h_counter = 23;
			entry.v_counter = 241;

			std::string expected =
				"C010  9D 00 40  STA $4000,X @ 4017 = FF         A:00 X:17 Y:00 P:24 SP:FF PPU: 23,241 CYC:57189\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestRelative)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC009] = 0x10;	// BPL $C006 (backward branch)
			test_mem[0xC00A] = 0xFB;

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC009;
			entry.a = 0x12;
			entry.x = 0x34;
			entry.y = 0x56;
			entry.s = 0xFD;
			entry.p = 0xA5;
			entry.phi_counter = 34;
			entry.h_counter = 10;
			entry.v_counter = 20;

			std::string expected =
				"C009  10 FB     BPL $C006                       A:12 X:34 Y:56 P:A5 SP:FD PPU: 10, 20 CYC:34\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestZeroPage)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC020] = 0xA5;	// LDA $10
			test_mem[0xC021] = 0x10;
			test_mem[0x0010] = 0x42;

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC020;
			entry.a = 0x00;
			entry.x = 0x00;
			entry.y = 0x00;
			entry.s = 0xFD;
			entry.p = 0x26;
			entry.phi_counter = 40;
			entry.h_counter = 0;
			entry.v_counter = 0;

			std::string expected =
				"C020  A5 10     LDA $10 = 42                    A:00 X:00 Y:00 P:26 SP:FD PPU:  0,  0 CYC:40\r\n";

			AssertLine(expected, entry);
		}

		TEST_METHOD(TestIndirectY)
		{
			memset(test_mem, 0, sizeof(test_mem));
			test_mem[0xC030] = 0xB1;	// LDA ($20),Y
			test_mem[0xC031] = 0x20;
			test_mem[0x0020] = 0x00;	// pointer low
			test_mem[0x0021] = 0x80;	// pointer high -> $8000
			test_mem[0x8004] = 0x7E;	// effective address = $8000 + Y = $8004

			NintendulatorLog::Entry entry{};
			entry.pc = 0xC030;
			entry.a = 0x00;
			entry.x = 0x00;
			entry.y = 0x04;
			entry.s = 0xFD;
			entry.p = 0x26;
			entry.phi_counter = 45;
			entry.h_counter = 100;
			entry.v_counter = 50;

			std::string expected =
				"C030  B1 20     LDA ($20),Y = 8000 @ 8004 = 7E  A:00 X:00 Y:04 P:26 SP:FD PPU:100, 50 CYC:45\r\n";

			AssertLine(expected, entry);
		}
	};
}
