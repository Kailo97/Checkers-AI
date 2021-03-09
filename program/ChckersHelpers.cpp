#include "CheckersRules.h"
#include <array>

namespace CheckersHelpers
{
	const char* ColorName(CheckersPlayerColor color)
	{
		switch (color)
		{
		case CheckersPlayerColor::White: return "White";
		case CheckersPlayerColor::Black: return "Black";
		}
		return "";
	}

	std::string MoveDesc(CheckersMove_s move)
	{
		std::string str;
		str.append(PosDesc(move.from));
		str.push_back('-');
		str.append(PosDesc(move.to));
		return str;
	}

	std::string PosDesc(CheckersTilePos pos)
	{
		std::string str;
		str.append(Letter(pos.column))
			.append(Number(pos.row));
		return str;
	}
	
	const char* Number(CheckersRow n)
	{
		constexpr std::array<const char*, 9> il {
			"1",
			"2",
			"3",
			"4",
			"5",
			"6",
			"7",
			"8",
			"9",
		};
		return il[static_cast<int>(n)];
	}

	const char* Letter(CheckersColumn l)
	{
		constexpr std::array<const char*, 9> il {
			"A",
			"B",
			"C",
			"D",
			"E",
			"F",
			"G",
			"H",
			"O",
		};
		return il[static_cast<int>(l)];
	}
}