// Pcb - a simulated cartridge PCB.

#include "Pcb.h"
#include "ChipsAdapter.h"

#include <cstring>

using namespace BaseLogic;

namespace CartPcb
{
	Pcb::Pcb()
	{
	}

	Pcb::~Pcb()
	{
	}

	Pcb::Component* Pcb::AddComponent(const std::string& id, Component::Kind kind, Bus bus, size_t size, const std::string& image)
	{
		Component* c = new Component;
		c->id = id;
		c->kind = kind;
		c->bus = bus;
		c->size = size;
		c->image = image;

		switch (kind)
		{
			case Component::Kind::Rom:
			{
				// 2^bits bytes
				size_t bits = 0;
				size_t sz = size;
				while ((1ULL << bits) < sz)
				{
					bits++;
				}
				c->rom = new BaseBoard::RomChip(id.c_str(), bits);
				break;
			}
			case Component::Kind::Ram:
			{
				size_t bits = 0;
				size_t sz = size;
				while ((1ULL << bits) < sz)
				{
					bits++;
				}
				c->ram = new BaseBoard::SRAM(id.c_str(), bits);
				break;
			}
			default:
				break;
		}

		components.push_back(std::unique_ptr<Component>(c));
		return c;
	}

	Pcb::Component* Pcb::AddChip(const std::string& id, const std::string& chipType)
	{
		Component* c = new Component;
		c->id = id;
		c->kind = Component::Kind::Chip;
		c->chipType = chipType;

		c->chip = CreateChipInstance(chipType);

		if (c->chip != nullptr)
		{
			const ChipDesc* desc = c->chip->GetDesc();

			size_t inIdx = 0;
			size_t outIdx = 0;

			for (auto& pin : desc->pins)
			{
				if (pin.output)
				{
					c->outputPinIndex[pin.name] = outIdx++;
				}
				else
				{
					c->inputPinIndex[pin.name] = inIdx++;
				}
			}

			c->chipInputs.resize(inIdx);
			c->chipOutputs.resize(outIdx);
		}

		components.push_back(std::unique_ptr<Component>(c));
		return c;
	}

	void Pcb::AddCpuAttachment(Attachment att)
	{
		cpuAttachments.push_back(att);
	}

	void Pcb::AddPpuAttachment(Attachment att)
	{
		ppuAttachments.push_back(att);
	}

	void Pcb::AddNet(const Net& net)
	{
		nets.push_back(net);
	}

	void Pcb::SetHardwiredMirroring(bool vertical)
	{
		mirrorMode = MirrorMode::Hardwired;
		hardwiredVertical = vertical;
	}

	void Pcb::SetMapperMirroring(LogicExprPtr net)
	{
		mirrorMode = MirrorMode::Mapper;
		mapperMirrorNet = net;
	}

	void Pcb::ApplyPadMirroring(int padH, int padV)
	{
		if (mirrorMode != MirrorMode::Hardwired)
			return;

		// v=1: vertical (VRAM_A10 = PA10), h=1: horizontal (VRAM_A10 = PA11)
		if (padV == 1)
		{
			hardwiredVertical = true;
		}
		else if (padH == 1)
		{
			hardwiredVertical = false;
		}
	}

	void Pcb::SetBoardType(const std::string& type, const std::string& pcb)
	{
		boardType = type;
		boardPcb = pcb;
	}

	void Pcb::LoadImages(const CartImage& image)
	{
		for (auto& c : components)
		{
			if (c->kind != Component::Kind::Rom || c->rom == nullptr)
				continue;

			const uint8_t* src = nullptr;
			size_t srcSize = 0;

			if (!c->image.empty())
			{
				if (c->image == "prg") { src = image.prg; srcSize = image.prgSize; }
				else if (c->image == "chr") { src = image.chr; srcSize = image.chrSize; }
				else if (c->image == "ram") { src = image.ram; srcSize = image.ramSize; }
			}
			else
			{
				if (c->bus == Bus::CPU) { src = image.prg; srcSize = image.prgSize; }
				else if (c->bus == Bus::PPU) { src = image.chr; srcSize = image.chrSize; }
			}

			if (src != nullptr && srcSize != 0)
			{
				c->rom->LoadImage(src, srcSize);
			}
		}
	}

	Pcb::Component* Pcb::FindComponent(const std::string& id)
	{
		for (auto& c : components)
		{
			if (c->id == id)
			{
				return c.get();
			}
		}
		return nullptr;
	}

	TriState Pcb::EvalChipPin(const std::string& chipId, const std::string& pin) const
	{
		for (auto& c : components)
		{
			if (c->id != chipId)
				continue;

			if (c->kind == Component::Kind::Latch)
			{
				// Q0..Q7
				if (pin.size() == 2 && pin[0] == 'Q' && pin[1] >= '0' && pin[1] <= '7')
				{
					int bit = pin[1] - '0';
					return FromByte((c->latch >> bit) & 1);
				}
				return TriState::Z;
			}

			if (c->kind == Component::Kind::Chip && c->chip != nullptr)
			{
				auto it = c->outputPinIndex.find(pin);
				if (it != c->outputPinIndex.end())
				{
					return c->chipOutputs[it->second];
				}
				return TriState::Z;
			}

			return TriState::Z;
		}

		return TriState::Z;
	}

	TriState Pcb::EvalLogic(const LogicExpr& e, const SimContext& ctx) const
	{
		switch (e.op)
		{
			case LogicExpr::Op::Not:
			{
				if (!e.a) return TriState::Z;
				return NOT(EvalLogic(*e.a, ctx));
			}
			case LogicExpr::Op::And:
			{
				if (!e.a || !e.b) return TriState::Z;
				return AND(EvalLogic(*e.a, ctx), EvalLogic(*e.b, ctx));
			}
			case LogicExpr::Op::Or:
			{
				if (!e.a || !e.b) return TriState::Z;
				return OR(EvalLogic(*e.a, ctx), EvalLogic(*e.b, ctx));
			}
			case LogicExpr::Op::Xor:
			{
				if (!e.a || !e.b) return TriState::Z;
				return XOR(EvalLogic(*e.a, ctx), EvalLogic(*e.b, ctx));
			}
			case LogicExpr::Op::Atom:
				break;
		}

		switch (e.atom)
		{
			case LogicExpr::AtomKind::Signal:
			{
				if (e.name == "gnd") return TriState::Zero;
				if (e.name == "vdd") return TriState::One;

				if (e.name == "SYSTEM_CLK") return ctx.cart_in[(size_t)CartInput::SYSTEM_CLK];
				if (e.name == "M2") return ctx.cart_in[(size_t)CartInput::M2];
				if (e.name == "nROMSEL") return ctx.cart_in[(size_t)CartInput::nROMSEL];
				if (e.name == "RnW") return ctx.cart_in[(size_t)CartInput::RnW];
				if (e.name == "nRD") return ctx.cart_in[(size_t)CartInput::nRD];
				if (e.name == "nWR") return ctx.cart_in[(size_t)CartInput::nWR];
				if (e.name == "nPA13") return ctx.cart_in[(size_t)CartInput::nPA13];
				if (e.name == "CIC_CLK") return ctx.cart_in[(size_t)CartInput::CIC_CLK];
				if (e.name == "CIC_TO_CART") return ctx.cart_in[(size_t)CartInput::CIC_TO_CART];

				return TriState::Z;
			}
			case LogicExpr::AtomKind::Net:
			{
				for (auto& net : nets)
				{
					if (net.name == e.name)
					{
						return net.value;
					}
				}
				return TriState::Z;
			}
			case LogicExpr::AtomKind::CpuAddrBit:
				return FromByte((ctx.cpu_addr >> e.bit) & 1);
			case LogicExpr::AtomKind::PpuAddrBit:
				return FromByte((ctx.ppu_addr >> e.bit) & 1);
			case LogicExpr::AtomKind::CpuDataBit:
				if (ctx.cpu_data != nullptr)
					return FromByte((*ctx.cpu_data >> e.bit) & 1);
				return TriState::Z;
			case LogicExpr::AtomKind::ChipPin:
				return EvalChipPin(e.chip, e.name);
			case LogicExpr::AtomKind::Gnd:
				return TriState::Zero;
			case LogicExpr::AtomKind::Vdd:
				return TriState::One;
			default:
				return TriState::Z;
		}
	}

	bool Pcb::EvalAddrBits(const AddrExpr& e, const SimContext& ctx, std::vector<TriState>& outBits, bool& ok) const
	{
		ok = true;

		for (auto& term : e.terms)
		{
			switch (term.kind)
			{
				case AddrTerm::Kind::CpuAddrRange:
					for (int bit = term.hi; bit >= term.lo; bit--)
						outBits.push_back(FromByte((ctx.cpu_addr >> bit) & 1));
					break;
				case AddrTerm::Kind::PpuAddrRange:
					for (int bit = term.hi; bit >= term.lo; bit--)
						outBits.push_back(FromByte((ctx.ppu_addr >> bit) & 1));
					break;
				case AddrTerm::Kind::CpuDataRange:
					if (ctx.cpu_data != nullptr)
					{
						for (int bit = term.hi; bit >= term.lo; bit--)
							outBits.push_back(FromByte((*ctx.cpu_data >> bit) & 1));
					}
					else
					{
						ok = false;
					}
					break;
				case AddrTerm::Kind::ChipPinList:
					for (auto& pin : term.pins)
						outBits.push_back(EvalChipPin(term.chip, pin));
					break;
				case AddrTerm::Kind::Gnd:
					outBits.push_back(TriState::Zero);
					break;
				case AddrTerm::Kind::Vdd:
					outBits.push_back(TriState::One);
					break;
				default:
					ok = false;
					break;
			}
		}

		return true;
	}

	bool Pcb::EvalAddr(const AddrExpr& e, const SimContext& ctx, size_t& outAddr, bool& ok) const
	{
		std::vector<TriState> bits;
		EvalAddrBits(e, ctx, bits, ok);

		if (!ok)
			return false;

		outAddr = 0;

		for (auto& bit : bits)
		{
			if (bit == TriState::Z)
			{
				ok = false;
				return false;
			}
			outAddr = (outAddr << 1) | (size_t)ToByte(bit);
		}

		return true;
	}

	void Pcb::SimulateChips(const SimContext& ctx)
	{
		for (auto& c : components)
		{
			if (c->kind != Component::Kind::Chip || c->chip == nullptr)
				continue;

			// Gather the inputs from the nets named "<chip>.<PIN>".
			std::string prefix = c->id + ".";

			for (auto& net : nets)
			{
				if (net.name.rfind(prefix, 0) != 0)
					continue;

				std::string pin = net.name.substr(prefix.size());
				auto it = c->inputPinIndex.find(pin);
				if (it != c->inputPinIndex.end())
				{
					c->chipInputs[it->second] = net.value;
				}
			}

			c->chip->sim(c->chipInputs.data(), c->chipOutputs.data());
		}
	}

	void Pcb::SimulateLatches(const SimContext& ctx)
	{
		for (auto& c : components)
		{
			if (c->kind != Component::Kind::Latch)
				continue;

			const Attachment* att = nullptr;

			for (auto& a : cpuAttachments)
			{
				if (a.isLatch && a.componentId == c->id)
				{
					att = &a;
					break;
				}
			}

			if (att == nullptr || !att->clk || !att->we)
				continue;

			TriState clk = EvalLogic(*att->clk, ctx);
			TriState we = EvalLogic(*att->we, ctx);

			bool active = (clk == TriState::Zero && we == TriState::Zero);

			if (active && !c->latch_strobe)
			{
				// Capture the data bits on the edge into the strobe state.
				AddrExprPtr data = att->data;
				if (!data)
				{
					// Default: the full data byte
					AddrExpr def;
					AddrTerm t;
					t.kind = AddrTerm::Kind::CpuDataRange;
					t.hi = 7;
					t.lo = 0;
					def.terms.push_back(t);
					data = std::make_shared<AddrExpr>(def);
				}

				std::vector<TriState> bits;
				bool ok = false;
				EvalAddrBits(*data, ctx, bits, ok);

				if (ok)
				{
					uint8_t val = 0;
					bool valid = true;
					for (auto& bit : bits)
					{
						if (bit == TriState::Z)
						{
							valid = false;
							break;
						}
						val = (uint8_t)((val << 1) | ToByte(bit));
					}
					if (valid)
					{
						c->latch = val;
					}
				}
			}

			c->latch_strobe = active;
		}
	}

	void Pcb::SimulateMemory(const SimContext& ctx,
		TriState cart_out[(size_t)CartOutput::Max],
		uint8_t* cpu_data, bool& cpu_data_dirty,
		uint8_t* ppu_data, bool& ppu_data_dirty)
	{
		// CPU bus

		for (auto& att : cpuAttachments)
		{
			if (att.isLatch)
				continue;

			Component* c = FindComponent(att.componentId);
			if (c == nullptr)
				continue;

			TriState n_cs = EvalLogic(*att.n_cs, ctx);
			if (n_cs != TriState::Zero)
				continue;

			TriState n_oe = att.n_oe ? EvalLogic(*att.n_oe, ctx) : TriState::Zero;
			TriState n_we = att.n_we ? EvalLogic(*att.n_we, ctx) : TriState::One;

			size_t addr = 0;
			bool ok = false;
			if (!EvalAddr(*att.addr, ctx, addr, ok) || !ok)
				continue;

			if (n_we == TriState::Zero)
			{
				// Write
				if (c->kind == Component::Kind::Ram && c->ram != nullptr)
				{
					c->ram->Dbg_WriteByte(addr, *cpu_data);
				}
			}
			else if (n_oe == TriState::Zero)
			{
				// Read (wired-AND bus)
				uint8_t val = 0;

				if (c->kind == Component::Kind::Rom && c->rom != nullptr)
					val = c->rom->Dbg_ReadByte(addr);
				else if (c->kind == Component::Kind::Ram && c->ram != nullptr)
					val = c->ram->Dbg_ReadByte(addr);
				else
					continue;

				if (!cpu_data_dirty)
				{
					*cpu_data = val;
					cpu_data_dirty = true;
				}
				else
				{
					*cpu_data = *cpu_data & val;
				}
			}
		}

		// PPU bus

		for (auto& att : ppuAttachments)
		{
			if (att.isLatch)
				continue;

			Component* c = FindComponent(att.componentId);
			if (c == nullptr)
				continue;

			TriState n_cs = EvalLogic(*att.n_cs, ctx);
			if (n_cs != TriState::Zero)
				continue;

			TriState n_oe = att.n_oe ? EvalLogic(*att.n_oe, ctx) : TriState::Zero;
			TriState n_we = att.n_we ? EvalLogic(*att.n_we, ctx) : TriState::One;

			size_t addr = 0;
			bool ok = false;
			if (!EvalAddr(*att.addr, ctx, addr, ok) || !ok)
				continue;

			if (n_we == TriState::Zero)
			{
				// Write
				if (c->kind == Component::Kind::Ram && c->ram != nullptr)
				{
					c->ram->Dbg_WriteByte(addr, *ppu_data);
				}
			}
			else if (n_oe == TriState::Zero)
			{
				// Read (wired-AND bus)
				uint8_t val = 0;

				if (c->kind == Component::Kind::Rom && c->rom != nullptr)
					val = c->rom->Dbg_ReadByte(addr);
				else if (c->kind == Component::Kind::Ram && c->ram != nullptr)
					val = c->ram->Dbg_ReadByte(addr);
				else
					continue;

				if (!ppu_data_dirty)
				{
					*ppu_data = val;
					ppu_data_dirty = true;
				}
				else
				{
					*ppu_data = *ppu_data & val;
				}
			}
		}
	}

	void Pcb::sim(
		TriState cart_in[(size_t)CartInput::Max],
		TriState cart_out[(size_t)CartOutput::Max],
		uint16_t cpu_addr,
		uint8_t* cpu_data, bool& cpu_data_dirty,
		uint16_t ppu_addr,
		uint8_t* ppu_data, bool& ppu_data_dirty,
		CartAudioOutSignal* snd_out,
		uint16_t* exp, bool& exp_dirty)
	{
		SimContext ctx;
		memcpy(ctx.cart_in, cart_in, sizeof(ctx.cart_in));
		ctx.cpu_addr = cpu_addr;
		ctx.ppu_addr = ppu_addr;
		ctx.cpu_data = cpu_data;

		// 1. Evaluate the nets
		for (auto& net : nets)
		{
			net.value = EvalLogic(*net.from, ctx);
		}

		// 2. Simulate the mapper chips
		SimulateChips(ctx);

		// 3. Simulate the latches
		SimulateLatches(ctx);

		// 4. Memory
		SimulateMemory(ctx, cart_out, cpu_data, cpu_data_dirty, ppu_data, ppu_data_dirty);

		// 5. Mirroring

		switch (mirrorMode)
		{
			case MirrorMode::Hardwired:
				cart_out[(size_t)CartOutput::VRAM_A10] =
					hardwiredVertical ? FromByte((ppu_addr >> 10) & 1) : FromByte((ppu_addr >> 11) & 1);
				break;

			case MirrorMode::Mapper:
				cart_out[(size_t)CartOutput::VRAM_A10] = mapperMirrorNet ? EvalLogic(*mapperMirrorNet, ctx) : TriState::Z;
				break;

			default:
				cart_out[(size_t)CartOutput::VRAM_A10] = TriState::Z;
				break;
		}

		// Contains a jumper between `/PA13` and `/VRAM_CS`
		cart_out[(size_t)CartOutput::VRAM_nCS] = cart_in[(size_t)CartInput::nPA13];

		cart_out[(size_t)CartOutput::nIRQ] = TriState::Z;

		if (snd_out)
		{
			snd_out->normalized = 0.0f;
		}
	}

	uint8_t Pcb::Dbg_ReadPRGByte(size_t cpu_addr)
	{
		SimContext ctx;
		ctx.cpu_addr = (uint16_t)(cpu_addr & 0x7fff);
		ctx.ppu_addr = 0;
		ctx.cpu_data = nullptr;

		// Assumed bus state during a CPU read from the cartridge window.
		ctx.cart_in[(size_t)CartInput::M2] = TriState::One;
		ctx.cart_in[(size_t)CartInput::nROMSEL] = (cpu_addr >= 0x8000) ? TriState::Zero : TriState::One;
		ctx.cart_in[(size_t)CartInput::RnW] = TriState::One;
		ctx.cart_in[(size_t)CartInput::nRD] = TriState::Zero;
		ctx.cart_in[(size_t)CartInput::nWR] = TriState::One;
		ctx.cart_in[(size_t)CartInput::nPA13] = TriState::One;

		// Evaluate the nets, chips and latches so that latch-based boards
		// (UNROM/AOROM) reflect the current bank state.
		for (auto& net : nets)
		{
			net.value = EvalLogic(*net.from, ctx);
		}
		SimulateChips(ctx);
		SimulateLatches(ctx);

		for (auto& att : cpuAttachments)
		{
			if (att.isLatch)
				continue;

			Component* c = FindComponent(att.componentId);
			if (c == nullptr)
				continue;

			TriState n_cs = EvalLogic(*att.n_cs, ctx);
			if (n_cs != TriState::Zero)
				continue;

			size_t addr = 0;
			bool ok = false;
			if (!EvalAddr(*att.addr, ctx, addr, ok) || !ok)
				continue;

			if (c->kind == Component::Kind::Rom && c->rom != nullptr)
				return c->rom->Dbg_ReadByte(addr);

			if (c->kind == Component::Kind::Ram && c->ram != nullptr)
				return c->ram->Dbg_ReadByte(addr);
		}

		return 0;
	}

	size_t Pcb::Dbg_GetPRGSize()
	{
		for (auto& c : components)
		{
			if (c->kind == Component::Kind::Rom && c->bus == Bus::CPU)
			{
				return c->size;
			}
		}
		return 0;
	}

	size_t Pcb::Dbg_GetCHRSize()
	{
		for (auto& c : components)
		{
			if ((c->kind == Component::Kind::Rom || c->kind == Component::Kind::Ram) && c->bus == Bus::PPU)
			{
				return c->size;
			}
		}
		return 0;
	}

	size_t Pcb::Dbg_GetWRAMSize()
	{
		for (auto& c : components)
		{
			if (c->kind == Component::Kind::Ram && c->bus == Bus::CPU)
			{
				return c->size;
			}
		}
		return 0;
	}

	uint8_t Pcb::Dbg_ReadCHRByte(size_t addr)
	{
		for (auto& c : components)
		{
			if ((c->kind == Component::Kind::Rom || c->kind == Component::Kind::Ram) && c->bus == Bus::PPU)
			{
				if (c->kind == Component::Kind::Rom && c->rom != nullptr)
					return c->rom->Dbg_ReadByte(addr);
				if (c->kind == Component::Kind::Ram && c->ram != nullptr)
					return c->ram->Dbg_ReadByte(addr);
			}
		}
		return 0;
	}

	void Pcb::Dbg_WriteCHRByte(size_t addr, uint8_t data)
	{
		for (auto& c : components)
		{
			if ((c->kind == Component::Kind::Rom || c->kind == Component::Kind::Ram) && c->bus == Bus::PPU)
			{
				if (c->kind == Component::Kind::Rom && c->rom != nullptr)
					c->rom->Dbg_WriteByte(addr, data);
				if (c->kind == Component::Kind::Ram && c->ram != nullptr)
					c->ram->Dbg_WriteByte(addr, data);
			}
		}
	}

	uint8_t Pcb::Dbg_GetLatchState(const std::string& id)
	{
		for (auto& c : components)
		{
			if (c->id == id && c->kind == Component::Kind::Latch)
			{
				return c->latch;
			}
		}
		return 0;
	}

	uint8_t Pcb::Dbg_ReadWRAMByte(size_t addr)
	{
		for (auto& c : components)
		{
			if (c->kind == Component::Kind::Ram && c->bus == Bus::CPU)
			{
				return c->ram->Dbg_ReadByte(addr);
			}
		}
		return 0;
	}

	void Pcb::Dbg_WriteWRAMByte(size_t addr, uint8_t data)
	{
		for (auto& c : components)
		{
			if (c->kind == Component::Kind::Ram && c->bus == Bus::CPU)
			{
				c->ram->Dbg_WriteByte(addr, data);
			}
		}
	}
}
