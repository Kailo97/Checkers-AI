#pragma once
#include "CCheckers.h"
#include "Net.h"

class CheckersBoardAdaptor
{
public:
	CheckersBoardAdaptor(const CCheckers& checkers, CheckersPlayerColor clr);

	static std::pair<size_t, size_t> t(CheckersTilePos pos);

	void map(matrix<15, 15>& m, CheckersTilePos pos) const;

	void fmap(matrix<15, 15>& m, const CCheckersMove& move) const;

	matrix<21, 22> field;
};
