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
	enum class DataType
	{
		Rows,
		Columns
	};

	template<
		typename T, size_t R, size_t C,
		template<typename, size_t, size_t, typename Traits> typename Matrix,
		template<typename, size_t, size_t, typename Traits> typename Final,
		typename Traits,
		typename Derived>
	class BaseMatrixHelper
	{
	private:
		using TFinal = Final<T, R, C, Traits>;

	protected:

		struct Multiply
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.data.GetCell<ir, ic>() *= value;
			}
		};
		
		struct Plus
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.data.GetCell<ir, ic>() += value;
			}

			template<size_t ir, size_t ic,
				template<typename, size_t, size_t, typename> typename U>
			static void Execute(TFinal& _this, const U<T, R, C, Traits>& value)
			{
				_this.data.GetCell<ir, ic>() +=
					value.data.GetCell<ir, ic>();
			}
		};

		struct Minus
		{
			template<size_t ir, size_t ic>
			static void Execute(TFinal& _this, T value)
			{
				_this.data.GetCell<ir, ic>() -= value;
			}

			template<size_t ir, size_t ic,
				template<typename, size_t, size_t, typename> typename U>
			static void Execute(TFinal& _this, const U<T, R, C, Traits>& value)
			{
				_this.data.GetCell<ir, ic>() -=
					value.data.GetCell<ir, ic>();
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
			template<typename, size_t, size_t, typename Traits> typename Matrix,
			template<typename, size_t, size_t, typename Traits> typename Final,
			typename Traits>
		class CommonMatrixHelpers :
			public BaseMatrixHelper<T, R, C, Matrix, Final, Traits,
				CommonMatrixHelpers<T, R, C, Matrix, Final, Traits>>
		{
			using TFinal = Final<T, R, C, Traits>;

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

			template<template<typename, size_t, size_t, typename> typename U>
			TFinal& operator+=(const U<T, R, C, Traits>& value)
			{
				return ForEachCell<Plus>(value);
			}

			template<template<typename, size_t, size_t, typename> typename U>
			TFinal operator+(const U<T, R, C, Traits>& value) const
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
			template<typename, size_t, size_t, typename Traits> typename Matrix,
			template<typename, size_t, size_t, typename Traits> typename Final,
			typename Traits,
			typename Enable = void>
		class SignedMatrixHelpers :
			public BaseMatrixHelper<T, R, C, Matrix, Final, Traits,
			SignedMatrixHelpers<T, R, C, Matrix, Final, Traits>>
		{
		public:
		};

		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t, typename Traits> typename Matrix,
			template<typename, size_t, size_t, typename Traits> typename Final,
			typename Traits>
		class SignedMatrixHelpers<T, R, C, Matrix, Final, Traits,
			std::enable_if_t<std::is_signed<T>::value>> :
			public BaseMatrixHelper<T, R, C, Matrix, Final, Traits,
				SignedMatrixHelpers<T, R, C, Matrix, Final, Traits>>
		{
			using TFinal = Final<T, R, C, Traits>;

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

			template<template<typename, size_t, size_t, typename> typename U>
			TFinal& operator-=(const U<T, R, C, Traits>& value)
			{
				return ForEachCell<Minus>(value);
			}

			template<template<typename, size_t, size_t, typename> typename U>
			TFinal operator-(const U<T, R, C, Traits>& value) const
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
	
		template<typename T, size_t R, size_t C, typename Traits>
		typename Matrix,
	
		template<typename T, size_t R, size_t C, typename Traits>
		typename Final,

		typename Traits,
	
		template<
			typename T, size_t R, size_t C,
			template<typename T, size_t R, size_t C, typename Traits> typename Matrix,
			template<typename T, size_t R, size_t C, typename Traits> typename Final,
			typename Traits,
			typename...>
		typename... Mixins>
	class EMPTY_BASES MatrixFunctions :
		public Mixins<T, R, C, Matrix, Final, Traits>...
	{
	};

	namespace matrix_data_details
	{
		template<typename T, size_t N>
		using StaticArray = std::array<T, N>;

		template<typename T, size_t R, size_t C, typename Traits>
		union UnitedRowsData
		{
			using Row = StaticArray<T, C>;

			template<size_t ir, size_t ic>
			T& GetCell() { return rows[ir][ic]; }
			T& GetCell(size_t ir, size_t ic) { return rows[ir][ic]; }

			template<size_t ir, size_t ic>
			const T& GetCell() const { return rows[ir][ic]; }
			const T& GetCell(size_t ir, size_t ic) const { return rows[ir][ic]; }
			
			StaticArray<Row, R> rows;
		};

		template<typename T, size_t R, size_t C, typename Traits>
		union UnitedColumnsData
		{
			using Column = StaticArray<T, R>;

			template<size_t ir, size_t ic>
			T& GetCell() { return columns[ic][ir]; }
			T& GetCell(size_t ir, size_t ic) { return columns[ic][ir]; }

			template<size_t ir, size_t ic>
			const T& GetCell() const { return columns[ic][ir]; }
			const T& GetCell(size_t ir, size_t ic) const { return columns[ic][ir]; }

			StaticArray<Column, C> columns;
		};

		template<typename T, size_t R, size_t C,
			typename Traits>
		using UnitedData = std::conditional_t<
			Traits::DataType == DataType::Rows,
			UnitedRowsData<T, R, C, Traits>,
			UnitedColumnsData<T, R, C, Traits>>;
		
		template<
			typename T, size_t R, size_t C,
			template<typename, size_t, size_t, typename Traits> typename Matrix,
			template<typename, size_t, size_t, typename Traits> typename Final,
			typename Traits>
		class MatrixData
		{
		public:
			UnitedData<T, R, C, Traits> data;
		};
	}

	class DefaultTraits
	{
	public:
		static constexpr DataType DataType = DataType::Rows;
	};

	template<typename T, size_t R, size_t C, typename Traits>
	class Matrix :
		public MatrixFunctions<T, R, C, Matrix, Matrix, Traits,
			CommonMatrixHelpers::CommonMatrixHelpers,
			SignedMatrixHelpers::SignedMatrixHelpers>,
		public matrix_data_details::MatrixData<T, R, C, Matrix, Matrix, Traits>
	{
	};
}

template<typename T, size_t R, size_t C, typename Traits = matrix_details::DefaultTraits>
using Matrix = matrix_details::Matrix<T, R, C, Traits>;