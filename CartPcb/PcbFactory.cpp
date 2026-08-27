// PcbFactory - builds a Pcb from a board JSON document.

#include "PcbFactory.h"

#include <cstdlib>
#include <cctype>

#include "../Common/JsonLib/Json.h"

namespace CartPcb
{
	namespace
	{
		// ------------------------------------------------------------------
		// Json helpers
		// ------------------------------------------------------------------

		std::string Narrow(const wchar_t* s)
		{
			if (s == nullptr)
				return std::string();

			std::string out;
			for (; *s; s++)
			{
				out += (char)*s;	// board JSON names are ASCII
			}
			return out;
		}

		bool GetStr(Json::Value* v, const char* name, std::string& out)
		{
			Json::Value* c = v->ByName(name);
			if (c != nullptr && c->type == Json::ValueType::String && c->value.AsString != nullptr)
			{
				out = Narrow(c->value.AsString);
				return true;
			}
			return false;
		}

		bool GetInt(Json::Value* v, const char* name, int& out)
		{
			Json::Value* c = v->ByName(name);
			if (c != nullptr && c->type == Json::ValueType::Int)
			{
				out = (int)c->value.AsInt;
				return true;
			}
			return false;
		}

		Json::Value* FindBoardValue(Json::Value* root)
		{
			if (root == nullptr)
				return nullptr;

			Json::Value* board = root->ByName("board");
			if (board != nullptr)
				return board;

			// JSONES-style nesting: game -> cartridges[] -> board
			Json::Value* game = nullptr;

			Json::Value* games = root->ByName("games");
			if (games != nullptr && games->type == Json::ValueType::Array && !games->children.empty())
			{
				game = games->children.front();
			}
			else
			{
				game = root->ByName("game");
			}

			if (game != nullptr)
			{
				Json::Value* carts = game->ByName("cartridges");
				if (carts != nullptr && carts->type == Json::ValueType::Array && !carts->children.empty())
				{
					Json::Value* cart = carts->children.front();
					Json::Value* b = cart->ByName("board");
					if (b != nullptr)
						return b;
				}
			}

			return nullptr;
		}

		// ------------------------------------------------------------------
		// Expression parser
		// ------------------------------------------------------------------

		struct ExprParser
		{
			const char* text;
			size_t pos = 0;
			bool failed = false;
			std::string error;

			ExprParser(const std::string& s) : text(s.c_str()) {}

			void SkipSpaces()
			{
				while (text[pos] == ' ' || text[pos] == '\t')
					pos++;
			}

			char Peek()
			{
				SkipSpaces();
				return text[pos];
			}

			char Peek2()
			{
				SkipSpaces();
				return text[pos + 1];
			}

			void Next()
			{
				SkipSpaces();
				pos++;
			}

			bool Eat(char c)
			{
				SkipSpaces();
				if (text[pos] == c)
				{
					pos++;
					return true;
				}
				return false;
			}

			bool End()
			{
				SkipSpaces();
				return text[pos] == 0;
			}

			static bool IsIdentChar(char c)
			{
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
			}

			std::string ReadIdent()
			{
				SkipSpaces();
				std::string id;
				while (IsIdentChar(text[pos]))
				{
					id += text[pos++];
				}
				return id;
			}

			int ReadInt()
			{
				SkipSpaces();
				int v = 0;
				while (text[pos] >= '0' && text[pos] <= '9')
				{
					v = v * 10 + (text[pos] - '0');
					pos++;
				}
				return v;
			}
		};

		LogicExprPtr ParseOr(ExprParser& p);
		LogicExprPtr ParseXor(ExprParser& p);
		LogicExprPtr ParseAnd(ExprParser& p);
		LogicExprPtr ParseUnary(ExprParser& p);

		LogicExprPtr ParseUnary(ExprParser& p)
		{
			if (p.Peek() == '!')
			{
				p.Next();
				auto e = std::make_shared<LogicExpr>();
				e->op = LogicExpr::Op::Not;
				e->a = ParseUnary(p);
				return e;
			}

			if (p.Peek() == '(')
			{
				p.Next();
				auto e = ParseOr(p);
				if (!p.Eat(')'))
				{
					p.failed = true;
					p.error = "missing ')'";
				}
				return e;
			}

			auto e = std::make_shared<LogicExpr>();
			e->op = LogicExpr::Op::Atom;

			std::string ident = p.ReadIdent();

			if (ident == "gnd")
			{
				e->atom = LogicExpr::AtomKind::Gnd;
				return e;
			}
			if (ident == "vdd")
			{
				e->atom = LogicExpr::AtomKind::Vdd;
				return e;
			}

			if (p.Peek() == '[')
			{
				p.Next();
				int bit = p.ReadInt();
				p.Eat(']');

				if (ident == "cpu_addr")
				{
					e->atom = LogicExpr::AtomKind::CpuAddrBit;
					e->bit = bit;
				}
				else if (ident == "ppu_addr")
				{
					e->atom = LogicExpr::AtomKind::PpuAddrBit;
					e->bit = bit;
				}
				else if (ident == "cpu_data")
				{
					e->atom = LogicExpr::AtomKind::CpuDataBit;
					e->bit = bit;
				}
				else
				{
					p.failed = true;
					p.error = "unknown bus '" + ident + "'";
				}
				return e;
			}

			if (p.Peek() == '.')
			{
				p.Next();
				std::string pin = p.ReadIdent();
				e->atom = LogicExpr::AtomKind::ChipPin;
				e->chip = ident;
				e->name = pin;
				return e;
			}

			e->atom = LogicExpr::AtomKind::Signal;
			e->name = ident;
			return e;
		}

		LogicExprPtr ParseAnd(ExprParser& p)
		{
			auto left = ParseUnary(p);

			while (p.Peek() == '&')
			{
				p.Next();
				auto right = ParseUnary(p);
				auto e = std::make_shared<LogicExpr>();
				e->op = LogicExpr::Op::And;
				e->a = left;
				e->b = right;
				left = e;
			}

			return left;
		}

		LogicExprPtr ParseXor(ExprParser& p)
		{
			auto left = ParseAnd(p);

			while (p.Peek() == '^')
			{
				p.Next();
				auto right = ParseAnd(p);
				auto e = std::make_shared<LogicExpr>();
				e->op = LogicExpr::Op::Xor;
				e->a = left;
				e->b = right;
				left = e;
			}

			return left;
		}

		LogicExprPtr ParseOr(ExprParser& p)
		{
			auto left = ParseXor(p);

			while (p.Peek() == '|')
			{
				p.Next();
				auto right = ParseXor(p);
				auto e = std::make_shared<LogicExpr>();
				e->op = LogicExpr::Op::Or;
				e->a = left;
				e->b = right;
				left = e;
			}

			return left;
		}

		std::vector<std::string> ExpandPinRange(const std::string& a, const std::string& b)
		{
			// Try to expand a numeric pin range: "PRG_A14".."PRG_A17" -> PRG_A14, PRG_A15, PRG_A16, PRG_A17
			size_t ia = a.size();
			size_t ib = b.size();

			while (ia > 0 && isdigit((unsigned char)a[ia - 1]))
				ia--;
			while (ib > 0 && isdigit((unsigned char)b[ib - 1]))
				ib--;

			if (ia > 0 && ib > 0 && a.substr(0, ia) == b.substr(0, ib))
			{
				int na = atoi(a.c_str() + ia);
				int nb = atoi(b.c_str() + ib);
				std::string prefix = a.substr(0, ia);
				std::vector<std::string> pins;

				if (na <= nb)
				{
					for (int i = na; i <= nb; i++)
						pins.push_back(prefix + std::to_string(i));
				}
				else
				{
					for (int i = na; i >= nb; i--)
						pins.push_back(prefix + std::to_string(i));
				}
				return pins;
			}

			// Fallback: a plain two-pin list in the given order.
			return { a, b };
		}

		void ParseAddrPart(ExprParser& p, AddrExpr* e)
		{
			AddrTerm t;

			std::string ident = p.ReadIdent();

			if (ident == "gnd")
			{
				t.kind = AddrTerm::Kind::Gnd;
				e->terms.push_back(t);
				return;
			}
			if (ident == "vdd")
			{
				t.kind = AddrTerm::Kind::Vdd;
				e->terms.push_back(t);
				return;
			}

			if (p.Peek() == '[')
			{
				p.Next();
				int a = p.ReadInt();
				int b = a;
				if (p.Eat(':'))
				{
					b = p.ReadInt();
				}
				p.Eat(']');

				t.hi = a;
				t.lo = b;

				if (ident == "cpu_addr")
					t.kind = AddrTerm::Kind::CpuAddrRange;
				else if (ident == "ppu_addr")
					t.kind = AddrTerm::Kind::PpuAddrRange;
				else if (ident == "cpu_data")
					t.kind = AddrTerm::Kind::CpuDataRange;
				else
				{
					p.failed = true;
					p.error = "unknown bus '" + ident + "' in addr";
				}

				e->terms.push_back(t);
				return;
			}

			if (p.Peek() == '.')
			{
				p.Next();
				std::string pinA = p.ReadIdent();

				t.chip = ident;
				t.kind = AddrTerm::Kind::ChipPinList;

				if (p.Peek() == '.' && p.Peek2() == '.')
				{
					p.Next();
					p.Next();
					std::string pinB = p.ReadIdent();
					t.pins = ExpandPinRange(pinA, pinB);
				}
				else
				{
					t.pins.push_back(pinA);
				}

				e->terms.push_back(t);
				return;
			}

			p.failed = true;
			p.error = "unknown addr term '" + ident + "'";
		}

		bool ParseBusAttachments(Pcb* pcb, Json::Value* busValue, bool isCpu, std::string& error)
		{
			if (busValue == nullptr || busValue->type != Json::ValueType::Object)
				return true;

			for (auto& child : busValue->children)
			{
				// The attachment key is a label; the component is named by the
				// `chip` field (falls back to the key).
				std::string compId = child->name != nullptr ? child->name : "";
				std::string chipField;
				if (GetStr(child, "chip", chipField))
				{
					compId = chipField;
				}

				Pcb::Attachment a;
				a.componentId = compId;

				std::string s;

				// A latch attachment has a `clk` strobe
				if (child->ByName("clk") != nullptr)
				{
					a.isLatch = true;

					if (GetStr(child, "clk", s))
					{
						a.clk = PcbFactory::ParseLogicExpr(s, error);
						if (!a.clk)
							return false;
					}
					if (GetStr(child, "n_we", s))
					{
						a.we = PcbFactory::ParseLogicExpr(s, error);
						if (!a.we)
							return false;
					}
					if (GetStr(child, "data", s))
					{
						a.data = PcbFactory::ParseAddrExpr(s, error);
						if (!a.data)
							return false;
					}
				}
				else
				{
					if (GetStr(child, "n_cs", s))
					{
						a.n_cs = PcbFactory::ParseLogicExpr(s, error);
						if (!a.n_cs)
							return false;
					}
					if (GetStr(child, "n_oe", s))
					{
						a.n_oe = PcbFactory::ParseLogicExpr(s, error);
						if (!a.n_oe)
							return false;
					}
					if (GetStr(child, "n_we", s))
					{
						a.n_we = PcbFactory::ParseLogicExpr(s, error);
						if (!a.n_we)
							return false;
					}
					if (GetStr(child, "addr", s))
					{
						a.addr = PcbFactory::ParseAddrExpr(s, error);
						if (!a.addr)
							return false;
					}
					else
					{
						error = "attachment '" + compId + "' has no addr";
						return false;
					}
				}

				if (isCpu)
					pcb->AddCpuAttachment(a);
				else
					pcb->AddPpuAttachment(a);
			}

			return true;
		}
	}

	LogicExprPtr PcbFactory::ParseLogicExpr(const std::string& text, std::string& error)
	{
		ExprParser p(text);
		auto e = ParseOr(p);

		if (p.failed || !p.End())
		{
			error = "logic expression error: '" + text + "'" + (p.error.empty() ? "" : " (" + p.error + ")");
			return nullptr;
		}

		return e;
	}

	AddrExprPtr PcbFactory::ParseAddrExpr(const std::string& text, std::string& error)
	{
		ExprParser p(text);
		auto e = std::make_shared<AddrExpr>();

		ParseAddrPart(p, e.get());
		if (p.failed)
		{
			error = "addr expression error: '" + text + "' (" + p.error + ")";
			return nullptr;
		}

		while (p.Peek() == '|')
		{
			p.Next();
			ParseAddrPart(p, e.get());
			if (p.failed)
			{
				error = "addr expression error: '" + text + "' (" + p.error + ")";
				return nullptr;
			}
		}

		if (!p.End())
		{
			error = "addr expression error: '" + text + "'";
			return nullptr;
		}

		return e;
	}

	Pcb* PcbFactory::Create(const std::string& jsonText, const CartImage& image, std::string& error)
	{
		Json json;

		try
		{
			json.Deserialize((void*)jsonText.c_str(), jsonText.size());
		}
		catch (...)
		{
			error = "board JSON parse error";
			return nullptr;
		}

		if (json.root.children.empty())
		{
			error = "empty board JSON";
			return nullptr;
		}

		Json::Value* board = FindBoardValue(json.root.children.front());
		if (board == nullptr)
		{
			error = "no 'board' found in JSON";
			return nullptr;
		}

		return CreateFromBoardValue(board, image, error);
	}

	Pcb* PcbFactory::CreateFromBoardValue(Json::Value* board, const CartImage& image, std::string& error)
	{
		Pcb* pcb = new Pcb;

		std::string type;
		std::string pcbRev;

		if (GetStr(board, "type", type))
		{
			GetStr(board, "pcb", pcbRev);
			pcb->SetBoardType(type, pcbRev);
		}

		// ---- components ----

		Json::Value* comps = board->ByName("components");
		if (comps != nullptr && comps->type == Json::ValueType::Object)
		{
			for (auto& child : comps->children)
			{
				std::string id = child->name != nullptr ? child->name : "";

				std::string kind;
				std::string bus;
				std::string imageName;
				int size = 0;

				GetStr(child, "kind", kind);
				GetStr(child, "bus", bus);
				GetStr(child, "image", imageName);
				GetInt(child, "size", size);

				Bus busEnum = (bus == "ppu") ? Bus::PPU : Bus::CPU;

				if (kind == "rom")
				{
					pcb->AddComponent(id, Pcb::Component::Kind::Rom, busEnum, (size_t)size, imageName);
				}
				else if (kind == "ram")
				{
					pcb->AddComponent(id, Pcb::Component::Kind::Ram, busEnum, (size_t)size, imageName);
				}
				else if (kind == "latch")
				{
					pcb->AddComponent(id, Pcb::Component::Kind::Latch, Bus::None, 0, "");
				}
				else if (kind == "chip")
				{
					std::string chipType;
					GetStr(child, "chip", chipType);
					pcb->AddChip(id, chipType);
				}
				else
				{
					error = "unknown component kind '" + kind + "' for '" + id + "'";
					delete pcb;
					return nullptr;
				}
			}
		}

		// ---- circuit ----

		Json::Value* circuit = board->ByName("circuit");
		if (circuit != nullptr)
		{
			Json::Value* mirroring = circuit->ByName("mirroring");
			if (mirroring != nullptr)
			{
				std::string mode;
				GetStr(mirroring, "mode", mode);

				if (mode == "hardwired")
				{
					int h = 0;
					int v = 0;
					GetInt(mirroring, "h", h);
					GetInt(mirroring, "v", v);
					pcb->SetHardwiredMirroring(v == 1);
				}
				else if (mode == "mapper")
				{
					std::string net;
					if (GetStr(mirroring, "net", net))
					{
						LogicExprPtr e = ParseLogicExpr(net, error);
						if (!e)
						{
							delete pcb;
							return nullptr;
						}
						pcb->SetMapperMirroring(e);
					}
				}
			}

			if (!ParseBusAttachments(pcb, circuit->ByName("cpu"), true, error))
			{
				delete pcb;
				return nullptr;
			}

			if (!ParseBusAttachments(pcb, circuit->ByName("ppu"), false, error))
			{
				delete pcb;
				return nullptr;
			}

			Json::Value* nets = circuit->ByName("nets");
			if (nets != nullptr && nets->type == Json::ValueType::Array)
			{
				for (auto& nv : nets->children)
				{
					std::string name;
					std::string from;

					if (!GetStr(nv, "name", name) || !GetStr(nv, "from", from))
					{
						error = "bad net entry";
						delete pcb;
						return nullptr;
					}

					Pcb::Net net;
					net.name = name;
					net.from = ParseLogicExpr(from, error);

					if (!net.from)
					{
						delete pcb;
						return nullptr;
					}

					pcb->AddNet(net);
				}
			}
		}

		pcb->LoadImages(image);
		return pcb;
	}
}
