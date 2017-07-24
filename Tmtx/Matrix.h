#pragma once


#include "ArrayView.h"

#ifdef EMPTY_BASES
	#pragma error This macro is already defined!
#else
	#if defined(_MSC_VER) || defined __clang__
		#define EMPTY_BASES __declspec(empty_bases)
	#else
		#pragma error Unsupported compiler!
	#endif
#endif

namespace matrix_details
{
	template<
		typename T, size_t R, size_t C,
		template<typename, size_t, size_t> typename Matrix,
		template<typename, size_t, size_t> typename Final>
	class CommonMatrixHelpers
	{
		using TFinal = Final<T, R, C>;

	public:

		TFinal& operator*=(T value)
		{
			auto& _this = static_cast<TFinal&>(*this);
			ForEachCell<Multiplyer>(value);
			return _this;
		}

		TFinal operator*(T value) const
		{
			auto result = static_cast<const TFinal&>(*this);
			result.ForEachCell<Multiplyer>(value);
			return result;
		}

	private:
		struct Multiplyer
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.GetRow<ir>()[ic] *= value;
			}
		};

		template<typename U>
		void ForEachCell(T value)
		{
			ForEachRow<U>(value, std::make_index_sequence<R>());
		}

		template<typename U, size_t... indices>
		void ForEachRow(T value, std::index_sequence<indices...>)
		{
			(void)std::initializer_list<int> { (ForEachCellInRow<U, indices>(value, std::make_index_sequence<C>()), 0)... };
		}

		template<typename U, size_t ir, size_t... indices>
		void ForEachCellInRow(T value, std::index_sequence<indices...>)
		{
			(void)std::initializer_list<int> { (Multiplyer::Execute<ir, indices>(
				static_cast<TFinal&>(*this), value), 0)... };
		}
	};
	
	// 1. Combines mix-ins into one class.
	// 2. Applies 'empty base class' optimization.
	template<
		typename T, size_t R, size_t C,
	
		template<typename T, size_t R, size_t C>
		typename Matrix,
	
		template<typename T, size_t R, size_t C>
		typename Final,
	
		template<
			typename T, size_t R, size_t C,
			template<typename T, size_t R, size_t C> typename Matrix,
			template<typename T, size_t R, size_t C> typename Final,
			typename...>
		typename... Mixins>
	class EMPTY_BASES MatrixFunctions :
		public Mixins<T, R, C, Matrix, Final>...
	{
	};

	namespace matrix_data_details
	{
		template<typename T, size_t N>
		using StaticArray = std::array<T, N>;
		
		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final>
		class MatrixData
		{
		public:
			using Row = StaticArray<T, C>;

			Row& GetRow(int index)
			{
				return rows[index];
			}

			decltype(auto) GetColumn()
			{
				return StaticArrayView<T, C, sizeof(T) * R>(rows.data()->data());
			}

			const Row& GetRow(int index) const
			{
				return rows[index];
			}

			decltype(auto) GetColumn() const
			{
				return StaticArrayView<const T, C, sizeof(T) * R>(rows.data()->data());
			}

			template<size_t N>
			Row& GetRow()
			{
				return rows[N];
			}

			template<size_t N>
			const Row& GetRow() const
			{
				return rows[N];
			}
			
			StaticArray<Row, R> rows;
		};

		template<
			typename T,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final>
		class MatrixData<T, 1, 2, Matrix, Final>
		{
		public:
			using Row = StaticArray<T, 2>;

			Row& GetRow(int index)
			{
				assert(index > 0);
				return rows[0];
			}

			union
			{
				struct { T x, y; };
				struct { T i, j; };
				StaticArray<Row, 1> rows;
			};
		};
	}

	template<typename T, size_t R, size_t C>
	class Matrix :
		public MatrixFunctions<T, R, C, Matrix, Matrix,
			CommonMatrixHelpers>,
		public matrix_data_details::MatrixData<T, R, C, Matrix, Matrix>
	{
	};
}

template<typename T, size_t R, size_t C>
using Matrix = matrix_details::Matrix<T, R, C>;