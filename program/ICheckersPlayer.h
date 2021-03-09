#pragma once
#include "CheckersRules.h"

class ICheckersPlayer
{
public:
	virtual CheckersPlayerColor Color() const = 0;
	virtual const ICheckersPlayer &Enemy() const = 0;
};