// NesCartDb - cartridge identification by content.

#include "NesCartDb.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "../Common/JsonLib/Json.h"

namespace CartPcb
{
	namespace
	{
		std::string Narrow(const wchar_t* s)
		{
			if (s == nullptr)
				return std::string();

			std::string out;
			for (; *s; s++)
			{
				out += (char)*s;
			}
			return out;
		}

		bool ReadFile(const std::string& path, std::string& out)
		{
			FILE* f = fopen(path.c_str(), "rb");
			if (f == nullptr)
				return false;

			fseek(f, 0, SEEK_END);
			long size = ftell(f);
			fseek(f, 0, SEEK_SET);

			if (size < 0)
			{
				fclose(f);
				return false;
			}

			out.resize((size_t)size);

			if (size > 0)
			{
				size_t n = fread(&out[0], 1, (size_t)size, f);
				if (n != (size_t)size)
				{
					fclose(f);
					return false;
				}
			}

			fclose(f);
			return true;
		}

		// "D3D248C9" -> 0xD3D248C9
		uint32_t ParseCrc(const wchar_t* s)
		{
			if (s == nullptr)
				return 0;

			uint32_t crc = 0;

			for (size_t n = 0; n < 8 && s[n] != 0; n++)
			{
				wchar_t c = s[n];
				int nibble = -1;

				if (c >= '0' && c <= '9') nibble = c - '0';
				else if (c >= 'a' && c <= 'f') nibble = c - 'a' + 10;
				else if (c >= 'A' && c <= 'F') nibble = c - 'A' + 10;

				if (nibble < 0)
					return 0;

				crc = (crc << 4) | (uint32_t)nibble;
			}

			return crc;
		}
	}

	uint32_t NesCartDb::Crc32(const uint8_t* data, size_t size)
	{
		static uint32_t table[256];
		static bool init = false;

		if (!init)
		{
			for (uint32_t i = 0; i < 256; i++)
			{
				uint32_t c = i;
				for (int k = 0; k < 8; k++)
				{
					c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
				}
				table[i] = c;
			}
			init = true;
		}

		uint32_t crc = 0xFFFFFFFFu;

		for (size_t i = 0; i < size; i++)
		{
			crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
		}

		return crc ^ 0xFFFFFFFFu;
	}

	bool NesCartDb::Load(const std::string& indexPath)
	{
		std::string text;
		if (!ReadFile(indexPath, text))
			return false;

		Json json;

		try
		{
			json.Deserialize((void*)text.c_str(), text.size());
		}
		catch (...)
		{
			return false;
		}

		if (json.root.children.empty())
			return false;

		Json::Value* root = json.root.children.front();

		if (root->type != Json::ValueType::Array)
			return false;

		records.clear();

		for (auto& child : root->children)
		{
			Record rec;

			Json::Value* prgCrc = child->ByName("prg_crc");
			Json::Value* chrCrc = child->ByName("chr_crc");
			Json::Value* type = child->ByName("type");

			if (prgCrc == nullptr || chrCrc == nullptr || type == nullptr)
				continue;
			if (prgCrc->type != Json::ValueType::String || chrCrc->type != Json::ValueType::String)
				continue;

			rec.prgCrc = ParseCrc(prgCrc->value.AsString);
			rec.chrCrc = ParseCrc(chrCrc->value.AsString);

			rec.ref.type = Narrow(type->value.AsString);

			Json::Value* system = child->ByName("system");
			if (system != nullptr && system->type == Json::ValueType::String)
				rec.ref.system = Narrow(system->value.AsString);

			Json::Value* pcb = child->ByName("pcb");
			if (pcb != nullptr && pcb->type == Json::ValueType::String)
				rec.ref.pcb = Narrow(pcb->value.AsString);

			Json::Value* mapper = child->ByName("mapper");
			if (mapper != nullptr && mapper->type == Json::ValueType::Int)
				rec.ref.mapper = (int)mapper->value.AsInt;

			records.push_back(rec);
		}

		loaded = true;
		return true;
	}

	void NesCartDb::FindBoards(uint32_t prgCrc, uint32_t chrCrc, std::vector<BoardRef>& out) const
	{
		out.clear();

		for (auto& rec : records)
		{
			if (rec.prgCrc == prgCrc && rec.chrCrc == chrCrc)
			{
				out.push_back(rec.ref);
			}
		}
	}
}
