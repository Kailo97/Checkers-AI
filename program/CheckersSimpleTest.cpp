#include <iostream>
#include <random>
#include "CCheckers.h"

class DumbFirstChekersPlayer : public CCheckersPlayerBehavior
{
public:
	CCheckersMovesIt ChooseNextMove(const CCheckers& checkers, const ICheckersPlayer& player, CCheckersMovesIt first, CCheckersMovesIt last) const override
	{
		return first;
	}
};

class DumpRandomCheckersPlayer : public CCheckersPlayerBehavior
{
public:
	CCheckersMovesIt ChooseNextMove(const CCheckers &checkers, const ICheckersPlayer &player, CCheckersMovesIt first, CCheckersMovesIt last) const override
	{
		std::random_device r;
		std::default_random_engine generator(r());
		std::uniform_int_distribution<CCheckersMovesIt::difference_type> distribution(0, std::distance(first, last) - 1);
		std::advance(first, distribution(generator));
		return first;
	}
};

#include "Net.h"

void CheckresSimpleTest()
{
	DumbFirstChekersPlayer first;
	DumpRandomCheckersPlayer random;
	CCheckers checkers(first, random);
	checkers.InitToDefault();
	const CCheckersPlayer &winner = checkers.Play();
	
	std::cout << "Test success!" << std::endl;
	std::cout << "Winner is " << CheckersHelpers::ColorName(winner.Color()) << std::endl;
}