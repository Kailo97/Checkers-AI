#pragma once

#include <numeric>
#include <array>
#include <memory>
#include <algorithm>
#include <vector>

template <size_t M, size_t N>
class matrix
{
public:
	//matrix() = delete;
	matrix() : data(std::make_unique<float[]>(M * N)) {}
	matrix(const float(&o)[M * N]) : matrix()
	{
		std::memcpy(data.get(), o, M * N * sizeof(float));
	}
	matrix(const std::array<float, M * N>& a) : matrix()
	{
		std::memcpy(data.get(), &a.front(), M * N * sizeof(float));
	}
	matrix(const matrix<M, N>& m) : matrix()
	{
		std::memcpy(data.get(), m.data.get(), M * N * sizeof(float));
	}
	template<class RandomIt>
	matrix(RandomIt first, RandomIt last) : matrix()
	{
		copy(first, last);
	}
	matrix(float v) : matrix()
	{
		fill(v);
	}
	matrix(matrix<M, N>&& m) noexcept : data(std::move(m.data)) {}

	matrix<M, N>& operator=(matrix<M, N>&& m) noexcept
	{
		data = std::move(m.data);
		return *this;
	}

	template<class RandomIt>
	void copy(RandomIt first, RandomIt last)
	{
		std::copy(first, last, data.get());
	}

	float& at(size_t j, size_t i)
	{
		return data[j * N + i];
	}
	const float& at(size_t j, size_t i) const
	{
		return data[j * N + i];
	}

	void fill(float v)
	{
		std::fill(begin(), end(), v);
	}

	constexpr size_t rows() const
	{
		return M;
	}
	constexpr size_t columns() const
	{
		return N;
	}

	float* begin() const
	{
		return data.get();
	}
	float* end() const
	{
		return data.get() + (M * N);
	}

	float* rowbegin(size_t i) const
	{
		return data.get() + (i * N);
	}

	float* rowend(size_t i) const
	{
		return data.get() + ((i + 1) * N);
	}

	template<size_t P>
	matrix<M, P> operator*(const matrix<N, P>& other)
	{
		struct matrix_column_iterator
		{
			matrix_column_iterator() = delete;
			matrix_column_iterator(const matrix<N, P>& m, size_t col) : it(m.begin() + col) {};

			matrix_column_iterator& operator++()
			{
				it += P;
				return *this;
			}

			//bool operator==(const matrix_column_iterator& other)
			//{
			//	return it == other.it;
			//}

			const float& operator*()
			{
				return *it;
			}

			float* it;
		};

		matrix<M, P> result;
		for (size_t j = 0; j < M; j++)
		{
			for (size_t i = 0; i < P; i++)
			{
				result.at(j, i) = std::transform_reduce(rowbegin(j), rowend(j), matrix_column_iterator{other, i}, 0.0f);
			}
		}
		return result;
	}

private:
	std::unique_ptr<float[]> data;
};

template<size_t M, size_t N, class T, std::enable_if_t<(M > 2 && N > 2), bool> = true>
matrix<M - 2, N - 2> Convolutional(const matrix<M, N>& map, const T& k)
{
	matrix<M - 2, N - 2> a;
	for (size_t j = 0; j < map.rows() - 2; j++)
	{
		for (size_t i = 0; i < map.columns() - 2; i++)
		{
			float(&& slice)[9] = {
				map.at(j + 0, i + 0), map.at(j + 0, i + 1), map.at(j + 0, i + 2),
				map.at(j + 1, i + 0), map.at(j + 1, i + 1), map.at(j + 1, i + 2),
				map.at(j + 2, i + 0), map.at(j + 2, i + 1), map.at(j + 2, i + 2),
			};
			a.at(j, i) = std::transform_reduce(std::begin(slice), std::end(slice), std::begin(k), 0.0f);
		}
	}
	return a;
}

template<size_t M, size_t N>
std::vector<matrix<M - 2, N - 2>> Convolutional_Layer(const std::vector<matrix<M, N>>& maps, const std::vector<matrix<3, 3>>& kernels)
{
	std::vector<matrix<M - 2, N - 2>> out(maps.size() * kernels.size());

	for (size_t i = 0; i < maps.size(); i++)
	{
		for (size_t j = 0; j < kernels.size(); j++)
		{
			out[i * maps.size() + j] = Convolutional(maps[i], kernels[j]);
		}
	}

	return out;
}

/*template<size_t M, size_t N, std::enable_if_t<M % 2 == 0 && N % 2 == 0, bool> = true>
matrix<M / 2, N / 2> Pooling(const matrix<M, N>& map)
{
	matrix<M / 2, N / 2> a;
	for (size_t i = 0; i < a.rows(); i++)
	{
		for (size_t j = 0; j < a.columns(); j++)
		{
			float(&& slice)[4] = {
				map.at(j * 2 + 0, i * 2 + 0), map.at(j * 2 + 0, i * 2 + 1),
				map.at(j * 2 + 1, i * 2 + 0), map.at(j * 2 + 1, i * 2 + 1),
			};
			a.at(j, i) = *std::max_element(std::begin(slice), std::end(slice));
		}
	}
	return a;
}

template<size_t M, size_t N, std::enable_if_t<M % 2 != 0 && N % 2 != 0, bool> = true>
matrix<M / 2 + 1, N / 2 + 1> Pooling(const matrix<M, N>& map)
{
	matrix<M / 2 + 1, N / 2 + 1> a;
	for (size_t i = 0; i < a.rows(); i++)
	{
		for (size_t j = 0; j < a.columns(); j++)
		{
			float(&& slice)[4] = {
				map.at(j * 2 + 0, i * 2 + 0), map.at(j * 2 + 0, i * 2 + 1),
				map.at(j * 2 + 1, i * 2 + 0), map.at(j * 2 + 1, i * 2 + 1),
			};
			a.at(j, i) = *std::max_element(std::begin(slice), std::end(slice));
		}
	}
	return a;
}*/

template<size_t M, size_t N>
matrix<(M + 1) / 2, (N + 1) / 2> Pooling(const matrix<M, N>& map)
{
	matrix<(M + 1) / 2, (N + 1) / 2> a;
	for (size_t j = 0; j < M / 2; j++)
	{
		for (size_t i = 0; i < N / 2; i++)
		{
			float(&& slice)[4] = {
				map.at(j * 2 + 0, i * 2 + 0), map.at(j * 2 + 0, i * 2 + 1),
				map.at(j * 2 + 1, i * 2 + 0), map.at(j * 2 + 1, i * 2 + 1),
			};
			a.at(j, i) = *std::max_element(std::begin(slice), std::end(slice));
		}
	}

	if (M % 2 != 0)
	{
		size_t j = M / 2;
		for (size_t i = 0; i < N / 2; i++)
		{
			float(&& slice)[2] = {
				map.at(j * 2 + 0, i * 2 + 0), map.at(j * 2 + 0, i * 2 + 1),
			};
			a.at(j, i) = *std::max_element(std::begin(slice), std::end(slice));
		}
	}

	if (N % 2 != 0)
	{
		size_t i = N / 2;
		for (size_t j = 0; j < M / 2; j++)
		{
			float(&& slice)[2] = {
				map.at(j * 2 + 0, i * 2 + 0),
				map.at(j * 2 + 1, i * 2 + 0),
			};
			a.at(j, i) = *std::max_element(std::begin(slice), std::end(slice));
		}
	}

	if (M % 2 != 0 && N % 2 != 0)
	{
		size_t j = M / 2;
		size_t i = N / 2;
		a.at(j, i) = map.at(j * 2 + 0, i * 2 + 0);
	}

	return a;
}

template<size_t M, size_t N>
std::vector<matrix<(M + 1) / 2, (N + 1) / 2>> Pooling_Layer(const std::vector<matrix<M, N>>& maps)
{
	std::vector<matrix<(M + 1) / 2, (N + 1) / 2>> out(maps.size());

	std::transform(maps.begin(), maps.end(), out.begin(), Pooling<M, N>);

	return out;
}

float Leaky_ReLU(float x);
