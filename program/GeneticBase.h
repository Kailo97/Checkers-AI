#pragma once
#include <vector>

template<class T>
class GeneticBase
{
public:
	virtual std::vector<T> Selection(std::vector<T> population) = 0;
	virtual void CrossOver(const T& parentA, const T& parentB, T& child) = 0;
	virtual std::vector<T> Reproduction(std::vector<T> population) = 0;
	
	virtual void Mutation(T &offspring)
	{
	}

	// Pass one population
	std::vector<T> Evolution(std::vector<T> population)
	{
		return Reproduction(Selection(std::move(population)));
	}
};
