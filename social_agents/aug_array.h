#pragma once

// aug::array : array that is indexed from 1 to dim, instead of 0 to dim - 1; for consistency with component numbers used elsewhere in the project


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
		inline Val& operator[](size_t);
		inline constexpr const Val& operator[](size_t) const;
		inline Val& at(size_t);
		inline constexpr const Val& at(size_t) const;
	};
}

namespace std
{
	template <typename Val, size_t dim>
	struct tuple_size<aug::array<Val, dim>> : tuple_size<std::array<Val, dim>> {};

	template <size_t I, typename Val, size_t dim>
	struct tuple_element<I, aug::array<Val, dim>> : tuple_element<I - 1, std::array<Val, dim>> {};
}

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::operator[](size_t i) { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::operator[](size_t i) const { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::at(size_t i) { return base_array::at(i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::at(size_t i) const { return base_array::at(i - 1); }

