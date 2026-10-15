// H/V Decoder

#pragma once

namespace PPUSimFast
{
	union HDecoderInput
	{
		struct
		{
			unsigned H8 : 1;		// input 0
			unsigned n_H8 : 1;
			unsigned H7 : 1;
			unsigned n_H7 : 1;
			unsigned H6 : 1;
			unsigned n_H6 : 1;
			unsigned H5 : 1;
			unsigned n_H5 : 1;
			unsigned H4 : 1;
			unsigned n_H4 : 1;
			unsigned H3 : 1;
			unsigned n_H3 : 1;
			unsigned H2 : 1;
			unsigned n_H2 : 1;
			unsigned H1 : 1;
			unsigned n_H1 : 1;
			unsigned H0 : 1;
			unsigned n_H0 : 1;
			unsigned VB : 1;
			unsigned BLNK : 1;		// input Max
		};
		size_t packed_bits;
	};

	union VDecoderInput
	{
		struct
		{
			unsigned V8 : 1;		// input 0
			unsigned n_V8 : 1;
			unsigned V7 : 1;
			unsigned n_V7 : 1;
			unsigned V6 : 1;
			unsigned n_V6 : 1;
			unsigned V5 : 1;
			unsigned n_V5 : 1;
			unsigned V4 : 1;
			unsigned n_V4 : 1;
			unsigned V3 : 1;
			unsigned n_V3 : 1;
			unsigned V2 : 1;
			unsigned n_V2 : 1;
			unsigned V1 : 1;
			unsigned n_V1 : 1;
			unsigned V0 : 1;
			unsigned n_V0 : 1;		// input Max
		};
		size_t packed_bits;
	};

	class HVDecoder
	{
		PPU* ppu = nullptr;

		uint32_t h_events[2048]{};
		uint16_t v_events[512]{};
		BaseLogic::TriState h_legacy[24]{}, v_legacy[10]{};
		size_t vpla_outputs = 0;

	public:
		HVDecoder(PPU* parent);
		~HVDecoder();

		uint32_t sim_HDecoderPacked(BaseLogic::TriState VB, BaseLogic::TriState BLNK);
		uint16_t sim_VDecoderPacked();
		void sim_HDecoder(BaseLogic::TriState VB, BaseLogic::TriState BLNK, BaseLogic::TriState** outputs);
		void sim_VDecoder(BaseLogic::TriState** outputs);
	};
}
