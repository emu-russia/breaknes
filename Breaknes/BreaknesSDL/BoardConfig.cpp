#include "pch.h"

#include "BoardConfig.h"
#include "../../Common/JsonLib/Json.h"

namespace BreaknesSDL
{
	namespace
	{
		/// <summary>
		/// Convert a JSON (wide) string to a simple Ansi string. The board names and
		/// revisions are plain ASCII, so any non-ASCII character is replaced.
		/// </summary>
		void WideToAnsi(const wchar_t* str, std::string& out)
		{
			out.clear();
			if (str == nullptr)
				return;

			size_t len = wcslen(str);
			out.resize(len);
			for (size_t i = 0; i < len; i++)
			{
				out[i] = (str[i] < 0x80) ? (char)str[i] : '?';
			}
		}

		/// <summary>
		/// Read the string value of a JSON member. Returns false if the member is missing
		/// or is not a string.
		/// </summary>
		bool GetStringValue(Json::Value* obj, const char* member_name, std::string& out)
		{
			if (obj == nullptr)
				return false;

			Json::Value* value = obj->ByName(member_name);
			if (value == nullptr || value->type != Json::ValueType::String)
				return false;

			WideToAnsi(value->value.AsString, out);
			return true;
		}
	}

	bool LoadBoardConfig(const char* path, BoardConfig& out)
	{
		BoardConfig config;

		FILE* f = fopen(path, "rb");
		if (f == nullptr)
			return false;

		fseek(f, 0, SEEK_END);
		long file_size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (file_size <= 0)
		{
			fclose(f);
			return false;
		}

		std::string text;
		text.resize((size_t)file_size);

		size_t readed = fread(&text[0], 1, (size_t)file_size, f);
		fclose(f);

		if (readed != (size_t)file_size)
			return false;

		try
		{
			Json json;
			json.Deserialize((void*)text.c_str(), text.size());

			if (json.root.children.empty())
				return false;

			Json::Value* root = json.root.children.front();

			Json::Value* boards_value = root->ByName("boards");
			if (boards_value == nullptr || boards_value->type != Json::ValueType::Array)
				return false;

			// Find the board entry by our own board name.
			for (auto it = boards_value->children.begin(); it != boards_value->children.end(); ++it)
			{
				Json::Value* board = *it;
				if (board == nullptr || board->type != Json::ValueType::Object)
					continue;

				std::string name;
				if (!GetStringValue(board, "name", name))
					continue;

				if (name != kSDLBoardName)
					continue;

				// The board entry found: use its APU/PPU/p1 and the TV settings.

				std::string value;
				if (GetStringValue(board, "apu", value) && !value.empty())
					config.apu = value;
				if (GetStringValue(board, "p1", value) && !value.empty())
					config.p1 = value;
				if (GetStringValue(board, "tv_layout", value) && !value.empty())
					config.tv_layout = value;

				config.name = name;

				// The PPU list: the "ppus" array (preferred) or the legacy "ppu" string.
				Json::Value* ppus_value = board->ByName("ppus");
				if (ppus_value != nullptr && ppus_value->type == Json::ValueType::Array)
				{
					for (auto pit = ppus_value->children.begin(); pit != ppus_value->children.end(); ++pit)
					{
						Json::Value* ppu = *pit;
						if (ppu != nullptr && ppu->type == Json::ValueType::String)
						{
							std::string ppu_name;
							WideToAnsi(ppu->value.AsString, ppu_name);
							config.ppus.push_back(ppu_name);
						}
					}
				}
				else if (GetStringValue(board, "ppu", value) && !value.empty())
				{
					config.ppus.push_back(value);
				}

				// The TV<->PPU binding: the "tvs" array (TV[i] shows PPU tvs[i]).
				Json::Value* tvs_value = board->ByName("tvs");
				if (tvs_value != nullptr && tvs_value->type == Json::ValueType::Array)
				{
					for (auto tit = tvs_value->children.begin(); tit != tvs_value->children.end(); ++tit)
					{
						Json::Value* tv = *tit;
						if (tv != nullptr && tv->type == Json::ValueType::Int)
						{
							config.tvs.push_back((int)tv->value.AsInt);
						}
					}
				}

				out = config;
				return true;
			}
		}
		catch (...)
		{
			// Fall back to the built-in defaults.
		}

		return false;
	}
}
