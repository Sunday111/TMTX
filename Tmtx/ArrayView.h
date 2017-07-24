#pragma once

#include <cassert>

namespace array_view_helpers
{
	template<typename T>
	struct CanCast
	{
		template<typename U>
		struct To
		{
			static constexpr bool value =
				std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value &&
				!(std::is_const<T>::value && !std::is_const<U>::value);
		};
	};

	template<
		typename T,
		template<typename T> typename Final>
	class ArrayViewBase
	{
	private:
		decltype(auto) Size() const { return This().Size(); }
		decltype(auto) Stride() const { return This().Stride(); }
		decltype(auto) Data() const { return This().Data(); }

	protected:
		decltype(auto) This() { return static_cast<Final<T>&>(*this); }
		decltype(auto) This() const { return static_cast<const Final<T>&>(*this); }

		template<typename U>
		static constexpr bool CanCastTo() { return typename CanCast<T>::To<U>::value; }

		template<typename U>
		static constexpr bool CanCastFrom() { return typename CanCast<U>::To<T>::value; }

	public:
		T& operator[](size_t index) const
		{
			assert(Data() != nullptr && index < Size());
			return *reinterpret_cast<T*>(reinterpret_cast<long>(Data()) + Stride() * index);
		}
	};
}

template<typename T>
class ArrayView :
	public array_view_helpers::ArrayViewBase<T, ::ArrayView>
{
public:
	explicit ArrayView(T* ptr = nullptr, size_t elements = 0, size_t stride = sizeof(T)) :
		m_p(ptr),
		m_size(elements),
		m_stride(stride)
	{
	}

	T* Data() const { return m_p; }
	size_t Size() const { return m_size; }
	size_t Stride() const { return m_stride; }

	template<typename U, class = std::enable_if_t<CanCastTo<U>()>>
	operator ArrayView<U>() const
	{
		return ArrayView<U>(static_cast<U*>(m_p, m_size, m_stride));
	}

	template<typename U, class = std::enable_if_t<CanCastFrom<U>()>>
	ArrayView& operator=(const ArrayView<U>& rhs)
	{
		m_p = rhs.Data();
		m_size = rhs.Size();
		m_stride = rhs.Stride();

		return This();
	}

private:
	T* m_p;
	size_t m_size;
	size_t m_stride;
};

template<
	size_t Size, size_t Stride,
	template<typename, size_t, size_t> typename ArrView>
class Skipper
{
public:
	template<typename T>
	using Type = ArrView<T, Size, Stride>;
};

template<typename T, size_t size, size_t stride = sizeof(T)>
class StaticArrayView :
	public array_view_helpers::ArrayViewBase<T, typename Skipper<size, stride, ::StaticArrayView>::Type>
{
public:
	explicit StaticArrayView(T* ptr) :
		m_p(ptr)
	{
		assert(m_p != nullptr);
	}

	T* Data() const { return m_p; }
	constexpr size_t Size() const { return size; }
	constexpr size_t Stride() const { return stride; }

	operator ::ArrayView<T>() const
	{
		return ::ArrayView<T>(m_p, size, stride);
	}

	template<typename U, class = std::enable_if_t<CanCastTo<U>()>>
	operator StaticArrayView<U, size, stride>() const
	{
		return StaticArrayView<U, size, stride>(static_cast<U*>(m_p));
	}

	template<typename U, class = std::enable_if_t<CanCastFrom<U>()>>
	StaticArrayView& operator=(const StaticArrayView<U, size, stride>& rhs)
	{
		m_p = rhs.Data();
		return This();
	}

private:
	T* m_p;
};