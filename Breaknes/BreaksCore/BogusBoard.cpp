#include "pch.h"

namespace Breaknes
{
	BogusBoard::BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) : Board (apu_rev, ppu_rev)
	{

	}

	BogusBoard::~BogusBoard()
	{

	}

	void BogusBoard::Step()
	{

	}

	void BogusBoard::Reset()
	{

	}

	bool BogusBoard::InResetState()
	{
		return false;
	}

	void BogusBoard::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{

	}

	void BogusBoard::EnableNSFBanking(bool enable)
	{

	}

	void BogusBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{

	}
}
