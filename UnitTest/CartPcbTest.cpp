// Unit tests for the CartPcb component: PcbFactory (board JSON -> Pcb),
// Pcb simulation (NROM/UNROM/AOROM behavior) and NesCartDb (CRC identification).

#include "pch.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <direct.h>

#include "../CartPcb/CartPcb.h"
#include "../Mappers/pch.h"

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

			void Sim(Mappers::AbstractCartridge* cart, uint16_t cpu_addr, uint16_t ppu_addr)
			{
				cpu_data = 0;
				cpu_dirty = false;
				ppu_data = 0;
				ppu_dirty = false;
				exp_dirty = false;

				cart->sim(in, out, cpu_addr, &cpu_data, cpu_dirty, ppu_addr, &ppu_data, ppu_dirty, nullptr, nullptr, exp_dirty);
			}
		};

		Pcb* MakePcb(const char* json, const CartImage& image, std::string& error)
		{
			return PcbFactory::Create(json, image, error);
		}

		const char* SGROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"HVC-SGROM\", \"pcb\" : \"HVC-SGROM-03\", \"mapper\" : 1,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\" },"
			"      \"chr\" : { \"kind\" : \"rom\", \"bus\" : \"ppu\" },"
			"      \"mmc1\" : { \"kind\" : \"chip\", \"chip\" : \"MMC1\" } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"mapper\", \"net\" : \"mmc1.VRAM_A10\" },"
			"      \"cpu\" : {"
			"        \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"mmc1.PRG_nCE\", \"addr\" : \"mmc1.PRG_A17..PRG_A14 | cpu_addr[13:0]\" } },"
			"      \"ppu\" : {"
			"        \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"addr\" : \"mmc1.CHR_A16..CHR_A12 | ppu_addr[11:0]\" } },"
			"      \"nets\" : ["
			"        { \"name\" : \"mmc1.M2\",      \"from\" : \"M2\" },"
			"        { \"name\" : \"mmc1.nROMSEL\", \"from\" : \"nROMSEL\" },"
			"        { \"name\" : \"mmc1.CPU_RnW\", \"from\" : \"RnW\" },"
			"        { \"name\" : \"mmc1.CPU_A13\", \"from\" : \"cpu_addr[13]\" },"
			"        { \"name\" : \"mmc1.CPU_A14\", \"from\" : \"cpu_addr[14]\" },"
			"        { \"name\" : \"mmc1.CPU_D0\",  \"from\" : \"cpu_data[0]\" },"
			"        { \"name\" : \"mmc1.CPU_D7\",  \"from\" : \"cpu_data[7]\" },"
			"        { \"name\" : \"mmc1.PPU_A10\", \"from\" : \"ppu_addr[10]\" },"
			"        { \"name\" : \"mmc1.PPU_A11\", \"from\" : \"ppu_addr[11]\" },"
			"        { \"name\" : \"mmc1.PPU_A12\", \"from\" : \"ppu_addr[12]\" } ] } } }";

		// One scripted bus state for the parity tests.
		struct Step
		{
			TriState M2 = TriState::One;
			TriState nROMSEL = TriState::One;
			TriState RnW = TriState::One;
			TriState nRD = TriState::One;
			TriState nWR = TriState::One;
			TriState nPA13 = TriState::One;
			uint16_t cpu_addr = 0;
			uint16_t ppu_addr = 0;
			uint8_t cpu_data = 0;
			uint8_t ppu_data = 0;
		};

		// Run the same step through two cartridges and compare every observable.
		bool ParityStep(Mappers::AbstractCartridge& a, Mappers::AbstractCartridge& b, const Step& s)
		{
			TriState in1[(size_t)CartInput::Max]{};
			TriState in2[(size_t)CartInput::Max]{};

			for (size_t n = 0; n < (size_t)CartInput::Max; n++)
			{
				in1[n] = TriState::One;
				in2[n] = TriState::One;
			}

			in1[(size_t)CartInput::M2] = s.M2;
			in2[(size_t)CartInput::M2] = s.M2;
			in1[(size_t)CartInput::nROMSEL] = s.nROMSEL;
			in2[(size_t)CartInput::nROMSEL] = s.nROMSEL;
			in1[(size_t)CartInput::RnW] = s.RnW;
			in2[(size_t)CartInput::RnW] = s.RnW;
			in1[(size_t)CartInput::nRD] = s.nRD;
			in2[(size_t)CartInput::nRD] = s.nRD;
			in1[(size_t)CartInput::nWR] = s.nWR;
			in2[(size_t)CartInput::nWR] = s.nWR;
			in1[(size_t)CartInput::nPA13] = s.nPA13;
			in2[(size_t)CartInput::nPA13] = s.nPA13;

			TriState out1[(size_t)CartOutput::Max]{};
			TriState out2[(size_t)CartOutput::Max]{};

			// The old implementations normalize uninitialized outputs to Z; start
			// both sides at Z so the comparison is meaningful.
			for (size_t n = 0; n < (size_t)CartOutput::Max; n++)
			{
				out1[n] = TriState::Z;
				out2[n] = TriState::Z;
			}

			uint8_t c1 = s.cpu_data, c2 = s.cpu_data;
			bool d1 = false, d2 = false;
			uint8_t p1 = s.ppu_data, p2 = s.ppu_data;
			bool pd1 = false, pd2 = false;
			bool e1 = false, e2 = false;

			a.sim(in1, out1, s.cpu_addr, &c1, d1, s.ppu_addr, &p1, pd1, nullptr, nullptr, e1);
			b.sim(in2, out2, s.cpu_addr, &c2, d2, s.ppu_addr, &p2, pd2, nullptr, nullptr, e2);

			if (c1 != c2 || d1 != d2 || p1 != p2 || pd1 != pd2)
			{
				Logger::WriteMessage(("PARITY DIFF cpu_data=" + std::to_string(c1) + " vs " + std::to_string(c2) +
					" cpu_dirty=" + std::to_string(d1) + "/" + std::to_string(d2) +
					" ppu_data=" + std::to_string(p1) + "/" + std::to_string(p2) +
					" ppu_dirty=" + std::to_string(pd1) + "/" + std::to_string(pd2)).c_str());
				return false;
			}

			for (size_t n = 0; n < (size_t)CartOutput::Max; n++)
			{
				if (out1[n] != out2[n])
				{
					Logger::WriteMessage(("PARITY DIFF out[" + std::to_string(n) + "]=" + std::to_string((int)out1[n]) + " vs " + std::to_string((int)out2[n])).c_str());
					return false;
				}
			}

			return true;
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

	/// <summary>
	/// A/B parity: the old Mappers implementations vs the new CartPcb boards on
	/// the same .nes images. The legacy mappers register memory regions with the
	/// global DebugHub, so a hub instance is provided for the duration of the test.
	/// </summary>
	TEST_CLASS(CartPcbParityUnitTest)
	{
	public:
		TEST_METHOD(TestParityNrom)
		{
			uint8_t image[16 + 0x8000 + 0x2000];
			memset(image, 0, sizeof(image));

			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 2;	// 32K PRG
			image[5] = 1;	// 8K CHR
			image[6] = 1;	// vertical mirroring
			image[7] = 0;	// mapper 0

			for (size_t i = 0; i < 0x8000; i++) image[16 + i] = (uint8_t)(i * 7);
			for (size_t i = 0; i < 0x2000; i++) image[16 + 0x8000 + i] = (uint8_t)(i * 13);

			DebugHub hub;
			DebugHub* saved = dbg_hub;
			dbg_hub = &hub;

			{
				Mappers::NROM old(ConnectorType::FamicomStyle, image, sizeof(image));
				Assert::IsTrue(old.Valid());

				CartImage ci;
				ci.prg = image + 16;
				ci.prgSize = 0x8000;
				ci.chr = image + 16 + 0x8000;
				ci.chrSize = 0x2000;

				std::string error;
				Pcb* pcb = MakePcb(NROM_BOARD, ci, error);
				Assert::IsTrue(pcb != nullptr);
				CartPcbCartridge nw(ConnectorType::FamicomStyle, pcb);

				for (int step = 0; step < 64; step++)
				{
					Step s;
					s.nROMSEL = (step & 1) ? TriState::Zero : TriState::One;
					s.nRD = (step & 1) ? TriState::Zero : TriState::One;
					s.cpu_addr = (uint16_t)((step * 0x211) & 0x7fff);
					s.ppu_addr = (uint16_t)((step * 0x137) & 0x3fff);
					s.nPA13 = (s.ppu_addr & 0x2000) ? TriState::Zero : TriState::One;
					Assert::IsTrue(ParityStep(old, nw, s));
				}
			}

			dbg_hub = saved;
		}

		TEST_METHOD(TestParityUnrom)
		{
			uint8_t image[16 + 0x20000];
			memset(image, 0, sizeof(image));

			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 8;	// 128K PRG
			image[5] = 0;	// CHR-RAM (none in the image)
			image[6] = 0;	// horizontal mirroring
			image[7] = 2;	// mapper 2

			for (size_t i = 0; i < 0x20000; i++) image[16 + i] = (uint8_t)(i >> 8);

			DebugHub hub;
			DebugHub* saved = dbg_hub;
			dbg_hub = &hub;

			{
				Mappers::UNROM old(ConnectorType::FamicomStyle, image, sizeof(image));
				Assert::IsTrue(old.Valid());

				CartImage ci;
				ci.prg = image + 16;
				ci.prgSize = 0x20000;
				ci.chr = nullptr;
				ci.chrSize = 0;

				std::string error;
				Pcb* pcb = MakePcb(UNROM_BOARD, ci, error);
				Assert::IsTrue(pcb != nullptr);
				CartPcbCartridge nw(ConnectorType::FamicomStyle, pcb);

				// Bank writes + reads across the $8000/$C000 windows.
				// The legacy 74LS161-based implementations latch the bank on the
				// rising edge of /ROMSEL, so a write is driven as a 3-tick sequence:
				// active write, rising edge, idle.
				for (int bank = 0; bank < 8; bank++)
				{
					Step w;
					w.nROMSEL = TriState::Zero;
					w.RnW = TriState::Zero;
					w.nRD = TriState::One;
					w.nWR = TriState::Zero;
					w.cpu_data = (uint8_t)bank;
					Assert::IsTrue(ParityStep(old, nw, w));

					Step rise;
					rise.nROMSEL = TriState::One;
					rise.RnW = TriState::Zero;
					rise.nRD = TriState::One;
					rise.nWR = TriState::Zero;
					rise.cpu_data = (uint8_t)bank;
					Assert::IsTrue(ParityStep(old, nw, rise));

					Step idle;
					Assert::IsTrue(ParityStep(old, nw, idle));

					for (int off = 0; off < 16; off++)
					{
						Step r;
						r.nROMSEL = TriState::Zero;
						r.cpu_addr = (uint16_t)(off * 0x100);
						Assert::IsTrue(ParityStep(old, nw, r));

						r.cpu_addr = (uint16_t)(0x4000 + off * 0x100);	// $C000 window
						Assert::IsTrue(ParityStep(old, nw, r));
					}
				}
			}

			dbg_hub = saved;
		}

		TEST_METHOD(TestParityAorom)
		{
			uint8_t image[16 + 0x40000];
			memset(image, 0, sizeof(image));

			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 16;	// 256K PRG
			image[5] = 0;	// CHR-RAM
			image[6] = 0;
			image[7] = 7;	// mapper 7

			for (size_t i = 0; i < 0x40000; i++) image[16 + i] = (uint8_t)(i >> 15);

			DebugHub hub;
			DebugHub* saved = dbg_hub;
			dbg_hub = &hub;

			{
				Mappers::AOROM old(ConnectorType::FamicomStyle, image, sizeof(image));
				Assert::IsTrue(old.Valid());

				CartImage ci;
				ci.prg = image + 16;
				ci.prgSize = 0x40000;
				ci.chr = nullptr;
				ci.chrSize = 0;

				std::string error;
				Pcb* pcb = MakePcb(AOROM_BOARD, ci, error);
				Assert::IsTrue(pcb != nullptr);
				CartPcbCartridge nw(ConnectorType::FamicomStyle, pcb);

				for (int bank = 0; bank < 8; bank++)
				{
					Step w;
					w.nROMSEL = TriState::Zero;
					w.RnW = TriState::Zero;
					w.nRD = TriState::One;
					w.nWR = TriState::Zero;
					w.cpu_data = (uint8_t)(bank | 0x10);	// bank + mirroring bit
					Assert::IsTrue(ParityStep(old, nw, w));

					Step rise;
					rise.nROMSEL = TriState::One;
					rise.RnW = TriState::Zero;
					rise.nRD = TriState::One;
					rise.nWR = TriState::Zero;
					rise.cpu_data = (uint8_t)(bank | 0x10);
					Assert::IsTrue(ParityStep(old, nw, rise));

					Step idle;
					Assert::IsTrue(ParityStep(old, nw, idle));

					for (int off = 0; off < 16; off++)
					{
						Step r;
						r.nROMSEL = TriState::Zero;
						r.cpu_addr = (uint16_t)(off * 0x100);
						r.ppu_addr = (uint16_t)(off * 0x77);
						Assert::IsTrue(ParityStep(old, nw, r));
					}
				}
			}

			dbg_hub = saved;
		}

		/// <summary>
		/// Drive one 5-bit MMC1 serial register write (MSB first) through both
		/// cartridges. D7=1 on the first bit resets the shift register, which is
		/// how real games start a register write.
		/// </summary>
		void MMC1SerialWrite(Mappers::AbstractCartridge& a, Mappers::AbstractCartridge& b, uint8_t value, int a13, int a14)
		{
			for (int bit = 4; bit >= 0; bit--)
			{
				bool bv = (value >> bit) & 1;

				// A CPU write to the MMC1 is signaled by /ROMSEL + RnW; the PPU
				// strobes nRD/nWR stay inactive (on the real board they are the
				// PPU's own strobes and are never asserted during CPU access).
				Step s;
				s.M2 = TriState::One;
				s.nROMSEL = TriState::Zero;
				s.RnW = TriState::Zero;
				s.nRD = TriState::One;
				s.nWR = TriState::One;
				s.cpu_addr = (uint16_t)((a14 << 14) | (a13 << 13));
				s.cpu_data = (bv ? 1 : 0) | (bit == 4 ? 0x80 : 0);
				Assert::IsTrue(ParityStep(a, b, s));

				Step s2 = s;
				s2.M2 = TriState::Zero;
				Assert::IsTrue(ParityStep(a, b, s2));

				Step s3;
				s3.M2 = TriState::One;
				s3.nROMSEL = TriState::One;
				s3.RnW = TriState::One;
				Assert::IsTrue(ParityStep(a, b, s3));
			}
		}

		TEST_METHOD(TestParitySgrom)
		{
			// 256K PRG: with the MMC1 in 32K mode and high banks the PRG address
			// reaches $3C000, which exceeds a 128K PRG. The legacy implementation
			// reads past the end of the buffer in that case (a latent bug), so the
			// test uses a 256K image where both paths stay in bounds.
			uint8_t image[16 + 0x40000 + 0x2000];
			memset(image, 0, sizeof(image));

			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 16;	// 256K PRG
			image[5] = 1;	// 8K CHR
			image[6] = 0;
			image[7] = 1;	// mapper 1

			for (size_t i = 0; i < 0x40000; i++) image[16 + i] = (uint8_t)(i >> 14);
			for (size_t i = 0; i < 0x2000; i++) image[16 + 0x40000 + i] = (uint8_t)(i * 3);

			DebugHub hub;
			DebugHub* saved = dbg_hub;
			dbg_hub = &hub;

			{
				Mappers::MMC1_Based old(ConnectorType::FamicomStyle, image, sizeof(image));
				Assert::IsTrue(old.Valid());

				CartImage ci;
				ci.prg = image + 16;
				ci.prgSize = 0x40000;
				ci.chr = image + 16 + 0x40000;
				ci.chrSize = 0x2000;

				std::string error;
				Pcb* pcb = MakePcb(SGROM_BOARD, ci, error);
				Assert::IsTrue(pcb != nullptr);
				CartPcbCartridge nw(ConnectorType::FamicomStyle, pcb);

				// Reset the shift register, then write the PRG bank register (reg3).
				MMC1SerialWrite(old, nw, 0x00, 0, 0);	// reg0: control (mode 0, D4=0...)

				// reg3: PRG bank
				for (int bank = 0; bank < 8; bank++)
				{
					MMC1SerialWrite(old, nw, (uint8_t)bank, 1, 1);

					for (int off = 0; off < 8; off++)
					{
						Step r;
						r.nROMSEL = TriState::Zero;
						r.cpu_addr = (uint16_t)(off * 0x100);
						Assert::IsTrue(ParityStep(old, nw, r));

						r.cpu_addr = (uint16_t)(0x4000 + off * 0x100);
						Assert::IsTrue(ParityStep(old, nw, r));
					}
				}

				// Compare the debug PRG reads too (bank math).
				for (size_t addr = 0x8000; addr < 0x10000; addr += 0x111)
				{
					Assert::IsTrue(old.Dbg_ReadPRGByte(addr) == nw.Dbg_ReadPRGByte(addr));
				}
			}

			dbg_hub = saved;
		}
	};

	TEST_CLASS(CartPcbCustomBoardUnitTest)
	{
	public:
		/// <summary>
		/// Phase 4 (JSONES): a custom PCB that is not in the nescartdb database
		/// (e.g. an unlicensed board) is provided by the user as a JSONES document
		/// (game -> cartridge -> board) and loaded via the forced-board-type hook.
		/// </summary>
		TEST_METHOD(TestCustomJsonesBoard)
		{
			const char* customBoard =
				"{ \"schemaVersion\" : 1,"
				"  \"game\" : { \"name\" : \"Custom Unlicensed Game\", \"class\" : \"Unlicensed\","
				"    \"cartridges\" : [ { \"system\" : \"Dendy\","
				"      \"board\" : { \"type\" : \"CUSTOM-UNL-PCB\", \"mapper\" : 255,"
				"        \"components\" : {"
				"          \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\" },"
				"          \"chr\" : { \"kind\" : \"ram\", \"bus\" : \"ppu\", \"size\" : 8192 },"
				"          \"bank\" : { \"kind\" : \"latch\" } },"
				"        \"circuit\" : {"
				"          \"mirroring\" : { \"mode\" : \"hardwired\", \"h\" : 1, \"v\" : 0 },"
				"          \"cpu\" : {"
				"            \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"bank.Q2..Q0 | cpu_addr[14:0]\" },"
				"            \"bank\" : { \"clk\" : \"nROMSEL\", \"n_we\" : \"RnW\" } },"
				"          \"ppu\" : { \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"n_we\" : \"nWR\", \"addr\" : \"ppu_addr[12:0]\" } },"
				"          \"nets\" : [] } } } ] } }";

			const char* userDir = "test_user_boards";
			_mkdir(userDir);

			FILE* f = fopen((std::string(userDir) + "\\custom.json").c_str(), "wb");
			Assert::IsTrue(f != nullptr);
			fwrite(customBoard, 1, strlen(customBoard), f);
			fclose(f);

			// A .nes with mapper 255 in the header: the legacy iNES path cannot
			// run it, only the CartPcb path (forced board type) can.
			uint8_t image[16 + 0x20000];
			memset(image, 0, sizeof(image));
			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 8;	// 128K PRG
			image[5] = 0;	// CHR-RAM
			image[6] = 0;
			image[7] = 255;	// mapper 255

			for (size_t i = 0; i < 0x20000; i++) image[16 + i] = (uint8_t)(i >> 15);

			CartPcb::SetUserBoardsDir(userDir);
			CartPcb::SetForcedBoardType("CUSTOM-UNL-PCB");

			Mappers::AbstractCartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image, sizeof(image));
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());

			// Banked read: write bank 3, then read $8000+0x123.
			Bus b;
			b.SetDefaults();
			b.in[(size_t)CartInput::RnW] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.cpu_data = 3;
			cart->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			b.SetDefaults();
			b.Sim(cart, 0x0123, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == (uint8_t)((3 * 0x8000 + 0x123) >> 15));

			delete cart;

			CartPcb::SetForcedBoardType("");
			CartPcb::SetUserBoardsDir("");

			remove((std::string(userDir) + "\\custom.json").c_str());
			_rmdir(userDir);
		}
	};
}
