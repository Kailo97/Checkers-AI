#include "CheckersBoardAdaptor.h"

CheckersBoardAdaptor::CheckersBoardAdaptor(const CCheckers& checkers, CheckersPlayerColor clr)
{
	field.fill(0.0);
	auto op = [&](const CCheckersTile& tile) {
		auto [i, j] = CheckersBoardAdaptor::t(tile.Pos());
		auto p = tile.Piece();
		field.at(i + 7, j + 7) = p ? (p.value().get().Color() == clr ? 0.5f : 1.0f) : -1.0f;
	};
	std::for_each(checkers.tbegin(), checkers.tend(), op);
}

std::pair<size_t, size_t> CheckersBoardAdaptor::t(CheckersTilePos pos)
{
	std::pair<size_t, size_t> r_offs[] = {
		{3, 0},
		{3, 1},
		{4, 1},
		{4, 2},
		{5, 2},
		{5, 3},
		{6, 3},
		{6, 4},
	};

	std::pair<size_t, size_t> c_offs[] = {
		{ 0, 0},
		{ 0, 0},
		{-1, 1},
		{-1, 1},
		{-2, 2},
		{-2, 2},
		{-3, 3},
		{-3, 3},
	};

	auto ro = r_offs[static_cast<size_t>(pos.row)];
	auto co = c_offs[static_cast<size_t>(pos.column)];

	return { ro.first + co.first, ro.second + co.second };
}

void CheckersBoardAdaptor::map(matrix<15, 15>& m, CheckersTilePos pos) const
{
	auto [r, c] = t(pos);

	for (size_t i = 0; i < 15; i++)
	{
		std::copy(field.rowbegin(r + i) + c, field.rowbegin(r + i) + c + 14, m.rowbegin(i));
	}
}

void CheckersBoardAdaptor::fmap(matrix<15, 15>& m, const CCheckersMove& move) const
{
	map(m, move.From().Pos());

	auto c = t(move.From().Pos());
	auto op = [&](CheckersTilePos pos) -> float& {
		auto p = t(pos);
		return m.at(7 + (p.first - c.first), 7 + (p.second - c.second));
	};

	op(move.From().Pos()) = -1.0;
	op(move.To().Pos()) = 0.5;

	for (auto it = move.cptr_begin(); it != move.cptr_end(); it++)
	{
		op(it->get().Place().Pos()) = -1.0;
	}
}
