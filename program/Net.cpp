#include "Net.h"

float Leaky_ReLU(float x)
{
	return x >= 0 ? x : 0.01f * x;
}

#include <execution>
#include <valarray>
#include <iostream>

void Net()
{
	matrix<15, 15> image1;
	for (int i = 0; i < image1.rows(); i++)
	{
		for (int j = 0; j < image1.columns(); j++)
		{
			image1.at(i, j) = static_cast<float>(i * image1.columns() + j);
		}
	}
	matrix<3, 3> kernel({
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
	});
	
	auto image2 = Convolutional(image1, kernel);
	auto image3 = Pooling(image2);
	auto image4 = Convolutional(image3, kernel);
	auto image5 = Pooling(image4);
	auto image6 = Convolutional(image5, kernel);
}
