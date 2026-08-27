// Unit tests for the CartPcb component: PcbFactory (board JSON -> Pcb),
// Pcb simulation (NROM/UNROM/AOROM behavior) and NesCartDb (CRC identification).

#include "pch.h"

#include <string>
#include <cstring>
#include <cstdio>

#include "../CartPcb/CartPcb.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace BaseLogic;

namespace UnitTest
{
	namespace
	{
		using namespace CartPcb;

		// Note on the CHR chip select: on NROM-style boards the CHR /CE is tied to
		// PPU A13 (low for $0000-$1FFF). The connector signal nPA13 = NOT(PPU A13),
		// so the chip select is `!nPA13` (asserted when PPU A13 = 0).

		const char* NROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"NES-NROM-256\", \"pcb\" : \"NES-NROM-256-02\", \"mapper\" : 0,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\", \"size\" : 32768 },"
			"      \"chr\" : { \"kind\" : \"rom\", \"bus\" : \"ppu\", \"size\" : 8192 } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"hardwired\", \"h\" : 0, \"v\" : 1 },"
			"      \"cpu\" : { \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"cpu_addr[13:0]\" } },"
			"      \"ppu\" : { \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : [] } } }";

		const char* UNROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"NES-UNROM\", \"pcb\" : \"NES-UNROM-10\", \"mapper\" : 2,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\", \"size\" : 131072 },"
			"      \"chr\" : { \"kind\" : \"rom\", \"bus\" : \"ppu\", \"size\" : 8192 },"
			"      \"bank\" : { \"kind\" : \"latch\" } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"hardwired\", \"h\" : 1, \"v\" : 0 },"
			"      \"cpu\" : {"
			"        \"prg_lo\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL | cpu_addr[14]\", \"addr\" : \"bank.Q2..Q0 | cpu_addr[13:0]\" },"
			"        \"prg_hi\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL | !cpu_addr[14]\", \"addr\" : \"bank.Q2 | vdd | vdd | cpu_addr[13:0]\" },"
			"        \"bank\" : { \"clk\" : \"nROMSEL\", \"n_we\" : \"RnW\" } },"
			"      \"ppu\" : { \"chr\" : { \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : [] } } }";

		const char* AOROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"NES-AOROM\", \"pcb\" : \"NES-AOROM-01\", \"mapper\" : 7,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\", \"size\" : 262144 },"
			"      \"chr\" : { \"kind\" : \"ram\", \"bus\" : \"ppu\", \"size\" : 8192 },"
			"      \"bank\" : { \"kind\" : \"latch\" } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"mapper\", \"net\" : \"bank.Q4\" },"
			"      \"cpu\" : {"
			"        \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"bank.Q2..Q0 | cpu_addr[14:0]\" },"
			"        \"bank\" : { \"clk\" : \"nROMSEL\", \"n_we\" : \"RnW\", \"data\" : \"cpu_data[7:0]\" } },"
			"      \"ppu\" : { \"chr\" : { \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"n_we\" : \"nWR\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : [] } } }";

		// A test bus with distinct state for the CPU/PPU/expansion buses.
		struct Bus
		{
			TriState in[(size_t)CartInput::Max]{};
			TriState out[(size_t)CartOutput::Max]{};

			uint8_t cpu_data = 0;
			bool cpu_dirty = false;

			uint8_t ppu_data = 0;
			bool ppu_dirty = false;

			bool exp_dirty = false;

			void SetDefaults()
			{
				for (size_t n = 0; n < (size_t)CartInput::Max; n++)
				{
					in[n] = TriState::One;
				}
				in[(size_t)CartInput::M2] = TriState::One;
				in[(size_t)CartInput::nROMSEL] = TriState::Zero;	// $8000+ window
				in[(size_t)CartInput::nRD] = TriState::Zero;
				in[(size_t)CartInput::nWR] = TriState::One;
				in[(size_t)CartInput::RnW] = TriState::One;
				in[(size_t)CartInput::nPA13] = TriState::One;		// PPU $0000-$1FFF
			}

			void Sim(Pcb* pcb, uint16_t cpu_addr, uint16_t ppu_addr)
			{
				cpu_data = 0;
				cpu_dirty = false;
				ppu_data = 0;
				ppu_dirty = false;
				exp_dirty = false;

				pcb->sim(in, out, cpu_addr, &cpu_data, cpu_dirty, ppu_addr, &ppu_data, ppu_dirty, nullptr, nullptr, exp_dirty);
			}
		};

		Pcb* MakePcb(const char* json, const CartImage& image, std::string& error)
		{
			return PcbFactory::Create(json, image, error);
		}
	}

	TEST_CLASS(CartPcbFactoryUnitTest)
	{
	public:
		TEST_METHOD(TestParseLogicExpr)
		{
			std::string error;

			auto e1 = PcbFactory::ParseLogicExpr("nROMSEL & !cpu_addr[14]", error);
			Assert::IsTrue(e1 != nullptr);
			Assert::IsTrue(e1->op == LogicExpr::Op::And);
			Assert::IsTrue(e1->a->atom == LogicExpr::AtomKind::Signal && e1->a->name == "nROMSEL");
			Assert::IsTrue(e1->b->op == LogicExpr::Op::Not);
			Assert::IsTrue(e1->b->a->atom == LogicExpr::AtomKind::CpuAddrBit && e1->b->a->bit == 14);

			auto e2 = PcbFactory::ParseLogicExpr("bank.Q4", error);
			Assert::IsTrue(e2 != nullptr);
			Assert::IsTrue(e2->atom == LogicExpr::AtomKind::ChipPin && e2->chip == "bank" && e2->name == "Q4");

			auto e3 = PcbFactory::ParseLogicExpr("gnd", error);
			Assert::IsTrue(e3 != nullptr && e3->atom == LogicExpr::AtomKind::Gnd);

			// Invalid expression must fail
			auto e4 = PcbFactory::ParseLogicExpr("nROMSEL @", error);
			Assert::IsTrue(e4 == nullptr);
		}

		TEST_METHOD(TestParseAddrExpr)
		{
			std::string error;

			auto e1 = PcbFactory::ParseAddrExpr("bank.Q2..Q0 | cpu_addr[13:0]", error);
			Assert::IsTrue(e1 != nullptr && e1->terms.size() == 2);
			Assert::IsTrue(e1->terms[0].kind == AddrTerm::Kind::ChipPinList);
			Assert::IsTrue(e1->terms[0].pins.size() == 3);
			Assert::IsTrue(e1->terms[0].pins[0] == "Q2" && e1->terms[0].pins[1] == "Q1" && e1->terms[0].pins[2] == "Q0");
			Assert::IsTrue(e1->terms[1].kind == AddrTerm::Kind::CpuAddrRange && e1->terms[1].hi == 13 && e1->terms[1].lo == 0);

			auto e2 = PcbFactory::ParseAddrExpr("mmc1.PRG_A14..PRG_A17 | cpu_addr[13:0]", error);
			Assert::IsTrue(e2 != nullptr && e2->terms[0].pins.size() == 4);
			Assert::IsTrue(e2->terms[0].pins[0] == "PRG_A14" && e2->terms[0].pins[3] == "PRG_A17");

			auto e3 = PcbFactory::ParseAddrExpr("cpu_addr[14]", error);
			Assert::IsTrue(e3 != nullptr && e3->terms[0].hi == 14 && e3->terms[0].lo == 14);

			auto e4 = PcbFactory::ParseAddrExpr("bank.Q2 | vdd | vdd | cpu_addr[13:0]", error);
			Assert::IsTrue(e4 != nullptr && e4->terms.size() == 4);
			Assert::IsTrue(e4->terms[1].kind == AddrTerm::Kind::Vdd);
		}

		TEST_METHOD(TestCreateNromBoard)
		{
			uint8_t prg[32768]{};
			uint8_t chr[8192]{};
			prg[0x1234] = 0xAB;
			chr[0x456] = 0xCD;

			CartImage image;
			image.prg = prg;
			image.prgSize = sizeof(prg);
			image.chr = chr;
			image.chrSize = sizeof(chr);

			std::string error;
			Pcb* pcb = MakePcb(NROM_BOARD, image, error);
			Assert::IsTrue(pcb != nullptr);
			Assert::IsTrue(pcb->GetBoardType() == "NES-NROM-256");

			delete pcb;
		}
	};

	TEST_CLASS(CartPcbSimUnitTest)
	{
	public:
		TEST_METHOD(TestNromRead)
		{
			uint8_t prg[32768]{};
			uint8_t chr[8192]{};
			prg[0x1234] = 0xAB;
			chr[0x456] = 0xCD;

			CartImage image;
			image.prg = prg;
			image.prgSize = sizeof(prg);
			image.chr = chr;
			image.chrSize = sizeof(chr);

			std::string error;
			Pcb* pcb = MakePcb(NROM_BOARD, image, error);
			Assert::IsTrue(pcb != nullptr);

			Bus b;

			// CPU read $9234 (cart sees $1234)
			b.SetDefaults();
			b.Sim(pcb, 0x9234, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == 0xAB);

			// CPU read below $8000: cartridge does not drive the bus
			b.SetDefaults();
			b.in[(size_t)CartInput::nROMSEL] = TriState::One;
			b.Sim(pcb, 0x1234, 0x2000);
			Assert::IsTrue(b.cpu_dirty == false);

			// PPU read $0456
			b.SetDefaults();
			b.Sim(pcb, 0x8000, 0x0456);
			Assert::IsTrue(b.ppu_dirty == true);
			Assert::IsTrue(b.ppu_data == 0xCD);

			// Mirroring: hardwired vertical (v=1) -> VRAM_A10 = ppu_addr[10]
			b.SetDefaults();
			b.Sim(pcb, 0x8000, (1 << 10));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::One);

			b.SetDefaults();
			b.Sim(pcb, 0x8000, 0);
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::Zero);

			// Dbg_ReadPRGByte
			Assert::IsTrue(pcb->Dbg_ReadPRGByte(0x9234) == 0xAB);
			Assert::IsTrue(pcb->Dbg_ReadPRGByte(0x1234) == 0);

			delete pcb;
		}

		TEST_METHOD(TestUnromBankSwitching)
		{
			uint8_t prg[131072]{};
			uint8_t chr[8192]{};

			for (size_t n = 0; n < sizeof(prg); n++)
			{
				prg[n] = (uint8_t)(n >> 8);		// bank index in the high byte
			}

			CartImage image;
			image.prg = prg;
			image.prgSize = sizeof(prg);
			image.chr = chr;
			image.chrSize = sizeof(chr);

			std::string error;
			Pcb* pcb = MakePcb(UNROM_BOARD, image, error);
			Assert::IsTrue(pcb != nullptr);

			Bus b;

			// Write $8000 with data 5 -> bank register = 5
			b.SetDefaults();
			b.in[(size_t)CartInput::RnW] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.cpu_data = 5;
			pcb->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			// Read $8000+0x100 (cart $0100): bank 5 -> prg[5*0x4000 + 0x100]
			b.SetDefaults();
			b.Sim(pcb, 0x0100, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == (uint8_t)((5 * 0x4000 + 0x100) >> 8));

			// Read $C000+0x100 (cart $4100): fixed bank (Q2=1 -> bank 7)
			b.SetDefaults();
			b.Sim(pcb, 0x4100, 0x2000);
			Assert::IsTrue(b.cpu_data == (uint8_t)((7 * 0x4000 + 0x100) >> 8));

			// Dbg_ReadPRGByte must reflect the bank state
			Assert::IsTrue(pcb->Dbg_ReadPRGByte(0x8100) == (uint8_t)((5 * 0x4000 + 0x100) >> 8));

			delete pcb;
		}

		TEST_METHOD(TestAoromBankAndMirroring)
		{
			uint8_t prg[131072]{};
			uint8_t chr[8192]{};

			for (size_t n = 0; n < sizeof(prg); n++)
			{
				prg[n] = (uint8_t)(n >> 15);	// 32K bank index
			}

			CartImage image;
			image.prg = prg;
			image.prgSize = sizeof(prg);
			image.chr = chr;
			image.chrSize = sizeof(chr);

			std::string error;
			Pcb* pcb = MakePcb(AOROM_BOARD, image, error);
			Assert::IsTrue(pcb != nullptr);

			Bus b;

			// Write $8000, data = 0b1_0011: bank 3, mirroring bit (D4) = 1
			b.SetDefaults();
			b.in[(size_t)CartInput::RnW] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.cpu_data = 0x13;
			pcb->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			// Read $8000+0x123: 32K bank 3 -> prg[3*0x8000 + 0x123]
			b.SetDefaults();
			b.Sim(pcb, 0x0123, 0x2000);
			Assert::IsTrue(b.cpu_data == (uint8_t)((3 * 0x8000 + 0x123) >> 15));

			// Mirroring: VRAM_A10 = Q4 = 1 (one-screen)
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::One);

			// CHR-RAM write then read
			b.SetDefaults();
			b.in[(size_t)CartInput::nROMSEL] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::RnW] = TriState::One;
			b.ppu_data = 0x77;
			pcb->sim(b.in, b.out, 0x1234, &b.cpu_data, b.cpu_dirty, 0x0123, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			b.SetDefaults();
			b.in[(size_t)CartInput::nROMSEL] = TriState::One;
			b.Sim(pcb, 0x1234, 0x0123);
			Assert::IsTrue(b.ppu_dirty == true);
			Assert::IsTrue(b.ppu_data == 0x77);

			delete pcb;
		}
	};

	TEST_CLASS(CartPcbNesCartDbUnitTest)
	{
	public:
		TEST_METHOD(TestCrc32)
		{
			const char* data = "123456789";
			Assert::IsTrue(NesCartDb::Crc32((const uint8_t*)data, 9) == 0xCBF43926u);
		}

		TEST_METHOD(TestFindBoards)
		{
			const char* index =
				"[ { \"prg_crc\" : \"D3D248C9\", \"chr_crc\" : \"9C124A53\", \"system\" : \"Famicom\","
				"    \"type\" : \"IREM-NROM-128\", \"pcb\" : \"IREM-01-V\", \"mapper\" : 0 },"
				"  { \"prg_crc\" : \"AAAAAAAA\", \"chr_crc\" : \"BBBBBBBB\", \"system\" : \"NES-NTSC\","
				"    \"type\" : \"NES-UNROM\", \"pcb\" : \"NES-UNROM-10\", \"mapper\" : 2 } ]";

			// Write a temporary index file next to the test dll.
			const char* path = "test_index.json";
			FILE* f = fopen(path, "wb");
			Assert::IsTrue(f != nullptr);
			fwrite(index, 1, strlen(index), f);
			fclose(f);

			NesCartDb db;
			Assert::IsTrue(db.Load(path));
			Assert::IsTrue(db.IsLoaded());

			std::vector<BoardRef> out;
			db.FindBoards(0xD3D248C9u, 0x9C124A53u, out);
			Assert::IsTrue(out.size() == 1);
			Assert::IsTrue(out[0].type == "IREM-NROM-128");
			Assert::IsTrue(out[0].pcb == "IREM-01-V");
			Assert::IsTrue(out[0].mapper == 0);
			Assert::IsTrue(out[0].system == "Famicom");

			db.FindBoards(0xDEADBEEFu, 0x12345678u, out);
			Assert::IsTrue(out.empty());

			remove(path);
		}
	};
}
