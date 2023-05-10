#include "pch.h"

namespace Mappers
{
	CartridgeFactory::CartridgeFactory(ConnectorType p1, uint8_t* nesImage, size_t size)
	{
		p1_type = p1;
		data = nesImage;
		data_size = size;
	}
		
	CartridgeFactory::~CartridgeFactory()
	{
	}

	AbstractCartridge* CartridgeFactory::GetInstance()
	{
		if (!data || data_size > 16 * 1024 * 1024 || data_size < sizeof(NESHeader))
		{
			printf("Doesn't look like any adequate NES ROM at all.\n");
			return nullptr;
		}

		// Basic .nes header checks

		printf("Basic .nes header checks ...");

		NESHeader* head = (NESHeader*)data;

		if (head->PRGSize > 0x10 || head->CHRSize > 0x20)
		{
			printf(" FAILED! Odd size of PRG/CHR banks! (PRGSize: %d, CHRSize: %d)\n", head->PRGSize, head->CHRSize);
			return nullptr;
		}

		bool trainer = (head->Flags_6 & 0b100) != 0;

		size_t totalSize = (trainer ? 512 : 0) + 0x4000 * head->PRGSize + 0x2000 * head->CHRSize + sizeof(NESHeader);
		if (data_size < totalSize)
		{
			printf(" FAILED! Damaged .nes (file size is smaller than required).\n");
			return nullptr;
		}

		printf(" OK!\n");

		size_t mapperNum = (head->Flags_7 & 0xf0) | (head->Flags_6 >> 4);
		printf("Mapper: %zd\n", mapperNum);

		switch (mapperNum)
		{
			case 0:
				return new Mappers::NROM(p1_type, data, data_size);

			case 2:
				return new Mappers::UNROM(p1_type, data, data_size);

			case 7:
				return new Mappers::AOROM(p1_type, data, data_size);

			default:
				break;
		}

		return nullptr;
	}
}
