// Unit tests for the CartPcb component: PcbFactory (board JSON -> Pcb),
// Pcb simulation (NROM/UNROM/AOROM behavior) and NesCartDb (CRC identification).

#include "pch.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <direct.h>

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
			"      \"mirroring\" : { \"mode\" : \"scroll\" },"
			"      \"cpu\" : { \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"cpu_addr[14:0]\" } },"
			"      \"ppu\" : { \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : [] } } }";

		// The UNROM board with its real glue logic: a 74LS161 bank register and a
		// 74LS32 address multiplexer (the old Mappers implementation wired exactly
		// this: A15 = Q1|A14, A14 = Q0|A14, A16 = A14|Q2). The $C000 window is
		// forced to the last bank because the OR with A14 saturates the bank bits.
		const char* UNROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"NES-UNROM\", \"pcb\" : \"NES-UNROM-10\", \"mapper\" : 2,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\", \"size\" : 131072 },"
			"      \"chr\" : { \"kind\" : \"rom\", \"bus\" : \"ppu\", \"size\" : 8192 },"
			"      \"ls161\" : { \"kind\" : \"chip\", \"chip\" : \"LS161\" },"
			"      \"ls32\" : { \"kind\" : \"chip\", \"chip\" : \"LS32\" } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"scroll\" },"
			"      \"cpu\" : {"
			"        \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"ls32.Y3 | ls32.Y0 | ls32.Y1 | cpu_addr[13:0]\" } },"
			"      \"ppu\" : { \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : ["
			"        { \"name\" : \"ls161.CLK\",  \"from\" : \"nROMSEL\" },"
			"        { \"name\" : \"ls161.nRST\", \"from\" : \"vdd\" },"
			"        { \"name\" : \"ls161.nLD\",  \"from\" : \"RnW\" },"
			"        { \"name\" : \"ls161.EN_T\", \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls161.EN_P\", \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls161.P0\",   \"from\" : \"cpu_data[0]\" },"
			"        { \"name\" : \"ls161.P1\",   \"from\" : \"cpu_data[1]\" },"
			"        { \"name\" : \"ls161.P2\",   \"from\" : \"cpu_data[2]\" },"
			"        { \"name\" : \"ls161.P3\",   \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls32.A0\",    \"from\" : \"ls161.Q1\" },"
			"        { \"name\" : \"ls32.B0\",    \"from\" : \"cpu_addr[14]\" },"
			"        { \"name\" : \"ls32.A1\",    \"from\" : \"ls161.Q0\" },"
			"        { \"name\" : \"ls32.B1\",    \"from\" : \"cpu_addr[14]\" },"
			"        { \"name\" : \"ls32.A2\",    \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls32.B2\",    \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls32.A3\",    \"from\" : \"cpu_addr[14]\" },"
			"        { \"name\" : \"ls32.B3\",    \"from\" : \"ls161.Q2\" } ] } } }";

		// AOROM with its real bank register: a 74LS161 whose Q3 (loaded from
		// CPU data bit 4) drives VRAM A10 (one-screen mirroring).
		const char* AOROM_BOARD =
			"{ \"schemaVersion\" : 1,"
			"  \"board\" : { \"type\" : \"NES-AOROM\", \"pcb\" : \"NES-AOROM-01\", \"mapper\" : 7,"
			"    \"components\" : {"
			"      \"prg\" : { \"kind\" : \"rom\", \"bus\" : \"cpu\", \"size\" : 262144 },"
			"      \"chr\" : { \"kind\" : \"ram\", \"bus\" : \"ppu\", \"size\" : 8192 },"
			"      \"ls161\" : { \"kind\" : \"chip\", \"chip\" : \"LS161\" } },"
			"    \"circuit\" : {"
			"      \"mirroring\" : { \"mode\" : \"mapper\", \"net\" : \"ls161.Q3\" },"
			"      \"cpu\" : {"
			"        \"prg\" : { \"chip\" : \"prg\", \"n_cs\" : \"nROMSEL\", \"addr\" : \"ls161.Q2 | ls161.Q1 | ls161.Q0 | cpu_addr[14:0]\" } },"
			"      \"ppu\" : { \"chr\" : { \"chip\" : \"chr\", \"n_cs\" : \"!nPA13\", \"n_oe\" : \"nRD\", \"n_we\" : \"nWR\", \"addr\" : \"ppu_addr[12:0]\" } },"
			"      \"nets\" : ["
			"        { \"name\" : \"ls161.CLK\",  \"from\" : \"nROMSEL\" },"
			"        { \"name\" : \"ls161.nRST\", \"from\" : \"vdd\" },"
			"        { \"name\" : \"ls161.nLD\",  \"from\" : \"RnW\" },"
			"        { \"name\" : \"ls161.EN_T\", \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls161.EN_P\", \"from\" : \"gnd\" },"
			"        { \"name\" : \"ls161.P0\",   \"from\" : \"cpu_data[0]\" },"
			"        { \"name\" : \"ls161.P1\",   \"from\" : \"cpu_data[1]\" },"
			"        { \"name\" : \"ls161.P2\",   \"from\" : \"cpu_data[2]\" },"
			"        { \"name\" : \"ls161.P3\",   \"from\" : \"cpu_data[4]\" } ] } } }";

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

			void Sim(CartPcb::Cartridge* cart, uint16_t cpu_addr, uint16_t ppu_addr)
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

		// A CPU write to $8000+ latches the bank register on the strobe
		// DEASSERTION edge (nROMSEL 0 -> 1), like the 74LS161 on the real boards.
		// Drive the write as: strobe asserted (nROMSEL=0, RnW=0) then deasserted,
		// keeping the write data on the bus through the whole cycle (the 6502
		// holds it until the next cycle, so the LS161 samples it at the edge).
		void WriteBank(Pcb* pcb, uint8_t value)
		{
			Bus b;
			b.SetDefaults();
			b.in[(size_t)CartInput::RnW] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.cpu_data = value;
			pcb->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			b.in[(size_t)CartInput::nROMSEL] = TriState::One;
			b.cpu_data = value;
			pcb->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);
		}

		void WriteBank(CartPcb::Cartridge* cart, uint8_t value)
		{
			Bus b;
			b.SetDefaults();
			b.in[(size_t)CartInput::RnW] = TriState::Zero;
			b.in[(size_t)CartInput::nRD] = TriState::One;
			b.in[(size_t)CartInput::nWR] = TriState::Zero;
			b.cpu_data = value;
			cart->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);

			b.in[(size_t)CartInput::nROMSEL] = TriState::One;
			b.cpu_data = value;
			cart->sim(b.in, b.out, 0x0000, &b.cpu_data, b.cpu_dirty, 0x2000, &b.ppu_data, b.ppu_dirty, nullptr, nullptr, b.exp_dirty);
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
			prg[0x4234] = 0x5A;	// second 16 KiB of the 32 KiB PRG
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

			// CPU read $C234: the second 16 KiB of a 32 KiB NROM-256 PRG
			// (the full 15-bit address must reach it).
			b.SetDefaults();
			b.Sim(pcb, 0xC234, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == 0x5A);

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

			// Scroll jumper (issue #525): the .nes header bit 0 = 1 ("vertical
			// mirroring" in iNES terms) selects H Scroll -> VRAM_A10 = ppu_addr[10].
			pcb->ApplyScrollFromHeader(true);

			b.SetDefaults();
			b.Sim(pcb, 0x8000, (1 << 10));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::One);

			b.SetDefaults();
			b.Sim(pcb, 0x8000, 0);
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::Zero);

			// .nes header bit 0 = 0 ("horizontal mirroring") selects V Scroll ->
			// VRAM_A10 = ppu_addr[11].
			pcb->ApplyScrollFromHeader(false);

			b.SetDefaults();
			b.Sim(pcb, 0x8000, (1 << 11));
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

			// Fill pattern: the 16 KiB bank index, so banks 3 and 7 (and the
			// $8000/$C000 windows) are distinguishable.
			for (size_t n = 0; n < sizeof(prg); n++)
			{
				prg[n] = (uint8_t)(n >> 14);
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

			// The $C000-$FFFF window is hardwired to the LAST 16 KiB bank even
			// before any bank register write (the reset vector lives there).
			b.SetDefaults();
			b.Sim(pcb, 0x4100, 0x2000);
			Assert::IsTrue(b.cpu_data == (uint8_t)((7 * 0x4000 + 0x100) >> 14));

			// Write $8000 with data 5 -> bank register = 5
			WriteBank(pcb, 5);

			// Read $8000+0x100 (cart $0100): bank 5 -> prg[5*0x4000 + 0x100]
			b.SetDefaults();
			b.Sim(pcb, 0x0100, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == (uint8_t)((5 * 0x4000 + 0x100) >> 14));

			// Read $C000+0x100 (cart $4100): still the last bank, independent of Q
			b.SetDefaults();
			b.Sim(pcb, 0x4100, 0x2000);
			Assert::IsTrue(b.cpu_data == (uint8_t)((7 * 0x4000 + 0x100) >> 14));

			// Dbg_ReadPRGByte must reflect the bank state
			Assert::IsTrue(pcb->Dbg_ReadPRGByte(0x8100) == (uint8_t)((5 * 0x4000 + 0x100) >> 14));
			Assert::IsTrue(pcb->Dbg_ReadPRGByte(0xC100) == (uint8_t)((7 * 0x4000 + 0x100) >> 14));

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
			WriteBank(pcb, 0x13);

			// Read $8000+0x123: 32K bank 3 -> prg[3*0x8000 + 0x123]
			b.SetDefaults();
			b.Sim(pcb, 0x0123, 0x2000);
			Assert::IsTrue(b.cpu_data == (uint8_t)((3 * 0x8000 + 0x123) >> 15));

			// Mirroring: VRAM_A10 = ls161.Q3 = 1 (one-screen, data bit 4)
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

	TEST_CLASS(CartPcbInesTranslatorUnitTest)
	{
	public:
		/// <summary>
		/// Issue #514: the iNES-header fallback that translates a "wild" dump
		/// (unknown PRG/CHR CRCs) into a board type.
		/// </summary>
		TEST_METHOD(TestTranslateInesHeader)
		{
			NESHeader head{};
			head.Sign[0] = 'N'; head.Sign[1] = 'E'; head.Sign[2] = 'S'; head.Sign[3] = 0x1A;

			BoardRef ref;

			// Mapper 0 (NROM): the PRG size picks 128 (16 KiB) vs 256 (32 KiB).
			head.Flags_6 = 0; head.Flags_7 = 0;
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x4000, 0x2000, ref));
			Assert::IsTrue(ref.type == "NES-NROM-128");
			Assert::IsTrue(ref.mapper == 0);
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x8000, 0x2000, ref));
			Assert::IsTrue(ref.type == "NES-NROM-256");

			// Mapper 1 (MMC1): CHR-ROM -> SGROM wiring, CHR-RAM -> SHROM wiring.
			head.Flags_6 = 0x10; head.Flags_7 = 0;
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x8000, 0x2000, ref));
			Assert::IsTrue(ref.type == "HVC-SGROM");
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x8000, 0, ref));
			Assert::IsTrue(ref.type == "HVC-SHROM");

			// Mapper 2 (UNROM).
			head.Flags_6 = 0x20; head.Flags_7 = 0;
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x40000, 0, ref));
			Assert::IsTrue(ref.type == "NES-UNROM");

			// Mapper 3 (CNROM).
			head.Flags_6 = 0x30; head.Flags_7 = 0;
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x8000, 0x8000, ref));
			Assert::IsTrue(ref.type == "NES-CNROM");

			// Mapper 7 (AOROM).
			head.Flags_6 = 0x70; head.Flags_7 = 0;
			Assert::IsTrue(TryTranslateInesHeader(&head, 0x80000, 0, ref));
			Assert::IsTrue(ref.type == "NES-AOROM");

			// Mapper 4 (MMC3) is not translated yet (no MMC3 chip simulation).
			head.Flags_6 = 0x40; head.Flags_7 = 0;
			Assert::IsTrue(!TryTranslateInesHeader(&head, 0x40000, 0x20000, ref));

			// Mapper 255 (upper nibble of Flags_7) is unknown.
			head.Flags_6 = 0xF0; head.Flags_7 = 0xF0;
			Assert::IsTrue(!TryTranslateInesHeader(&head, 0x40000, 0, ref));

			// A null header must not crash.
			Assert::IsTrue(!TryTranslateInesHeader(nullptr, 0x4000, 0x2000, ref));
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

			CartPcb::Cartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image, sizeof(image));
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());

			// Banked read: write bank 3, then read $8000+0x123.
			WriteBank(cart, 3);

			Bus b;
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

	TEST_CLASS(CartPcbNesCartDbIntegrationUnitTest)
	{
	public:
		/// <summary>
		/// Issue #527: the NAMCOT-3305 board (The Tower of Druaga, Pac-Land) is
		/// electrically an NROM-256 (2x16K PRG + 8K CHR, no mapper logic) with the
		/// H-scroll jumper hardwired (iNES "vertical mirroring"). It must resolve
		/// to nrom.json through the family index, otherwise the carts fail to load.
		/// </summary>
		TEST_METHOD(TestNamcot3305Board)
		{
			// Locate the committed Nescartdb data (same probing as above).
			std::string dbDir;
			for (auto& candidate : { "Nescartdb", "../../../Nescartdb", "../../Nescartdb" })
			{
				NesCartDb probe;
				if (probe.Load(std::string(candidate) + "/index.json"))
				{
					dbDir = candidate;
					break;
				}
			}
			Assert::IsTrue(!dbDir.empty());

			// 1. The Tower of Druaga (PRG AF6E8571, CHR 6DC7E8EA) is identified
			// as NAMCOT-3305.
			NesCartDb db;
			Assert::IsTrue(db.Load(dbDir + "/index.json"));

			std::vector<BoardRef> out;
			db.FindBoards(0xAF6E8571u, 0x6DC7E8EAu, out);
			bool foundNamcot = false;
			for (auto& r : out)
			{
				if (r.type == "NAMCOT-3305")
					foundNamcot = true;
			}
			Assert::IsTrue(foundNamcot);

			// 2. The NAMCOT-3305 family mapping must resolve to the built-in
			// NROM board definition.
			std::string jsonText;
			Assert::IsTrue(PcbLoader::LoadBoard("NAMCOT-3305", dbDir, "", jsonText));
			Assert::IsTrue(!jsonText.empty());

			// 3. End-to-end on the real ROM (if present): the full .nes ->
			// identification -> nrom.json -> Pcb -> cartridge flow. The scroll
			// jumper must follow the .nes header (Flags6 bit 0 = 1, iNES
			// "vertical mirroring" -> H Scroll, VRAM_A10 = PA10) and the boot
			// sentinel "YAMAMO" must be readable at $8044-$8049 (issue #527).
			const char* path = "C:\\Roms\\NES\\Druaga no Tou (Japan).nes";

			FILE* f = fopen(path, "rb");
			if (f == nullptr)
			{
				Logger::WriteMessage("Druaga no Tou (Japan).nes not found, skipping end-to-end part");
				return;
			}

			fseek(f, 0, SEEK_END);
			long size = ftell(f);
			fseek(f, 0, SEEK_SET);

			std::vector<uint8_t> image((size_t)size);
			fread(image.data(), 1, (size_t)size, f);
			fclose(f);

			std::string savedDir = CartPcb::GetNescartdbDir();
			CartPcb::SetNescartdbDir(dbDir.c_str());

			CartPcb::Cartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image.data(), image.size());
			Logger::WriteMessage(("Druaga no Tou cart=" + std::string(cart ? "OK" : "NULL")).c_str());
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());

			// The sentinel the boot code compares RAM $0002-$0007 against.
			char sentinel[7] = {};
			for (int i = 0; i < 6; i++)
				sentinel[i] = (char)cart->Dbg_ReadPRGByte(0x8044 + i);
			Logger::WriteMessage(("Druaga no Tou sentinel @ $8044 = \"" + std::string(sentinel) + "\"").c_str());
			Assert::IsTrue(memcmp(sentinel, "YAMAMO", 6) == 0);

			// The scroll jumper: H Scroll (VRAM_A10 = PA10).
			Bus b;
			b.SetDefaults();
			b.Sim(cart, 0x8000, (1 << 10));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::One);

			b.SetDefaults();
			b.Sim(cart, 0x8000, (1 << 11));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::Zero);

			delete cart;
			CartPcb::SetNescartdbDir(savedDir.c_str());
		}

		/// <summary>
		/// Phase 6: the real converted Nescartdb data (committed in the repo) must
		/// load and identify dumps, and the real board definitions must load from
		/// disk through the family index (boards/index.json).
		/// </summary>
		TEST_METHOD(TestRealNescartdbData)
		{
			// Locate the committed Nescartdb data: it depends on the test runner's
			// working directory, so probe a couple of candidate paths.
			std::string dbDir;

			for (auto& candidate : { "Nescartdb", "../../../Nescartdb", "../../Nescartdb" })
			{
				NesCartDb probe;
				if (probe.Load(std::string(candidate) + "/index.json"))
				{
					dbDir = candidate;
					break;
				}
			}
			Assert::IsTrue(!dbDir.empty());

			// 1. The real index loads and identifies a known cartridge (10-Yard Fight).
			NesCartDb db;
			Assert::IsTrue(db.Load(dbDir + "/index.json"));
			Assert::IsTrue(db.IsLoaded());

			std::vector<BoardRef> out;
			db.FindBoards(0xD3D248C9u, 0x9C124A53u, out);
			Assert::IsTrue(out.size() >= 1);
			Assert::IsTrue(out[0].type == "IREM-NROM-128");

			// 2. A PRG-only record (CHR-RAM board) matches any CHR.
			db.FindBoards(0x12C6D5C7u, 0x12345678u, out);	// NES-UNROM (1943) PRG CRC
			bool foundUnrom = false;
			for (auto& r : out)
			{
				if (r.type == "NES-UNROM")
					foundUnrom = true;
			}
			Assert::IsTrue(foundUnrom);

			// 3. A board definition loads from disk through the family index and
			// builds a working cartridge (forced type bypasses identification).
			uint8_t image[16 + 0x8000 + 0x2000];
			memset(image, 0, sizeof(image));
			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 2;
			image[5] = 1;
			image[6] = 1;
			image[7] = 0;
			image[16 + 0x1234] = 0xAB;

			std::string savedDir = CartPcb::GetNescartdbDir();
			std::string savedForced = CartPcb::GetForcedBoardType();

			CartPcb::SetNescartdbDir(dbDir.c_str());
			CartPcb::SetForcedBoardType("NES-NROM-256");

			CartPcb::Cartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image, sizeof(image));
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());
			Assert::IsTrue(cart->Dbg_ReadPRGByte(0x9234) == 0xAB);

			delete cart;

			CartPcb::SetForcedBoardType(savedForced.c_str());
			CartPcb::SetNescartdbDir(savedDir.c_str());
		}

		/// <summary>
		/// End-to-end on a real ROM (if present): the full .nes -> identification ->
		/// board JSON (from disk) -> Pcb -> cartridge flow, including the mirroring
		/// source (.nes header for hardwired boards).
		/// </summary>
		TEST_METHOD(TestBomberNesEndToEnd)
		{
			const char* path = "C:\\work\\bomberman-nes\\bomber.nes";

			FILE* f = fopen(path, "rb");
			if (f == nullptr)
			{
				Logger::WriteMessage("bomber.nes not found, skipping");
				return;
			}

			fseek(f, 0, SEEK_END);
			long size = ftell(f);
			fseek(f, 0, SEEK_SET);

			std::vector<uint8_t> image((size_t)size);
			fread(image.data(), 1, (size_t)size, f);
			fclose(f);

			std::string dbDir;
			for (auto& candidate : { "Nescartdb", "../../../Nescartdb", "../../Nescartdb" })
			{
				NesCartDb probe;
				if (probe.Load(std::string(candidate) + "/index.json"))
				{
					dbDir = candidate;
					break;
				}
			}
			Assert::IsTrue(!dbDir.empty());

			std::string savedDir = CartPcb::GetNescartdbDir();
			CartPcb::SetNescartdbDir(dbDir.c_str());

			CartPcb::Cartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image.data(), image.size());
			Logger::WriteMessage(("bomber.nes cart=" + std::string(cart ? "OK" : "NULL")).c_str());
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());

			// The scroll jumper must follow the .nes header (Flags6 bit 0 = 1,
			// iNES "vertical mirroring" -> H Scroll, VRAM_A10 = PA10), not the
			// nescartdb pad.
			Bus b;
			b.SetDefaults();
			b.Sim(cart, 0x8000, (1 << 10));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::One);

			b.SetDefaults();
			b.Sim(cart, 0x8000, (1 << 11));
			Assert::IsTrue(b.out[(size_t)CartOutput::VRAM_A10] == TriState::Zero);

			// The managed app passes the *parent* of the Nescartdb folder; the
			// loader must fall back to <dir>/Nescartdb/index.json.
			std::string parentDir = dbDir;
			size_t pos = parentDir.rfind("Nescartdb");
			if (pos != std::string::npos)
			{
				parentDir = parentDir.substr(0, pos);
			}
			if (parentDir.empty())
			{
				parentDir = ".";
			}

			CartPcb::SetNescartdbDir(parentDir.c_str());
			CartPcb::Cartridge* cart2 = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image.data(), image.size());
			Logger::WriteMessage(("bomber.nes cart (parent-dir fallback)=" + std::string(cart2 ? "OK" : "NULL")).c_str());
			Assert::IsTrue(cart2 != nullptr);

			delete cart2;
			delete cart;
			CartPcb::SetNescartdbDir(savedDir.c_str());
		}

		/// <summary>
		/// Issue #514: a "wild" dump whose PRG/CHR CRCs are not in nescartdb must
		/// still load through the iNES-header fallback (mapper -> board type) and
		/// run on the same CartPcb board path.
		/// </summary>
		TEST_METHOD(TestWildDumpFallback)
		{
			// Locate the committed Nescartdb data (same probing as above).
			std::string dbDir;
			for (auto& candidate : { "Nescartdb", "../../../Nescartdb", "../../Nescartdb" })
			{
				NesCartDb probe;
				if (probe.Load(std::string(candidate) + "/index.json"))
				{
					dbDir = candidate;
					break;
				}
			}
			Assert::IsTrue(!dbDir.empty());

			// A homebrew-style UNROM dump: 128K PRG, CHR-RAM, mapper 2. The PRG
			// content is a synthetic pattern that is (with overwhelming
			// probability) not in nescartdb.
			uint8_t image[16 + 0x40000];
			memset(image, 0, sizeof(image));
			image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
			image[4] = 8;			// 128K PRG
			image[5] = 0;			// CHR-RAM
			image[6] = 0x20;		// mapper 2 (UNROM)
			image[7] = 0;
			for (size_t i = 0; i < 0x40000; i++)
			{
				image[16 + i] = (uint8_t)((i * 7 + 3) & 0xFF);	// synthetic pattern
			}

			NesCartDb db;
			Assert::IsTrue(db.Load(dbDir + "/index.json"));

			// Sanity: the synthetic PRG CRC is really unknown to nescartdb.
			uint32_t prgCrc = NesCartDb::Crc32(image + 16, 0x40000);
			std::vector<BoardRef> out;
			db.FindBoards(prgCrc, 0, out);
			Assert::IsTrue(out.empty());

			std::string savedDir = CartPcb::GetNescartdbDir();
			std::string savedForced = CartPcb::GetForcedBoardType();

			CartPcb::SetForcedBoardType("");			// no JSONES override
			CartPcb::SetNescartdbDir(dbDir.c_str());

			CartPcb::Cartridge* cart = CartPcb::CreateFromNesImage(ConnectorType::FamicomStyle, image, sizeof(image));
			Assert::IsTrue(cart != nullptr);
			Assert::IsTrue(cart->Valid());

			// The UNROM board from the fallback must bank-switch correctly
			// (write bank 5, then read $8000+0x100).
			WriteBank(cart, 5);

			Bus b;
			b.SetDefaults();
			b.Sim(cart, 0x0100, 0x2000);
			Assert::IsTrue(b.cpu_dirty == true);
			Assert::IsTrue(b.cpu_data == (uint8_t)(image[16 + 5 * 0x4000 + 0x100]));

			delete cart;

			CartPcb::SetForcedBoardType(savedForced.c_str());
			CartPcb::SetNescartdbDir(savedDir.c_str());
		}
	};
}
