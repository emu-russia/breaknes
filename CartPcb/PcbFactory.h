// PcbFactory - builds a Pcb from a board JSON document (see CartPcb/Readme.md §5).

#pragma once

#include <string>

#include "Pcb.h"
#include "CartImage.h"

#include "../Common/JsonLib/Json.h"

namespace CartPcb
{
	class PcbFactory
	{
	public:
		/// <summary>
		/// Parse a board JSON document (either a bare board definition or a
		/// JSONES-style game -> cartridge -> board nesting) and build a Pcb.
		/// Returns nullptr and fills `error` on failure.
		/// </summary>
		static Pcb* Create(const std::string& jsonText, const CartImage& image, std::string& error);

		/// <summary>
		/// Build a Pcb from an already-located `board` JSON value.
		/// </summary>
		static Pcb* CreateFromBoardValue(Json::Value* board, const CartImage& image, std::string& error);

		/// <summary>
		/// Parse a single-bit logic expression (signals, bus bits, chip pins,
		/// ! & | ^ ( )).
		/// </summary>
		static LogicExprPtr ParseLogicExpr(const std::string& text, std::string& error);

		/// <summary>
		/// Parse an address expression (bit ranges concatenated with `|`, MSB first).
		/// </summary>
		static AddrExprPtr ParseAddrExpr(const std::string& text, std::string& error);
	};
}
