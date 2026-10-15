// Compact, exact binary decoder domain. Timing bits are n_T0,n_T1X,n_T2..5.
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>

#ifndef M6502_PACKED_SMALL_ROW
#define M6502_PACKED_SMALL_ROW 0
#endif
#ifndef M6502_PACKED_SINGLE_COMMIT
#define M6502_PACKED_SINGLE_COMMIT 0
#endif

namespace M6502Core
{
    // Static products of decoder lines, precomputed per row into the free bits of the third row
    // word (lines occupy bits 0-129; these start at 130). Each is exactly the gate expression the
    // control evaluators formed from individual lines in v3; CpuKernelTests re-derives every one
    // from the independently simulated PLA lines for all 16,384 keys. IR0 is the opcode bit 0 of
    // the row's key (the row is always selected with the current IR).
    enum DerivedLine : unsigned
    {
        D_nSHIFT = 130,   // NOR(d106, d107)
        D_nMemOp,         // NOR5(d111, d122, d123, d124, d125)
        D_STOR,           // NOR(n_MemOp, NOT(d97))
        D_REST,           // NAND(n_SHIFT, NOT(d97))
        D_TRESX_IN,       // NOR(d91, d92)
        D_ENDX_STATIC,    // NOR3(NOR3(d96, NOT(n_SHIFT), n_MemOp), NOT(NOR6(d100..d105)), d93)
        D_WR_STATIC,      // NOR3(STOR, d98, d100)
        D_STK_ANY,        // NOT(NOR6(d21..d26))
        D_SBADD_ANY,      // OR(d30, d31, d45, d48, d47)
        D_AND,            // NOT(NOR(d69, d70))
        D_PGX_STATIC,     // NOT(NOR(d71, d72))
        D_NOADL,          // NOR7(d84, d85, d86, d87, d88, d89, d26)
        D_nSBXY,          // NAND(NOR3(d14, d15, d16), NOR3(d18, d19, d20))
        D_STXY,           // NOR(AND(STOR, d0), AND(STOR, d12))
        D_JB,             // NOR3(d94, d95, d96)
        D_INCSB_ANY,      // OR(d39, d40, d41, d42, d43)
        D_nDL_ADL,        // NOR(d81, d82)
        D_JSXY,           // NAND(NOT(d48), STXY)
        D_IND,            // NOT(NOR4(d89, AND(d90, NOT(d129)), d91, d84))
        D_IMPLIED,        // AND(AND(d128, NOT(d129)), NOT(IR0))
        D_ABS_2,          // AND(d83, NOT(d129))
        D_NAP_STATIC,     // NOR4(d84, ABS_2, d80, d93)
        D_nSB_AC,         // NOR7(d58..d64)
        D_nAC_SB,         // NOR5(AND(NOT(d64), d65), d66, d67, d68, _AND)
        D_AC_DB_IN,       // NOR(d74, AND(d79, STOR))
        D_Z_ADH17_IN,     // NOR(d57, NOT(n_DL_ADL))
        D_ZTST_STATIC,    // NOR3(n_SBXY, NOT(n_SB_AC), _AND)
        D_N2_ANY,         // OR(d45, d46, d47, d48)
        D_nPCH_DB,        // NOR(d77, d78)
        D_Max
    };
    static_assert(D_Max <= 192, "derived lines must fit the third row word");
    static_assert(!M6502_PACKED_SMALL_ROW, "the small-row layout uses the third word for descriptors; derived lines need it");

    struct PackedDecoderRow
    {
        std::array<uint64_t,3> words{};
#if !M6502_PACKED_SMALL_ROW
        uint16_t regs = 0, alu = 0, flags = 0;
#endif
        uint16_t RegDescriptor() const {
#if M6502_PACKED_SMALL_ROW
            return static_cast<uint16_t>((words[2]>>2)&511);
#else
            return regs;
#endif
        }
        uint16_t AluDescriptor() const {
#if M6502_PACKED_SMALL_ROW
            return static_cast<uint16_t>((words[2]>>11)&255);
#else
            return alu;
#endif
        }
        uint16_t FlagDescriptor() const {
#if M6502_PACKED_SMALL_ROW
            return static_cast<uint16_t>((words[2]>>19)&127);
#else
            return flags;
#endif
        }
        void SetDescriptors(uint16_t r,uint16_t a,uint16_t f) {
#if M6502_PACKED_SMALL_ROW
            // Only bits 0/1 of the third word are decoder lines 128/129.
            words[2]=(words[2]&3) | (uint64_t(r)<<2) | (uint64_t(a)<<11) | (uint64_t(f)<<19);
#else
            regs=r;alu=a;flags=f;
#endif
        }
        BaseLogic::TriState operator[](size_t n) const
        { return static_cast<BaseLogic::TriState>((words[n >> 6] >> (n & 63)) & 1); }
        bool any(uint64_t lo, uint64_t hi=0, uint64_t top=0) const
        { return (words[0]&lo) || (words[1]&hi) || (words[2]&(top&3)); }
        // Live arguments are binary producing cones (NOR/NOT or latches fed
        // by those cones). Physical four-state pad handling stays in sim_Top.
        uint8_t RegisterControls(bool ready, bool ready_latched) const;
        uint8_t AluControls(bool ready, bool t0, bool t6, bool forward, bool carry_n) const;
        uint8_t FlagControls(bool t6, bool t7) const;
    };

    // No table over contradictory opcode rails, filesystem cache or per-CPU PLA.
    class PackedDecoder
    {
    public:
        static uint32_t EncodeInput(uint8_t opcode, uint8_t timing);
        static const PackedDecoderRow& Lookup(uint8_t opcode, uint8_t timing);
    };

    template<bool Packed> class DecoderView
    {
        const BaseLogic::TriState* bytes;
        const PackedDecoderRow* row;
    public:
        DecoderView(const BaseLogic::TriState* b, const PackedDecoderRow* r) : bytes(b),row(r) {}
        BaseLogic::TriState operator[](size_t n) const
        {
            // Public core.h remains usable by the existing C++14 consumers.
            return Packed ? (*row)[n] : bytes[n];
        }
    };
}
