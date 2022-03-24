#pragma once

namespace Breaknes
{
	class CartridgeFactory
	{
	public:
		CartridgeFactory(uint8_t* nesImage, size_t size);
		~CartridgeFactory();

		AbstractCartridge* GetInstance();
	};
}
