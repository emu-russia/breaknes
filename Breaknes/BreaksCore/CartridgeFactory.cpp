#include "pch.h"

namespace Breaknes
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
		return new Mappers::NROM(p1_type, data, data_size);
	}
}
