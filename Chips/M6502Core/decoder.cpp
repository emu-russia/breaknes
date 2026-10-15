#include "pch.h"
#include <memory>

using namespace BaseLogic;

namespace M6502Core
{
	size_t* Decoder::Matrix()
	{
		static size_t bitmask[outputs_count] = {
			// A
			0b000101100000100100000,
			0b000000010110001000100,
			0b000000011010001001000,
			0b010100011001100100000,
			0b010101011010100100000,
			0b010110000001100100000,

			// B
			0b000000100010000001000,
			0b000001000000100010000,
			0b000000010101001001000,
			0b010101011001100010000,
			0b010110011001100010000,
			0b011010000001100100000,
			0b000101000000100010000,
			0b010101011010100010000,
			0b011001000000100010000,
			0b100110011001100010000,
			0b101010011001100100000,
			0b011001011010100010000,
			0b100100011001100100000,
			0b011001100000100100000,
			0b011001000001100100000,

			// C
			0b011001010101010100000,
			0b000101010101010100001,
			0b010100011001010100000,
			0b001010010101010100010,
			0b001000011001010100100,
			0b000110010101010100001,
			0b001010000000010010000,
			0b000000000000000001000,
			0b010110000000011000000,
			0b000010101001010100000,
			0b000000101001000001000,
			0b010101000000011000000,
			0b000000000100000001000,
			0b010000000000000000000,
			0b000000010001010101000,
			0b000000000001010100100,

			// D
			0b000001010101010100010,
			0b000110010101010100010,
			0b000000010101001000100,
			0b000000010110001000010,
			0b000000010110001001000,
			0b000000001010000000100,
			0b001000011001010100000,
			0b001010000000100010000,
			0b000000010101001000010,
			0b000000010110001000100,
			0b000010010101010100000,
			0b001001010101010101000,
			0b010010000001100100000,
			0b010110000000101000000,
			0b011010000000101000000,
			0b011010000000001000000,
			0b001001000000010010000,

			// E
			0b000010101001010100100,
			0b000001000000010010000,
			0b001001010101010100001,
			0b000000010001010101000,
			0b010101011010100100000,
			0b100000000000011000000,
			0b101010000000001000000,
			0b100000011001010010000,
			0b010101011001100010000,
			0b011010011001010100000,
			0b011001000000101000000,
			0b010000000000001000000,
			0b011001011001100100000,
			0b010000011001010010000,
			0b011001011001100010000,
			0b011001100001010100000,
			0b011001000000011000000,
			0b000000001010000000010,
			0b000000010110001000001,

			// F
			0b010000010110000100000,
			0b000110011001010101000,
			0b010010011001010010000,
			0b000010000000010010000,
			0b000101010101010101000,
			0b001001010101010100100,
			0b000101000000101000000,
			0b000000010110000101000,
			0b000000100100000001000,
			0b000000010100001001000,
			0b000000001000000001000,
			0b001010010101010100001,
			0b000000000000000000010,
			0b000000000000000000100,
			0b010100010101010100000,
			0b010010101001010100000,
			0b000000010101001000001,
			0b000000001000000000100,

			// G
			0b000000010110001000010,
			0b000000001010000000100,
			0b000000010110000100100,
			0b000100010101010100000,
			0b001001010101010100000,
			0b000010101001010100000,
			0b000101000000100000000,
			0b000101010101010100010,
			0b000101011001010101000,
			0b000100011001010101000,
			0b000010101001010100010,
			0b000010010101010100001,
			0b001001010101010100001,

			// H
			0b000110101001010101000,
			0b001000011001010100100,
			0b000010000000000010000,
			0b000001000000010010000,
			0b010010011010010100000,
			0b101001100001010100000,
			0b010001011010010100000,
			0b000000100110000000100,
			0b101010000000001000000,
			0b011001100001010100000,
			0b011001011001010100000,
			0b000110010101010100010,
			0b100110000000101000000,
			0b100010101001100100000,
			0b100001011001010010000,
			0b100010000101100100000,

			// K
			0b010010011010100100000,
			0b000001000000000000000,
			0b000000101001000000100,
			0b000000100101000001000,
			0b000000010100001000001,
			0b000000001010000000010,
			0b000000000000010000000,
			0b001001011010100100000,
			0b000000011000000000000,
			0b000000011001010100000,		// pp
		};

		return bitmask;
	}

	Decoder::Decoder()
	{
		pla = new PLA(inputs_count, outputs_count, (char*)"Decoder6502.bin");
		pla->SetMatrix(Matrix());
	}

	Decoder::~Decoder()
	{
		delete pla;
	}

	void Decoder::sim(size_t input_bits, TriState** outputs)
	{
		pla->sim(input_bits, outputs);
	}

	uint32_t PackedDecoder::EncodeInput(uint8_t op, uint8_t t)
	{
		uint32_t r = ((t >> 1) & 1u) | ((t & 1u) << 1) | ((t & 60u) << 15);
		const unsigned pairs[] = {5,6,2,3,4,7};
		for (unsigned n=0;n<6;++n)
		{
			const unsigned bit=(op >> pairs[n]) & 1u;
			r |= ((bit ^ 1u) | (bit << 1)) << (2+2*n);
		}
		r |= ((op & 1u) ^ 1u) << 14;
		r |= ((op & 3u) != 0 ? 1u : 0u) << 15;
		r |= (((op >> 1) & 1u) ^ 1u) << 16;
		return r;
	}

	const PackedDecoderRow& PackedDecoder::Lookup(uint8_t op, uint8_t t)
	{
		static const auto rows=[] {
			// Avoid a 512 KiB automatic object on the Windows 1 MiB stack.
			auto result=std::make_unique<std::array<PackedDecoderRow,16384>>();
			std::array<uint32_t,Decoder::outputs_count> terms{};
			for (unsigned n=0;n<terms.size();++n)
				for (unsigned bit=0;bit<Decoder::inputs_count;++bit)
					terms[n] |= ((Decoder::Matrix()[n] >> bit) & 1u) << (Decoder::inputs_count-1-bit);
			for (unsigned key=0;key<result->size();++key)
			{
				auto& r=(*result)[key];
				const uint32_t input=EncodeInput(static_cast<uint8_t>(key),static_cast<uint8_t>(key>>8));
				for (unsigned n=0;n<terms.size();++n)
					if ((input & terms[n]) == 0) r.words[n>>6] |= UINT64_C(1) << (n&63);
				const auto d=[&](unsigned n) { return r[n] == One; };
				const auto any=[&](unsigned first,unsigned last) {
					bool value=false;for(unsigned n=first;n<=last;++n) value |= d(n);return value;
				};
				const bool stor=d(97) && (d(111)||any(122,125));
				const uint16_t regs = unsigned(!(any(1,5)||(d(6)&&d(7))||(d(0)&&stor))) |
					(unsigned(!((stor&&d(12))||(d(6)&&!d(7))||any(8,11)||d(13)))<<1) |
					(unsigned(!any(14,16))<<2) | (unsigned(!any(18,20))<<3) |
					(unsigned(d(13))<<4) | (unsigned(d(48))<<5) | (unsigned(any(21,26))<<6) |
					(unsigned(d(21))<<7) | (unsigned(d(35))<<8);
				const uint16_t alu = unsigned((d(33)&&!d(34))||any(35,39)) | (unsigned(any(39,43))<<1) |
					(unsigned(d(44))<<2) | (unsigned(d(49)||d(50))<<3) | (unsigned(d(93))<<4) |
					(unsigned(d(52)||d(53))<<5) | (unsigned(d(54))<<6) | (unsigned(d(47))<<7);
				const uint16_t flags = unsigned(!(d(98)||d(99))) | (unsigned(!(d(114)||d(115)))<<1) |
					(unsigned(any(14,16)||any(18,20)||any(58,64)||d(69)||d(70))<<2) |
					(unsigned(d(75))<<3) | (unsigned(d(76))<<4) |
					(unsigned(d(112)||any(116,119))<<5) | (unsigned(d(107))<<6);
				r.SetDescriptors(regs,alu,flags);
				// Derived lines (see DerivedLine): the same gate products the evaluators formed per call.
				const auto set=[&](unsigned bit,bool value) { if (value) r.words[bit>>6] |= UINT64_C(1) << (bit&63); };
				const bool n_SHIFT=!(d(106)||d(107));
				const bool n_MemOp=!(d(111)||d(122)||d(123)||d(124)||d(125));
				const bool n_STORE=!d(97);
				const bool STOR=!(n_MemOp||n_STORE);
				const bool AND_=d(69)||d(70);
				const bool PGX_STATIC=d(71)||d(72);
				const bool n_SB_X=!(d(14)||d(15)||d(16)), n_SB_Y=!(d(18)||d(19)||d(20));
				const bool n_SBXY=!(n_SB_X&&n_SB_Y);
				const bool STXY=!((STOR&&d(0))||(STOR&&d(12)));
				const bool n_DL_ADL=!(d(81)||d(82));
				const bool pp=d(129), IR0=(key&1)!=0;
				const bool ABS_2=d(83)&&!pp;
				const bool n_SB_AC=!any(58,64);
				set(D_nSHIFT,n_SHIFT);
				set(D_nMemOp,n_MemOp);
				set(D_STOR,STOR);
				set(D_REST,!(n_SHIFT&&n_STORE));
				set(D_TRESX_IN,!(d(91)||d(92)));
				set(D_ENDX_STATIC,!( (!(d(96)||!n_SHIFT||n_MemOp)) || any(100,105) || d(93) ));
				set(D_WR_STATIC,!(STOR||d(98)||d(100)));
				set(D_STK_ANY,any(21,26));
				set(D_SBADD_ANY,d(30)||d(31)||d(45)||d(48)||d(47));
				set(D_AND,AND_);
				set(D_PGX_STATIC,PGX_STATIC);
				set(D_NOADL,!(any(84,89)||d(26)));
				set(D_nSBXY,n_SBXY);
				set(D_STXY,STXY);
				set(D_JB,!(d(94)||d(95)||d(96)));
				set(D_INCSB_ANY,any(39,43));
				set(D_nDL_ADL,n_DL_ADL);
				set(D_JSXY,!(!d(48)&&STXY));
				set(D_IND,d(89)||(d(90)&&!pp)||d(91)||d(84));
				set(D_IMPLIED,d(128)&&!pp&&!IR0);
				set(D_ABS_2,ABS_2);
				set(D_NAP_STATIC,!(d(84)||ABS_2||d(80)||d(93)));
				set(D_nSB_AC,n_SB_AC);
				set(D_nAC_SB,!((!d(64)&&d(65))||d(66)||d(67)||d(68)||AND_));
				set(D_AC_DB_IN,!(d(74)||(d(79)&&STOR)));
				set(D_Z_ADH17_IN,!(d(57)||!n_DL_ADL));
				set(D_ZTST_STATIC,!(n_SBXY||!n_SB_AC||AND_));
				set(D_N2_ANY,d(45)||d(46)||d(47)||d(48));
				set(D_nPCH_DB,!(d(77)||d(78)));
			}
			return result;
		}();
		return (*rows)[size_t(op) | ((size_t(t)&63)<<8)];
	}

	uint8_t PackedDecoderRow::RegisterControls(bool ready, bool ready_latched) const
	{
		const uint16_t regs=RegDescriptor();
		const bool stack=!ready_latched && (regs & 64);
		const bool sb_s=!((regs & 16) || ((regs & 32) && !ready) || stack);
		const bool s_adl=!(((regs & 128) && !ready_latched) || (regs & 256));
		return static_cast<uint8_t>((regs & 15) | (unsigned(sb_s)<<4) | (unsigned(s_adl)<<5));
	}

	uint8_t PackedDecoderRow::AluControls(bool ready, bool t0, bool t6, bool forward, bool carry_n) const
	{
		const uint16_t alu=AluDescriptor();
		const bool adl=!((alu & 1) || ready);
		const bool derived=!adl && (alu & 128);
		const bool inc=(alu & 2) || ((alu & 4) && t6);
		const bool branch=(alu & 8) || ((alu & 16) && !forward);
		const bool carry=((!carry_n && (t0 || t6)) && (alu & 32)) || (alu & 64);
		return static_cast<uint8_t>(unsigned(adl) | (unsigned(derived)<<1) | (unsigned(inc)<<2) |
			(unsigned(branch)<<3) | (unsigned(carry)<<4));
	}

	uint8_t PackedDecoderRow::FlagControls(bool t6, bool t7) const
	{
		const uint16_t flags=FlagDescriptor();
		const bool arit=!((flags & 32) || ((flags & 64) && t7));
		const bool z=(flags & 4) || t7;
		const bool shift=(flags & 8) || ((flags & 16) && t6);
		return static_cast<uint8_t>((flags & 1) | (unsigned(arit)<<1) | ((flags & 2)<<1) |
			(unsigned(z)<<3) | (unsigned(shift)<<4));
	}
}
