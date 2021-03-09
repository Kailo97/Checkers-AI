#pragma once
#include "ICheckersPlayer.h"

template<class C, class It>
class ICheckersPlayerBehavior
{
public:
	// Called for multiply possible moves
	virtual It ChooseNextMove(const C& checkers, const ICheckersPlayer &player, It first, It last) const = 0;
};