#include <array>
#include <cassert>

#include "Matrix.h"

struct Test
{
	int A;
	float B;
};

int main()
{
	Matrix<float, 4, 4> matrix;

	for (size_t i = 0; i < 4; ++i)
	{
		auto& row = matrix.GetRow(i);
		row.fill(1.f);
	}

	auto mul = matrix * 10.f;
	auto add = matrix + 10.f;
	auto add2 = mul + add;
	auto sub = matrix - 10.f;
	auto sub2 = matrix - sub;

	static_assert(sizeof(Matrix<float, 4, 4>) == sizeof(float) * 16, "Unexpected extra bytes in matrix layout?");
}