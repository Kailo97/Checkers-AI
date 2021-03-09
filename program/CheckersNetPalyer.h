#pragma once
#include "CCheckers.h"
#include "CheckersNet.h"

class CheckersNetPlayer : public CCheckersPlayerBehavior
{
public:
	CheckersNetPlayer(const GeneMap& genes);

	CCheckersMovesIt ChooseNextMove(const CCheckers& checkers, const ICheckersPlayer& player, CCheckersMovesIt first, CCheckersMovesIt last) const override;

private:
	CheckersNet net;
};

class CheckersHeuristicPlayer : public CCheckersPlayerBehavior
{
	CCheckersMovesIt ChooseNextMove(const CCheckers& checkers, const ICheckersPlayer& player, CCheckersMovesIt first, CCheckersMovesIt last) const override;
};
