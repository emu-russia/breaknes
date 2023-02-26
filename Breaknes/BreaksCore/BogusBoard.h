#pragma once

namespace Breaknes
{
	class BogusBoard : public Board
	{
	public:
		BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev);
		virtual ~BogusBoard();

		void Step() override;
		void Reset() override;
		bool InResetState() override;
		void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address) override;
		void EnableNSFBanking(bool enable) override;
		void LoadRegDump(uint8_t* data, size_t data_size) override;
	};
}
