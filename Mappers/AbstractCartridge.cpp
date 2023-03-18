#include "pch.h"

namespace Mappers
{
	AbstractCartridge::AbstractCartridge(ConnectorType _p1_type, uint8_t* nesImage, size_t size)
	{
		this->p1_type = _p1_type;
	}

	AbstractCartridge::~AbstractCartridge()
	{
	}

	bool AbstractCartridge::Valid()
	{
		return true;
	}
}
