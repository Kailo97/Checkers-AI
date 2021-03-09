#pragma once
#include <array>
#include "json.hpp"
using nlohmann::json;

namespace Genes
{
	struct GeneMap
	{
		template<std::size_t N>
		using locus = std::array<float, N>;

		// Convolutional neural network
		// input: matrix 15x15
		//
		// Pooling layer (8x8) x4 Convolutional layer 3x3 kernel (13x13)
		// Leaky ReLU layer (13x13)
		// x4 Convolutional layer 3x3 kernel (6x6)
		// Pooling layer (3x3)
		// Total: 144

		// Fully connected layers
		// Input layer 144 neurons
		// Hidden layer 64 neurons
		// Hidden layer 16 neurons
		// Output layer 1 neuron

		std::array<locus<9>, 4> L1;
		std::array<locus<9>, 4> L2;
		locus<144 * 64> PL1;
		locus<64 * 16> PL2;
		locus<16 * 1> PL3;

		// my color: 0 - white, 1 - black
		// capture count
		// moves count

		//locus<3 * 1> ONE;

		auto begin()
		{
			return const_cast<float*>(reinterpret_cast<const float*>(this));
		}

		auto end()
		{
			return const_cast<float*>(reinterpret_cast<const float*>(this)) + GeneMap::size();
		}

		auto begin() const
		{
			return reinterpret_cast<const float*>(this);
		}

		auto end() const
		{
			return reinterpret_cast<const float*>(this) + GeneMap::size();
		}

		static constexpr size_t size()
		{
			return sizeof(GeneMap) / sizeof(float);
		}
	};

	static inline void to_json(json& j, const GeneMap& gm) {
		j = json(gm.size(), 0.0f);
		std::copy(gm.begin(), gm.end(), j.begin());
	}

	static inline void from_json(const json& j, GeneMap& gm) {
		std::copy(j.begin(), j.end(), gm.begin());
	}
}

using Genes::GeneMap;

// auto genes = std::make_unique<GeneMap>();
// std::default_random_engine generator;
// std::uniform_real_distribution<float> distribution(-1.0, 1.0);
// std::for_each(genes->begin(), genes->end(), [&](float& weight) { weight = distribution(generator); });
