// Pcb - a simulated cartridge PCB.
//
// A board is a set of components (memory chips, latches, mapper chips) and the
// wiring that connects them to each other and to the cartridge edge connector
// (nets, bus attachments). See CartPcb/Readme.md for the specification.

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../Common/BaseLogicLib/BaseLogic.h"
#include "../Common/BaseBoardLib/RomChip.h"
#include "../Common/BaseBoardLib/SRAM.h"
#include "CartPcbPort.h"
#include "CartImage.h"

namespace CartPcb
{
	enum class Bus { None, CPU, PPU };

	// ---------------------------------------------------------------------
	// Expressions
	// ---------------------------------------------------------------------

	// Single-bit logic expression over TriState values.
	struct LogicExpr;
	using LogicExprPtr = std::shared_ptr<LogicExpr>;

	struct LogicExpr
	{
		enum class Op { Atom, Not, And, Or, Xor } op = Op::Atom;

		enum class AtomKind { Signal, Net, CpuAddrBit, PpuAddrBit, CpuDataBit, ChipPin, Gnd, Vdd, Invalid };
		AtomKind atom = AtomKind::Invalid;

		std::string name;		// Signal / Net name, or chip pin name (for ChipPin)
		std::string chip;		// chip id (for ChipPin)
		int bit = 0;			// bus bit index (for CpuAddrBit / PpuAddrBit / CpuDataBit)

		LogicExprPtr a, b;
	};

	// One bit-range term of an address expression (concatenated MSB first).
	struct AddrTerm
	{
		enum class Kind { CpuAddrRange, PpuAddrRange, CpuDataRange, ChipPinList, Gnd, Vdd, Invalid };
		Kind kind = Kind::Invalid;
		int hi = 0, lo = 0;				// bus bit range
		std::string chip;				// chip id
		std::vector<std::string> pins;	// ordered pin names (MSB first)
	};

	struct AddrExpr
	{
		std::vector<AddrTerm> terms;	// MSB first
	};
	using AddrExprPtr = std::shared_ptr<AddrExpr>;

	// ---------------------------------------------------------------------
	// Mapper chip interface (implementations live in Chips)
	// ---------------------------------------------------------------------

	struct ChipPin
	{
		std::string name;
		bool output = false;
	};

	struct ChipDesc
	{
		std::string type;
		std::vector<ChipPin> pins;
	};

	class ChipInstance
	{
	public:
		virtual ~ChipInstance() {}
		virtual void sim(BaseLogic::TriState* inputs, BaseLogic::TriState* outputs) = 0;
		virtual const ChipDesc* GetDesc() const = 0;

		/// <summary>
		/// Compute the PRG ROM address for the given CPU bus address using the
		/// current chip state, without simulating the chip (debugger support).
		/// </summary>
		virtual size_t Dbg_GetPRGAddress(size_t cpu_addr) { return 0; }
	};

	// Implemented in ChipsAdapter.cpp. Returns nullptr for unknown chip types.
	ChipInstance* CreateChipInstance(const std::string& type);

	// ---------------------------------------------------------------------
	// Pcb
	// ---------------------------------------------------------------------

	class Pcb
	{
	public:
		struct Component
		{
			std::string id;
			enum class Kind { Rom, Ram, Latch, Chip } kind = Kind::Rom;
			Bus bus = Bus::None;
			size_t size = 0;				// bytes (Rom/Ram)
			std::string image;				// "prg" / "chr" / "ram" (Rom; optional)

			BaseBoard::RomChip* rom = nullptr;		// Kind::Rom (owned)
			BaseBoard::SRAM* ram = nullptr;			// Kind::Ram (owned)

			uint8_t latch = 0;						// Kind::Latch state (Q0..Q7)
			bool latch_strobe = false;				// current strobe state (edge detection)

			std::string chipType;					// Kind::Chip
			ChipInstance* chip = nullptr;			// Kind::Chip (owned)
			std::vector<BaseLogic::TriState> chipInputs;
			std::vector<BaseLogic::TriState> chipOutputs;
			std::map<std::string, size_t> inputPinIndex;	// input pin name -> input index
			std::map<std::string, size_t> outputPinIndex;	// output pin name -> output index
		};

		struct Attachment
		{
			std::string componentId;
			bool isLatch = false;

			// Memory components
			LogicExprPtr n_cs;				// chip select (active low)
			LogicExprPtr n_oe;				// output enable (active low); null = always enabled
			LogicExprPtr n_we;				// write enable (active low); null = read-only
			AddrExprPtr addr;

			// Latch components
			LogicExprPtr clk;				// strobe (active low)
			LogicExprPtr we;				// write enable (active low)
			AddrExprPtr data;				// captured bits (default cpu_data[7:0])
		};

		struct Net
		{
			std::string name;
			LogicExprPtr from;
			BaseLogic::TriState value = BaseLogic::TriState::Z;
		};

		Pcb();
		~Pcb();

		// ---- Factory API (used by PcbFactory) ----

		Component* AddComponent(const std::string& id, Component::Kind kind, Bus bus, size_t size, const std::string& image);
		Component* AddChip(const std::string& id, const std::string& chipType);
		void AddCpuAttachment(Attachment att);
		void AddPpuAttachment(Attachment att);
		void AddNet(const Net& net);
		void SetHardwiredMirroring(bool vertical);		// true: VRAM_A10 = PA10, false: PA11
		void SetMapperMirroring(LogicExprPtr net);
		void SetBoardType(const std::string& type, const std::string& pcb);

		/// <summary>
		/// Load the PRG/CHR dumps into the ROM components (by default a ROM on
		/// the CPU bus receives the PRG image, a ROM on the PPU bus the CHR image;
		/// an explicit `image` attribute overrides this).
		/// </summary>
		void LoadImages(const CartImage& image);

		// ---- Simulation (the cartridge-port contract) ----

		void sim(
			BaseLogic::TriState cart_in[(size_t)CartInput::Max],
			BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
			uint16_t cpu_addr,
			uint8_t* cpu_data, bool& cpu_data_dirty,
			uint16_t ppu_addr,
			uint8_t* ppu_data, bool& ppu_data_dirty,
			// Famicom only
			CartAudioOutSignal* snd_out,
			// NES only
			uint16_t* exp, bool& exp_dirty);

		// ---- Debug ----

		uint8_t Dbg_ReadPRGByte(size_t cpu_addr);
		size_t Dbg_GetPRGSize();
		size_t Dbg_GetCHRSize();
		size_t Dbg_GetWRAMSize();
		uint8_t Dbg_ReadCHRByte(size_t addr);
		void Dbg_WriteCHRByte(size_t addr, uint8_t data);
		uint8_t Dbg_ReadWRAMByte(size_t addr);
		void Dbg_WriteWRAMByte(size_t addr, uint8_t data);
		uint8_t Dbg_GetLatchState(const std::string& id);

		const std::string& GetBoardType() const { return boardType; }
		const std::string& GetBoardPcb() const { return boardPcb; }

	private:
		struct SimContext
		{
			BaseLogic::TriState cart_in[(size_t)CartInput::Max]{};
			uint16_t cpu_addr = 0;
			uint16_t ppu_addr = 0;
			const uint8_t* cpu_data = nullptr;	// for data bit atoms
		};

		std::string boardType;
		std::string boardPcb;

		std::vector<std::unique_ptr<Component>> components;
		std::vector<Attachment> cpuAttachments;
		std::vector<Attachment> ppuAttachments;
		std::vector<Net> nets;

		enum class MirrorMode { None, Hardwired, Mapper };
		MirrorMode mirrorMode = MirrorMode::None;
		bool hardwiredVertical = false;
		LogicExprPtr mapperMirrorNet;

		// Evaluation helpers
		Component* FindComponent(const std::string& id);
		BaseLogic::TriState EvalLogic(const LogicExpr& e, const SimContext& ctx) const;
		BaseLogic::TriState EvalChipPin(const std::string& chipId, const std::string& pin) const;
		bool EvalAddr(const AddrExpr& e, const SimContext& ctx, size_t& outAddr, bool& ok) const;
		bool EvalAddrBits(const AddrExpr& e, const SimContext& ctx, std::vector<BaseLogic::TriState>& outBits, bool& ok) const;

		void SimulateChips(const SimContext& ctx);
		void SimulateLatches(const SimContext& ctx);
		void SimulateMemory(const SimContext& ctx,
			BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
			uint8_t* cpu_data, bool& cpu_data_dirty,
			uint8_t* ppu_data, bool& ppu_data_dirty);
	};
}
