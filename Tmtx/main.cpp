#include <array>
#include <cassert>

#include "Matrix.h"

struct Test
{
	int A;
	float B;
};

template<typename T, size_t R, size_t C, typename Traits>
void CheckMatrixValues(Matrix<T, R, C, Traits>& m, T checkValue)
{
	for (size_t ir = 0; ir < R; ++ir)
	{
		for (size_t ic = 0; ic < C; ++ic)
		{
			assert(checkValue == m.data.GetCell(ir, ic));
		}
	}
}


class AltTraits
{
public:
	static constexpr matrix_details::DataType DataType = matrix_details::DataType::Columns;
};

template<typename Traits>
void TMain()
{
	Matrix<float, 4, 4, Traits> matrix;

	for (size_t i = 0; i < 4; ++i)
	{
		for (size_t j = 0; j < 4; ++j)
		{
			matrix.data.GetCell(i, j) = 1.f;
		}
	}

	CheckMatrixValues(matrix, 1.f);

	constexpr float testValue = 10.f;

	auto mul = matrix * testValue;
	CheckMatrixValues(mul, testValue);

	auto add = matrix + 10.f;
	auto add2 = mul + add;
	auto sub = matrix - 10.f;
	auto sub2 = matrix - sub;

	static_assert(sizeof(Matrix<float, 4, 4>) == sizeof(float) * 16, "Unexpected extra bytes in matrix layout?");
}

int main()
{
	TMain<matrix_details::DefaultTraits>();
	TMain<AltTraits>();

	return 0;
}