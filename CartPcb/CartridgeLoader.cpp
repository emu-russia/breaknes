// CartridgeLoader - creates a cartridge from a .nes image using the CartPcb path.

#include "CartridgeLoader.h"
#include "NesCartDb.h"
#include "PcbLoader.h"
#include "PcbFactory.h"
#include "CartPcbCartridge.h"
#include "CartImage.h"
#include "InesTranslator.h"

#include "NesHeader.h"

namespace CartPcb
{
	namespace
	{
		std::string g_nescartdbDir = "Nescartdb";
		std::string g_userBoardsDir;
		std::string g_forcedBoardType;

		bool FileExists(const std::string& path)
		{
			FILE* f = fopen(path.c_str(), "rb");
			if (f != nullptr)
			{
				fclose(f);
				return true;
			}
			return false;
		}

		// Accept both a path to the Nescartdb folder itself and a path to its
		// parent (the application directory containing Nescartdb/).
		std::string ResolveNescartdbDir(const std::string& dir)
		{
			if (FileExists(dir + "/index.json"))
			{
				return dir;
			}
			if (FileExists(dir + "/Nescartdb/index.json"))
			{
				return dir + "/Nescartdb";
			}
			return dir;
		}
	}

	void SetNescartdbDir(const char* dir)
	{
		if (dir != nullptr)
		{
			g_nescartdbDir = dir;
		}
	}

	const std::string& GetNescartdbDir()
	{
		return g_nescartdbDir;
	}

	void SetUserBoardsDir(const char* dir)
	{
		if (dir != nullptr)
		{
			g_userBoardsDir = dir;
		}
	}

	const std::string& GetUserBoardsDir()
	{
		return g_userBoardsDir;
	}

	void SetForcedBoardType(const char* type)
	{
		g_forcedBoardType = type != nullptr ? type : "";
	}

	const std::string& GetForcedBoardType()
	{
		return g_forcedBoardType;
	}

	Cartridge* CreateFromNesImage(
		ConnectorType p1,
		uint8_t* nesImage, size_t nesImageSize)
	{
		if (nesImage == nullptr || nesImageSize < sizeof(NESHeader))
		{
			return nullptr;
		}

		NESHeader* head = (NESHeader*)nesImage;

		if (head->PRGSize > 0x10 || head->CHRSize > 0x20)
		{
			return nullptr;
		}

		bool trainer = (head->Flags_6 & 0b100) != 0;

		size_t prgOffset = sizeof(NESHeader) + (trainer ? NES_TRAINER_SIZE : 0);
		uint8_t* prg = nesImage + prgOffset;
		size_t prgSize = (size_t)head->PRGSize * 0x4000;

		uint8_t* chr = prg + prgSize;
		size_t chrSize = (size_t)head->CHRSize * 0x2000;

		uint32_t prgCrc = NesCartDb::Crc32(prg, prgSize);
		uint32_t chrCrc = chrSize != 0 ? NesCartDb::Crc32(chr, chrSize) : 0;

		std::vector<BoardRef> boards;

		if (g_forcedBoardType.empty())
		{
			// The index is loaded once per directory.
			static NesCartDb db;
			static std::string dbDir;

			if (dbDir != GetNescartdbDir())
			{
				dbDir = GetNescartdbDir();

				if (!db.Load(ResolveNescartdbDir(dbDir) + "/index.json"))
				{
					return nullptr;
				}
			}

			db.FindBoards(prgCrc, chrCrc, boards);

			if (boards.empty())
			{
				// "Wild" dump fallback (issue #514): the PRG/CHR CRCs are not in
				// nescartdb (homebrew, undocumented dumps), so infer the board
				// type from the iNES header (mapper number + sizes). The dump
				// then runs through the same CartPcb board path as any other.
				BoardRef ref;
				if (TryTranslateInesHeader(head, prgSize, chrSize, ref))
				{
					LOG_CART(Cat_Events, "PRG/CHR CRC not in nescartdb; iNES fallback: mapper %d -> %s",
						ref.mapper, ref.type.c_str());
					boards.push_back(ref);
				}
			}
		}
		else
		{
			// JSONES hook: a forced board type skips the identification.
			BoardRef ref;
			ref.type = g_forcedBoardType;
			boards.push_back(ref);
		}

		for (auto& ref : boards)
		{
			std::string jsonText;

			if (!PcbLoader::LoadBoard(ref.type, ResolveNescartdbDir(GetNescartdbDir()), GetUserBoardsDir(), jsonText))
			{
				continue;
			}

			CartImage image;
			image.prg = prg;
			image.prgSize = prgSize;
			image.chr = chr;
			image.chrSize = chrSize;

			std::string error;
			Pcb* pcb = PcbFactory::Create(jsonText, image, error);

			if (pcb == nullptr)
			{
				continue;
			}

			// The scroll jumper of scroll-jumper boards follows the .nes header
			// (iNES Flags6 bit 0), exactly as the pre-migration implementation did:
			// bit 0 = 1 (iNES "vertical mirroring") means the board's jumper is set
			// to H Scroll (VRAM_A10 = PA10). The nescartdb solder pads are kept in
			// the index but are not used to override the header, to avoid regressions
			// on dumps whose header and database disagree (e.g. some UNROM games).
			pcb->ApplyScrollFromHeader((head->Flags_6 & 1) != 0);

			return new CartPcbCartridge(p1, pcb);
		}

		return nullptr;
	}
}
