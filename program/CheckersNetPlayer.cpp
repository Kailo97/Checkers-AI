#include "CheckersNetPalyer.h"
#include "CheckersNet.h"
#include <vector>
#include <algorithm>

CheckersNetPlayer::CheckersNetPlayer(const GeneMap& genes)
	: net(genes)
{
}

CCheckersMovesIt CheckersNetPlayer::ChooseNextMove(const CCheckers& checkers, const ICheckersPlayer& player, CCheckersMovesIt first, CCheckersMovesIt last) const
{
	using namespace std::placeholders;
	
	CheckersBoardAdaptor adapter(checkers, player.Color());
	auto activate = std::bind(&CheckersNet::activate, &net, _1, std::cref(adapter));
	std::vector<float> imp(std::distance(first, last));
	std::transform(first, last, imp.begin(), activate);
	auto maxit = std::max_element(imp.begin(), imp.end());
	auto dist = std::distance(imp.begin(), maxit);
	auto move = first;
	std::advance(move, dist);

	return move;
}

CCheckersMovesIt CheckersHeuristicPlayer::ChooseNextMove(const CCheckers& checkers, const ICheckersPlayer& player, CCheckersMovesIt first, CCheckersMovesIt last) const
{
	// Если едим, ход с наибольшим съеденым количеством
	// Если несколько равных, предпочтительнее та, после которой мы не отдаем шашку
	// Если просто ходит, то выбирает шашку ближе к своему краю доски слева

	auto op1 = [&](const CCheckersMove& move) {
		auto dirs = {
			&CCheckersTile::FrontRight,
			&CCheckersTile::FrontLeft,
			&CCheckersTile::BackLeft,
			&CCheckersTile::BackRight,
		};

		for (auto d : dirs)
		{
			const CCheckersTile& p = (move.To().*d)();
			if (!p.Outside() && p.Busy() && p.Piece().value().get().Color() != player.Color())
				return true;
		}

		return false;
	};

	if (first->Capture())
	{
		auto op2 = [&] (const CCheckersMove& m1, const CCheckersMove& m2) {
			return m1.CaptureCount() < m2.CaptureCount() || m1.CaptureCount() == m2.CaptureCount() && op1(m1) && !op1(m2);
		};
		auto move = std::max_element(first, last, op2);
		return move;
	}
	else
	{
		std::function<bool(const CCheckersMove&, const CCheckersMove&)> op;
		if (player.Color() == CheckersPlayerColor::White)
		{
			op = [](const CCheckersMove& m1, const CCheckersMove& m2) {
				return m1.From().Pos().row > m2.From().Pos().row;
			};
		}
		else
		{
			op = [](const CCheckersMove& m1, const CCheckersMove& m2) {
				return m1.From().Pos().row < m2.From().Pos().row;
			};
		}
		
		auto move = std::max_element(first, last, op);
		return move;
	}
}
