#pragma once

#include <string>
#include <vector>

namespace BreaknesSDL
{
	/// <summary>
	/// The board configuration read from the BoardDescription.json (issue #515).
	/// The SDL application looks up the board by its own name (kSDLBoardName);
	/// the configuration is required - there is no built-in fallback.
	/// </summary>
	struct BoardConfig
	{
		/// <summary>
		/// Board name (equals kSDLBoardName).
		/// </summary>
		std::string name;

		std::string apu;

		/// <summary>
		/// PPU revision names of the board (up to 2).
		/// </summary>
		std::vector<std::string> ppus;

		std::string p1;

		/// <summary>
		/// Physical layout of the TV Sets in the window: "horizontal" or "vertical".
		/// </summary>
		std::string tv_layout = "horizontal";

		/// <summary>
		/// TV Set binding: tvs[i] = index of the PPU displayed on the i-th TV.
		/// Empty means the default identity binding (TV[i] shows PPU[i]).
		/// </summary>
		std::vector<int> tvs;
	};

	/// <summary>
	/// Load the board description from BoardDescription.json (next to the executable).
	/// Looks up the board by name (kSDLBoardName). Returns false when the file is
	/// missing or the board entry is not found.
	/// </summary>
	bool LoadBoardConfig(const char* path, BoardConfig& out);
}
