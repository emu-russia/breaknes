// PcbLoader - locate and read board JSON documents.

#include "PcbLoader.h"
#include "PcbFactory.h"

#include <cstdio>
#include <vector>
#include <map>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "../Common/JsonLib/Json.h"

namespace CartPcb
{
	namespace
	{
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

		// List *.json files in a directory.
		std::vector<std::string> ListJsonFiles(const std::string& dir)
		{
			std::vector<std::string> files;

#ifdef _WIN32
			WIN32_FIND_DATAA fd;
			HANDLE h = FindFirstFileA((dir + "\\*.json").c_str(), &fd);

			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						files.push_back(dir + "\\" + fd.cFileName);
					}
				} while (FindNextFileA(h, &fd) != 0);
				FindClose(h);
			}
#else
			DIR* d = opendir(dir.c_str());
			if (d != nullptr)
			{
				struct dirent* e;
				while ((e = readdir(d)) != nullptr)
				{
					const char* name = e->d_name;
					size_t len = strlen(name);
					if (len > 5 && strcmp(name + len - 5, ".json") == 0)
					{
						files.push_back(dir + "/" + name);
					}
				}
				closedir(d);
			}
#endif

			return files;
		}

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

		// Parse the JSON and check whether the board type matches.
		bool FileHasBoardType(const std::string& path, const std::string& boardType)
		{
			std::string text;
			if (!ReadFile(path, text))
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
			Json::Value* board = root->ByName("board");

			// JSONES-style nesting: game -> cartridges[] -> board
			if (board == nullptr)
			{
				Json::Value* game = root->ByName("game");
				Json::Value* games = root->ByName("games");

				if (game == nullptr && games != nullptr && !games->children.empty())
				{
					game = games->children.front();
				}

				if (game != nullptr)
				{
					Json::Value* carts = game->ByName("cartridges");
					if (carts != nullptr && !carts->children.empty())
					{
						board = carts->children.front()->ByName("board");
					}
				}
			}

			if (board == nullptr)
				return false;

			Json::Value* t = board->ByName("type");
			return t != nullptr && t->type == Json::ValueType::String &&
				Narrow(t->value.AsString) == boardType;
		}

		// Resolve a board type to its built-in definition file via
		// <builtinDir>/boards/index.json (family mapping: e.g. all NROM variants
		// map to nrom.json). Cached after the first load.
		bool ResolveBoardFile(const std::string& builtinDir, const std::string& boardType, std::string& outFile)
		{
			static std::map<std::string, std::string> familyMap;
			static std::string cachedDir;
			static bool loaded = false;

			if (!loaded || cachedDir != builtinDir)
			{
				familyMap.clear();
				cachedDir = builtinDir;
				loaded = true;

				std::string indexText;
				if (ReadFile(builtinDir + "/boards/index.json", indexText))
				{
					Json json;
					try
					{
						json.Deserialize((void*)indexText.c_str(), indexText.size());
					}
					catch (...)
					{
						return false;
					}

					if (!json.root.children.empty())
					{
						Json::Value* root = json.root.children.front();
						if (root->type == Json::ValueType::Object)
						{
							for (auto& child : root->children)
							{
								if (child->name != nullptr && child->type == Json::ValueType::String && child->value.AsString != nullptr)
								{
									familyMap[child->name] = Narrow(child->value.AsString);
								}
							}
						}
					}
				}
			}

			auto it = familyMap.find(boardType);
			if (it != familyMap.end())
			{
				outFile = builtinDir + "/boards/" + it->second;
				return true;
			}

			return false;
		}
	}

	bool PcbLoader::LoadBoard(const std::string& boardType, const std::string& builtinDir, const std::string& userDir, std::string& outJson)
	{
		// 1. User boards first: they can override the built-in definitions.
		if (!userDir.empty())
		{
			for (auto& f : ListJsonFiles(userDir))
			{
				if (FileHasBoardType(f, boardType))
				{
					return ReadFile(f, outJson);
				}
			}
		}

		// 2. Built-in board definitions.
		if (!builtinDir.empty())
		{
			// 2a. Family mapping via boards/index.json.
			std::string defFile;
			if (ResolveBoardFile(builtinDir, boardType, defFile))
			{
				if (ReadFile(defFile, outJson))
				{
					return true;
				}
			}

			// 2b. Direct file: boards/<boardType>.json.
			std::string path = builtinDir + "/boards/" + boardType + ".json";
			if (ReadFile(path, outJson))
			{
				return true;
			}
		}

		return false;
	}
}
