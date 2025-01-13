#pragma once

#include "container_traits.h"

#include <vector>
using std::size_t;

namespace aug
{
	template <typename Val>
	struct vector : std::vector<Val>
	{
	private:
		typedef std::vector<Val> base_vector;

	public:
		using base_vector::base_vector;

		vector(const base_vector&);

		inline Val& operator[](size_t);
		inline constexpr const Val& operator[](size_t) const;
		inline Val& at(size_t);
		inline constexpr const Val& at(size_t) const;
	};
}
/*
template <typename Val, class Unary_Func>
aug::vector<Val> unary_operate(const aug::vector<Val>&, Unary_Func);

template <typename Val, class Binary_Func>
aug::vector<Val> binary_operate(const aug::vector<Val>&, const aug::vector<Val>&, Binary_Func);

template <typename Val>
inline Val distance_squared(const aug::vector<Val>& vec_lhs, const aug::vector<Val>& vec_rhs);
*/

template <typename Val>
aug::vector<Val>::vector(const base_vector& b) : base_vector(b) {}

template <typename Val>
inline Val& aug::vector<Val>::operator[](size_t i) { return base_vector::operator[](i - 1); }

template <typename Val>
inline constexpr const Val& aug::vector<Val>::operator[](size_t i) const { return base_vector::operator[](i - 1); }

template <typename Val>
inline Val& aug::vector<Val>::at(size_t i) { return base_vector::at(i - 1); }

template <typename Val>
inline constexpr const Val& aug::vector<Val>::at(size_t i) const { return base_vector::at(i - 1); }
/*
template <typename Val, class Unary_Func>
aug::vector<Val> unary_operate(const aug::vector<Val>& vec, Unary_Func func)
{
	size_t siz = vec.size();
	aug::vector<Val> ret(siz);

	for (size_t i = 1; i <= siz; i++)
		ret[i] = func(vec[i]);

	return ret;
}

template <typename Val, class Binary_Func>
aug::vector<Val> binary_operate(const aug::vector<Val>& vec_lhs, const aug::vector<Val>& vec_rhs, Binary_Func func)
{
	size_t siz;
	if ((siz = vec_lhs.size()) == vec_rhs.size())
	{
		aug::vector<Val> ret(siz);

		for (size_t i = 1; i <= siz; i++)
			ret[i] = func(vec_lhs[i], vec_rhs[i]);

		return ret;
	}
	else
		return aug::vector<Val>();
}

template <typename Val>
inline Val distance_squared(const aug::vector<Val>& vec_lhs, const aug::vector<Val>& vec_rhs)
{
	aug::vector<Val> temp = binary_operate(vec_lhs, vec_rhs, [](Val lhs, Val rhs) { return (lhs - rhs) * (lhs, rhs); });

	Val ret = 0.0;
	for (size_t i = 1; i <= temp.size(); i++)
		ret += temp[i];

	return ret;
}
*/

template <typename Val>
struct cont_structure_helper<aug::vector<Val>>
{
	static inline aug::vector<Val> default_init(size_t);
	static inline aug::vector<Val> explicit_init(size_t);
};

template <typename Val>
inline aug::vector<Val> cont_structure_helper<aug::vector<Val>>::default_init(size_t siz) { return aug::vector<Val>(siz); }

template <typename Val>
inline aug::vector<Val> cont_structure_helper<aug::vector<Val>>::explicit_init(size_t siz) { return aug::vector<Val>(siz); }

