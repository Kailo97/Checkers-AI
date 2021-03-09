#include "CheckersNet.h"
#include "CCheckers.h"
#include "GeneMap.hpp"

CheckersNet::CheckersNet(const GeneMap& genes)
	: genes(genes)
{
}

float CheckersNet::activate(const CCheckersMove& move, const CheckersBoardAdaptor& adaptor) const
{
	std::vector<matrix<15, 15>> map1(1);
	adaptor.fmap(map1[0], move);
	std::vector<matrix<3, 3>> L1(genes.L1.begin(), genes.L1.end());
	std::vector<matrix<3, 3>> L2(genes.L2.begin(), genes.L2.end());
	matrix<144, 64> PL1(genes.PL1);
	matrix<64, 16> PL2(genes.PL2);
	matrix<16, 1> PL3(genes.PL3);

	auto map2 = Convolutional_Layer(map1, L1);
	auto map3 = Pooling_Layer(map2);
	auto map4 = Convolutional_Layer(map3, L2);
	auto map5 = Pooling_Layer(map4);

	matrix<1, 144> out1;
	for (size_t i = 0; i < 16; i++)
	{
		std::copy(map5[i].begin(), map5[i].end(), out1.begin() + 9 * i);
	}

	auto out2 = out1 * PL1;
	std::for_each(out2.begin(), out2.end(), Leaky_ReLU);
	auto out3 = out2 * PL2;
	std::for_each(out3.begin(), out3.end(), Leaky_ReLU);
	auto out4 = out3 * PL3;

	return out4.at(0, 0);
}
