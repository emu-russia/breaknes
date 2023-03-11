#pragma once

namespace Breaknes
{
	class CartridgeFactory
	{
		ConnectorType p1_type = ConnectorType::None;
		uint8_t* data = nullptr;
		size_t data_size = 0;

	public:
		CartridgeFactory(ConnectorType p1, uint8_t* nesImage, size_t size);
		~CartridgeFactory();

		AbstractCartridge* GetInstance();
	};
}
