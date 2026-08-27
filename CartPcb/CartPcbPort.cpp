// The cartridge edge-connector contract (Cartridge base implementation).

#include "CartPcbPort.h"

namespace CartPcb
{
	Cartridge::Cartridge(ConnectorType _p1_type)
	{
		p1_type = _p1_type;
	}

	Cartridge::~Cartridge()
	{
	}

	bool Cartridge::Valid()
	{
		return true;
	}
}
