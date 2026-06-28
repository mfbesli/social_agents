#pragma once

#include <array>
using std::size_t;

namespace aug
{
	template <typename Val, size_t dim>
	struct array : public std::array<Val, dim>
	{
	private:
		typedef std::array<Val, dim> base_array;

	public:
		// !!! debugger doesn't use below operators (even though optimizations are disabled) (why?)
		inline Val& operator[](size_t);
		inline constexpr const Val& operator[](size_t) const;
		inline Val& at(size_t);
		inline constexpr const Val& at(size_t) const;
	};
	/*
	template <class Cont>
	struct array_size;

	template <size_t dim, typename Val>
	struct array_size<array<Val, dim>> : std::tuple_size<std::array<Val, dim>> {};*/
}

namespace std
{
	template <typename Val, size_t dim>
	struct tuple_size<aug::array<Val, dim>> : tuple_size<std::array<Val, dim>> {}; // is this safe? (probably, as it is not from type_traits)

	template <size_t I, typename Val, size_t dim>
	struct tuple_element<I, aug::array<Val, dim>> : tuple_element<I - 1, std::array<Val, dim>> {}; // is I - 1 necessary?
}

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::operator[](size_t i) { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::operator[](size_t i) const { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::at(size_t i) { return base_array::at(i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::at(size_t i) const { return base_array::at(i - 1); }

