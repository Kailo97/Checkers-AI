#pragma once
#include <array>

template<typename _Ty, size_t _Size>
class Genome
{
public:
	constexpr _Ty& Map(size_t offset, size_t length)
	{
		static_assert(offset + length <= data.size());

		return data.at(offset);
	}

private:
	std::array<_Ty, _Size> data;
};