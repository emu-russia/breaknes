// PcbLoader - locate and read board JSON documents.
//
// Search order:
//   1. Built-in board definitions: <builtinDir>/boards/<boardType>.json
//   2. User board JSONs (JSONES subset): <userDir>/*.json — a file whose board
//      type matches overrides the built-in definition or adds a new board.

#pragma once

#include <string>

namespace CartPcb
{
	class PcbLoader
	{
	public:
		/// <summary>
		/// Load the board JSON text for the given board type.
		/// </summary>
		/// <param name="boardType">Board type, e.g. "NES-NROM-256".</param>
		/// <param name="builtinDir">The Nescartdb data directory.</param>
		/// <param name="userDir">A user directory with custom board JSONs (may be empty).</param>
		/// <param name="outJson">Receives the JSON text on success.</param>
		static bool LoadBoard(const std::string& boardType, const std::string& builtinDir, const std::string& userDir, std::string& outJson);
	};
}
