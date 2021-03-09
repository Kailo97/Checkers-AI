#pragma once

//  x - black
//  o - white
//   
//     a b c d e f g h
//   ------------------- 
// 8 | o x o x o x o x | 8
// 7 | x o x o x o x o | 7
// 6 | o x o x o x o x | 6
// 5 | x o x o x o x o | 5
// 4 | o x o x o x o x | 4
// 3 | x o x o x o x o | 3
// 2 | o x o x o x o x | 2
// 1 | x o x o x o x o | 1
//   -------------------
//     a b c d e f g h
//

enum class CheckersRow
{
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,

	Zero,
};

enum class CheckersColumn
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,

	O,
};

struct CheckersTilePos
{
	CheckersRow row;
	CheckersColumn column;
};

struct CheckersMove_s
{
	CheckersTilePos from;
	CheckersTilePos to;
};

enum class CheckersPlayerColor
{
	White,
	Black
};

#include <string>

namespace CheckersHelpers
{
	const char* ColorName(CheckersPlayerColor color);
	std::string MoveDesc(CheckersMove_s move);
	std::string PosDesc(CheckersTilePos pos);
	const char* Number(CheckersRow n);
	const char* Letter(CheckersColumn l);
};