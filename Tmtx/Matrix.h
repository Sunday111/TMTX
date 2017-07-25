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
		template<typename, size_t, size_t> typename Final,
		typename Derived>
	class BaseMatrixHelper
	{
	private:
		using TFinal = Final<T, R, C>;

	protected:
		struct Multiply
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.GetRow<ir>()[ic] *= value;
			}
		};

		struct Plus
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.GetRow<ir>()[ic] += value;
			}

			template<size_t ir, size_t ic,
				template<typename, size_t, size_t> typename U>
			static void Execute(TFinal& _this, const U<T, R, C>& value)
			{
				_this.GetRow<ir>()[ic] +=
					value.GetRow<ir>()[ic];
			}
		};

		struct Minus
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.GetRow<ir>()[ic] -= value;
			}

			template<size_t ir, size_t ic,
				template<typename, size_t, size_t> typename U>
			static void Execute(TFinal& _this, const U<T, R, C>& value)
			{
				_this.GetRow<ir>()[ic] -=
					value.GetRow<ir>()[ic];
			}
		};

		template<typename U, typename... Args>
		TFinal& ForEachCell(Args&... args)
		{
			ForEachRow<U>(std::make_index_sequence<R>(), args...);
			return This();
		}

		template<typename U, size_t... indices, typename... Args>
		void ForEachRow(std::index_sequence<indices...>, Args&... args)
		{
			(void)std::initializer_list<int>
			{
				(ForEachCellInRow<U, indices>(std::make_index_sequence<C>(), args...), 0)...
			};
		}

		template<typename U, size_t ir, size_t... indices, typename... Args>
		void ForEachCellInRow(std::index_sequence<indices...>, Args&... args)
		{
			(void)std::initializer_list<int>
			{
				(U::Execute<ir, indices>(This(), args...), 0)...
			};
		}

		TFinal& This() { return static_cast<TFinal&>(static_cast<Derived&>(*this)); }
		const TFinal& This() const { return static_cast<const TFinal&>(static_cast<const Derived&>(*this)); }
	};

	namespace CommonMatrixHelpers
	{
		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final>
		class CommonMatrixHelpers :
			public BaseMatrixHelper<T, R, C, Matrix, Final, CommonMatrixHelpers<T, R, C, Matrix, Final>>
		{
			using TFinal = Final<T, R, C>;

		public:
			TFinal& operator*=(T value)
			{
				return ForEachCell<Multiply>(value);
			}

			TFinal operator*(T value) const
			{
				auto result = static_cast<const TFinal&>(*this);
				result *= value;
				return result;
			}

			TFinal& operator+=(T value)
			{
				return ForEachCell<Plus>(value);
			}

			TFinal operator+(T value) const
			{
				auto result = static_cast<const TFinal&>(*this);
				result += value;
				return result;
			}

			template<template<typename, size_t, size_t> typename U>
			TFinal& operator+=(const U<T, R, C>& value)
			{
				return ForEachCell<Plus>(value);
			}

			template<template<typename, size_t, size_t> typename U>
			TFinal operator+(const U<T, R, C>& value) const
			{
				auto result = static_cast<const TFinal&>(*this);
				result += value;
				return result;
			}
		};
	}

	namespace SignedMatrixHelpers
	{
		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final,
			typename Enable = void>
		class SignedMatrixHelpers :
			public BaseMatrixHelper<T, R, C, Matrix, Final,
			SignedMatrixHelpers<T, R, C, Matrix, Final>>
		{
		public:
		};

		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final>
		class SignedMatrixHelpers<T, R, C, Matrix, Final,
			std::enable_if_t<std::is_signed<T>::value>> :
			public BaseMatrixHelper<T, R, C, Matrix, Final,
			SignedMatrixHelpers<T, R, C, Matrix, Final>>
		{
			using TFinal = Final<T, R, C>;

		public:
			TFinal& operator-=(T value)
			{
				return ForEachCell<Minus>(value);
			}

			TFinal operator-(T value) const
			{
				auto result = static_cast<const TFinal&>(*this);
				result -= value;
				return result;
			}

			template<template<typename, size_t, size_t> typename U>
			TFinal& operator-=(const U<T, R, C>& value)
			{
				return ForEachCell<Minus>(value);
			}

			template<template<typename, size_t, size_t> typename U>
			TFinal operator-(const U<T, R, C>& value) const
			{
				auto result = static_cast<const TFinal&>(*this);
				result -= value;
				return result;
			}
		};
	}
	
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

		template<typename T, size_t R, size_t C>
		union UnitedData
		{
			StaticArray<StaticArray<T, C>, R> rows;
		};

		template<typename T>
		union UnitedData<T, 1, 2>
		{
			struct XYZ { T x, y; } xyz;
			struct IJK { T i, j; } ijk;
			StaticArray<StaticArray<T, 2>, 1> rows;
		};
		
		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t> typename Matrix,
			template<typename, size_t, size_t> typename Final>
		class MatrixData
		{
		public:
			using Row = StaticArray<T, C>;

			Row& GetRow(size_t index)
			{
				return data.rows[index];
			}

			decltype(auto) GetColumn()
			{
				return StaticArrayView<T, C, sizeof(T) * R>(data.rows.data()->data());
			}

			const Row& GetRow(size_t index) const
			{
				return data.rows[index];
			}

			decltype(auto) GetColumn() const
			{
				return StaticArrayView<const T, C, sizeof(T) * R>(data.rows.data()->data());
			}

			template<size_t N>
			Row& GetRow() { return data.rows[N]; }

			template<size_t N>
			const Row& GetRow() const { return data.rows[N]; }
			
			UnitedData<T, R, C> data;
		};
	}

	template<typename T, size_t R, size_t C>
	class Matrix :
		public MatrixFunctions<T, R, C, Matrix, Matrix,
			CommonMatrixHelpers::CommonMatrixHelpers,
			SignedMatrixHelpers::SignedMatrixHelpers>,
		public matrix_data_details::MatrixData<T, R, C, Matrix, Matrix>
	{
	};
}

template<typename T, size_t R, size_t C>
using Matrix = matrix_details::Matrix<T, R, C>;