#pragma once

#include <array>
using std::size_t;

namespace aug
{
	template <size_t dim, typename Val>
	struct array_old : public std::array<Val, dim>
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
	struct array_size<array<dim, Val>> : std::tuple_size<std::array<Val, dim>> {};*/
}

namespace std
{
	template <size_t dim, typename Val>
	struct tuple_size<aug::array_old<dim, Val>> : tuple_size<std::array<Val, dim>> {}; // is this safe? (probably, as it is not from type_traits)

	template <size_t I, size_t dim, typename Val>
	struct tuple_element<I, aug::array_old<dim, Val>> : tuple_element<I - 1, std::array<Val, dim>> {}; // is I - 1 necessary?
}
/*
template <size_t dim, typename Val, class Unary_Func>
aug::array<dim, Val> unary_operate(const aug::array<dim, Val>&, Unary_Func);

template <size_t dim, typename Val, class Binary_Func>
aug::array<dim, Val> binary_operate(const aug::array<dim, Val>&, const aug::array<dim, Val>&, Binary_Func);

template <size_t dim, typename Val>
inline Val distance_squared(const aug::array<dim, Val>& arr_lhs, const aug::array<dim, Val>& arr_rhs);
*/

template <size_t dim, typename Val>
inline Val& aug::array_old<dim, Val>::operator[](size_t i) { return base_array::operator[](i - 1); }

template <size_t dim, typename Val>
inline constexpr const Val& aug::array_old<dim, Val>::operator[](size_t i) const { return base_array::operator[](i - 1); }

template <size_t dim, typename Val>
inline Val& aug::array_old<dim, Val>::at(size_t i) { return base_array::at(i - 1); }

template <size_t dim, typename Val>
inline constexpr const Val& aug::array_old<dim, Val>::at(size_t i) const { return base_array::at(i - 1); }
/*
template <size_t dim, typename Val, class Unary_Func>
aug::array<dim, Val> unary_operate(const aug::array<dim, Val>& arr, Unary_Func func)
{
	aug::array<dim, Val> ret;

	for (size_t i = 1; i <= dim; i++)
		ret[i] = func(arr[i]);

	return ret;
}

template <size_t dim, typename Val, class Binary_Func>
aug::array<dim, Val> binary_operate(const aug::array<dim, Val>& arr_lhs, const aug::array<dim, Val>& arr_rhs, Binary_Func func)
{
	aug::array<dim, Val> ret;

	for (size_t i = 1; i <= dim; i++)
		ret[i] = func(arr_lhs[i], arr_rhs[i]);

	return ret;
}

template <size_t dim, typename Val>
inline Val distance_squared(const aug::array<dim, Val>& arr_lhs, const aug::array<dim, Val>& arr_rhs)
{
	aug::array<dim, Val> temp = binary_operate(arr_lhs, arr_rhs, [](Val lhs, Val rhs) { return (lhs - rhs) * (lhs, rhs); });

	Val ret = 0.0;
	for (size_t i = 1; i <= dim; i++)
		ret += temp[i];

	return ret;
}
*/


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
	struct array_size<array<dim, Val>> : std::tuple_size<std::array<Val, dim>> {};*/
}

namespace std
{
	template <typename Val, size_t dim>
	struct tuple_size<aug::array<Val, dim>> : tuple_size<std::array<Val, dim>> {}; // is this safe? (probably, as it is not from type_traits)

	template <size_t I, typename Val, size_t dim>
	struct tuple_element<I, aug::array<Val, dim>> : tuple_element<I - 1, std::array<Val, dim>> {}; // is I - 1 necessary?
}
/*
template <size_t dim, typename Val, class Unary_Func>
aug::array<dim, Val> unary_operate(const aug::array<dim, Val>&, Unary_Func);

template <size_t dim, typename Val, class Binary_Func>
aug::array<dim, Val> binary_operate(const aug::array<dim, Val>&, const aug::array<dim, Val>&, Binary_Func);

template <size_t dim, typename Val>
inline Val distance_squared(const aug::array<dim, Val>& arr_lhs, const aug::array<dim, Val>& arr_rhs);
*/

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::operator[](size_t i) { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::operator[](size_t i) const { return base_array::operator[](i - 1); }

template <typename Val, size_t dim>
inline Val& aug::array<Val, dim>::at(size_t i) { return base_array::at(i - 1); }

template <typename Val, size_t dim>
inline constexpr const Val& aug::array<Val, dim>::at(size_t i) const { return base_array::at(i - 1); }

