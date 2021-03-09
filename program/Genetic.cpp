#include "Genetic.h"
#include <iostream>
#include "CCheckers.h"
#include "CheckersNetPalyer.h"
#include <random>
#include <map>
#include <mutex>
#include <execution>
#include <filesystem>
#include <fstream>

Genetic gen;

constexpr size_t population_size = 32;
//constexpr size_t best_reservation = 32;
constexpr size_t games_num = 3;
constexpr float mutate_rate = 0.2f;

std::vector<GeneMap> Genetic::Selection(std::vector<GeneMap> population)
{
	std::vector<GeneMap> parents;
	std::vector<int> fitness(population_size);
	std::multimap<size_t, std::reference_wrapper<GeneMap>> sort_tree;

	std::random_device r;
	std::default_random_engine generator(r());
	std::uniform_int_distribution<size_t> distribution(0, population_size - 1);

	std::mutex m;
	std::mutex m2;
	std::for_each(std::execution::par, population.begin(), population.end(), [&](auto& gene) {
		CheckersNetPlayer player1(gene);
		size_t fitness = 0;
		for (size_t u = 0; u < games_num; u++)
		{
			size_t e;
			{
				std::lock_guard<std::mutex> guard(m2);
				e = distribution(generator);
			}
			CheckersNetPlayer player2(population[e]);
			CCheckers checkers(player1, player2);
			checkers.InitToDefault();
			const CCheckersPlayer& winner = checkers.Play();
			if (winner.Color() == CheckersPlayerColor::White)
				fitness++;
		}
		std::lock_guard<std::mutex> guard(m);
		sort_tree.emplace(fitness, gene);
	});

	//for (size_t i = 0; i < population_size; i++)
	//{
	//	CheckersNetPlayer player1(population[i]);
	//	size_t fitness = 0;
	//	for (size_t u = 0; u < games_num; u++)
	//	{
	//		CheckersNetPlayer player2(population[distribution(generator)]);
	//		CCheckers checkers(player1, player2);
	//		checkers.InitToDefault();
	//		const CCheckersPlayer& winner = checkers.Play();
	//		if (winner.Color() == CheckersPlayerColor::White)
	//			fitness++;
	//	}
	//	sort_tree.emplace(fitness, population[i]);
	//}

	// TOOD: Почему-то они быстро становятся все одинаковыми
	// Не работает fitness функция!

	auto it = sort_tree.rbegin();
	for (size_t i = 0; i < population_size / 2; i++, it++)
		parents.emplace_back(it->second.get());

	it = sort_tree.rbegin();
	size_t best = 0;
	while (it != sort_tree.rend() && it->first == games_num)
	{
		best++;
		it++;
	}
	std::cout << "Best: " << best << std::endl;
	std::cout << "First: " << sort_tree.rbegin()->first << std::endl;

	// metrics
	CheckersHeuristicPlayer playerH;
	it = sort_tree.rbegin();
	std::uniform_int_distribution<size_t> pickside(0, 1);
	size_t wins = 0;
	for (size_t i = 0; i < 5; i++, it++)
	{
		CheckersNetPlayer playerN(it->second);
		if (pickside(generator))
		{
			CCheckers checkers(playerN, playerH);
			checkers.InitToDefault();
			const CCheckersPlayer& winner = checkers.Play();
			if (winner.Color() == CheckersPlayerColor::White)
				wins++;
		}
		else
		{
			CCheckers checkers(playerH, playerN);
			checkers.InitToDefault();
			const CCheckersPlayer& winner = checkers.Play();
			if (winner.Color() == CheckersPlayerColor::Black)
				wins++;
		}
	}
	std::cout << "Mark: " << wins << std::endl;


	return parents;
}

void Genetic::CrossOver(const GeneMap& parentA, const GeneMap& parentB, GeneMap& child)
{
	std::random_device r;
	std::default_random_engine generator(r());
	std::uniform_int_distribution<size_t> distribution(0, child.size());
	size_t divisor = distribution(generator);
	std::copy(parentA.begin(), parentA.begin() + divisor, child.begin());
	std::copy(parentB.begin() + divisor, parentB.end(), child.begin() + divisor);
}

std::vector<GeneMap> Genetic::Reproduction(std::vector<GeneMap> population)
{
	std::vector<GeneMap> new_population(population_size);

	//std::random_device r;
	//std::default_random_engine generator(r());
	//std::uniform_int_distribution<size_t> distribution(0, population.size() - 1);
	//
	//for (size_t i = 0; i < population_size; i++)
	//{
	//	CrossOver(population[distribution(generator)], population[distribution(generator)], new_population[i]);
	//	Mutation(new_population[i]);
	//}

	std::for_each(std::execution::par, new_population.begin(), new_population.end(), [&](auto& gene) {
		thread_local static std::default_random_engine* generator;
		if (!generator)
		{
			std::random_device r;
			generator = new std::default_random_engine(r());
		}
		std::uniform_int_distribution<size_t> distribution(0, population.size() - 1);
		CrossOver(population[distribution(*generator)], population[distribution(*generator)], gene);
		Mutation(gene);
	});

	return new_population;
}

void Genetic::Mutation(GeneMap& offspring)
{
	std::transform(offspring.begin(), offspring.end(), offspring.begin(), mutate);
}

float Genetic::mutate(float gene)
{
	std::random_device r;
	std::default_random_engine generator(r());
	auto rand = std::bind(std::generate_canonical<float, std::numeric_limits<float>::digits, std::default_random_engine>, generator);

	if (rand() < mutate_rate)
	{
		gene *= 1 + ((rand() - 0.5f) * 3 + (rand() - 0.5f));
	}

	return gene;
}

void Genetic::Run()
{
	while (running)
	{
		population = Evolution(std::move(population));
		generation++;
		std::cout << "Gen: " << generation << std::endl;
	}
}

Genetic::~Genetic()
{
	if (running)
	{
		running = false;
		th.join();
	}
}

void Genetic::Reset()
{
	if (running)
	{
		std::cout << "Active thread!" << std::endl;
		return;
	}

	population.clear();
	population.resize(population_size);

	std::random_device r;
	std::default_random_engine generator(r());
	std::uniform_real_distribution<float> distribution(-1.0, 1.0);

	for (auto& gene : population)
		std::for_each(gene.begin(), gene.end(), [&](float& weight) { weight = distribution(generator); });

	generation = 1;
}

void Genetic::Evolve()
{
	if (generation == 0)
	{
		std::cout << "Need to reset or load!" << std::endl;
		return;
	}

	if (running)
	{
		std::cout << "Already running!" << std::endl;
		return;
	}
	
	running = true;

	th = std::thread(&Genetic::Run, this);
}

void Genetic::Stop()
{
	if (!running)
	{
		std::cout << "Not running!" << std::endl;
		return;
	}

	running = false;
	th.join();
}

json Genetic::to_json() const
{
	return json{ {"gen", generation}, {"pop", population} };
}

void Genetic::from_json(const json& j)
{
	 j.at("gen").get_to(generation);
	 j.at("pop").get_to(population);
}

void Genetic::Save()
{
	if (running)
	{
		std::cout << "Active thread!" << std::endl;
		return;
	}

	std::filesystem::path path;

	std::cout << "Enter name: ";
	std::cin >> path;
	std::ofstream ofs(path);
	ofs << std::setw(2) << to_json() << std::endl;
}

void Genetic::Load()
{
	if (running)
	{
		std::cout << "Active thread!" << std::endl;
		return;
	}

	std::filesystem::path path;

	std::cout << "Enter name: ";
	std::cin >> path;
	std::ifstream ifs(path);
	json j;
	ifs >> j;
	from_json(j);
}

void Genetic::Stats()
{
	std::cout << "Cur generation: " << generation << std::endl;
}
