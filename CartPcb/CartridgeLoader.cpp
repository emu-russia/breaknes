// CartridgeLoader - creates a cartridge from a .nes image using the CartPcb path.

#include "CartridgeLoader.h"
#include "NesCartDb.h"
#include "PcbLoader.h"
#include "PcbFactory.h"
#include "CartPcbCartridge.h"
#include "CartImage.h"

#include "../Mappers/NES_Header.h"

namespace CartPcb
{
	namespace
	{
		std::string g_nescartdbDir = "Nescartdb";
		std::string g_userBoardsDir;
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

	Mappers::AbstractCartridge* CreateFromNesImage(
		Mappers::ConnectorType p1,
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

		// The index is loaded once per directory.
		static NesCartDb db;
		static std::string dbDir;

		if (dbDir != GetNescartdbDir())
		{
			dbDir = GetNescartdbDir();
			if (!db.Load(dbDir + "/index.json"))
			{
				return nullptr;
			}
		}

		std::vector<BoardRef> boards;
		db.FindBoards(prgCrc, chrCrc, boards);

		for (auto& ref : boards)
		{
			std::string jsonText;

			if (!PcbLoader::LoadBoard(ref.type, GetNescartdbDir(), GetUserBoardsDir(), jsonText))
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

			pcb->ApplyPadMirroring(ref.padH, ref.padV);

			return new CartPcbCartridge(p1, pcb);
		}

		return nullptr;
	}
}
