// The raw dumps that populate the board's ROM chips.

#pragma once

#include <cstdint>
#include <cstddef>

namespace CartPcb
{
	/// <summary>
	/// The raw dumps that are loaded into the board's ROM chips: the PRG image,
	/// the CHR image and (optionally) battery-backed RAM content.
	/// </summary>
	class CartImage
	{
	public:
		const uint8_t* prg = nullptr;
		size_t prgSize = 0;

		const uint8_t* chr = nullptr;
		size_t chrSize = 0;

		// Battery-backed RAM (may be null)
		const uint8_t* ram = nullptr;
		size_t ramSize = 0;
	};
}
