#pragma once

#include <vector>
#include "Net.h"
#include "CheckersBoardAdaptor.h"
#include "GeneMap.hpp"

class CCheckers;
class CCheckersMove;

class CheckersNet
{
public:
	CheckersNet(const GeneMap& genes);

	float activate(const CCheckersMove& move, const CheckersBoardAdaptor &adaptor) const;

private:
	const GeneMap& genes;
};