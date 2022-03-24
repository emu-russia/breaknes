#include "pch.h"

namespace Breaknes
{
	CartridgeFactory::CartridgeFactory(uint8_t* nesImage, size_t size)
	{
	}
		
	CartridgeFactory::~CartridgeFactory()
	{
	}

	AbstractCartridge* CartridgeFactory::GetInstance()
	{
		return nullptr;
	}
}
