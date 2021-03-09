#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include "GeneticBase.h"
#include "GeneMap.hpp"

class Genetic : public GeneticBase<GeneMap>
{
public:
	~Genetic();

	std::vector<GeneMap> Selection(std::vector<GeneMap> population) override;
	void CrossOver(const GeneMap& parentA, const GeneMap& parentB, GeneMap& child) override;
	std::vector<GeneMap> Reproduction(std::vector<GeneMap> population) override;
	void Mutation(GeneMap& offspring) override;

	void Reset();
	void Evolve();
	void Stop();
	void Save();
	void Load();
	void Stats();

	json to_json() const;
	void from_json(const json& j);

private:
	void Run();
	
	static float mutate(float gene);

private:
	std::atomic_bool running;
	std::thread th;

	std::vector<GeneMap> population;
	//std::vector<GeneMap> reservation;
	size_t generation = 0;
};