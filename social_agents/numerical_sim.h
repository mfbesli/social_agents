#pragma once

#include "aug_array.h"
#include "opers.h"

template <typename Val, size_t dim>
struct numerical_sim
{
	size_t fineness;

	numerical_sim(size_t f = 100);

	template <class Func>
	aug::array_old<dim, Val> operator()(Func, const aug::array_old<dim, Val>&, Val) const; // 4th order Runge-Kutta method
	// copies Func object, can it operate with functions returned by std::bind efficiently?
	// do we need a universal reference (Func&&)?
};


template <typename Val, size_t dim>
numerical_sim<Val, dim>::numerical_sim(size_t f) : fineness(f) {}

template <typename Val, size_t dim>
template <class Func>
aug::array_old<dim, Val> numerical_sim<Val, dim>::operator()(Func func, const aug::array_old<dim, Val>& init, Val time_step) const
{
	Val h = time_step / Val(fineness);
	aug::array_old<dim, Val> curr = init;

	for (size_t i = 1; i <= fineness; i++)
	{
		aug::array_old<dim, Val> k1 = func(curr);
		aug::array_old<dim, Val> temp = binary_operate(curr, k1, [h](Val lhs, Val rhs) { return lhs + (h / 2.0) * rhs; });
		aug::array_old<dim, Val> k2 = func(temp);
		temp = binary_operate(curr, k2, [h](Val lhs, Val rhs) { return lhs + (h / 2.0) * rhs; });
		aug::array_old<dim, Val> k3 = func(temp);
		temp = binary_operate(curr, k3, [h](Val lhs, Val rhs) { return lhs + h * rhs; });
		aug::array_old<dim, Val> k4 = func(temp);

		curr = binary_operate(curr, k1, [h](Val lhs, Val rhs) { return lhs + (h / 6.0) * rhs; });
		curr = binary_operate(curr, k2, [h](Val lhs, Val rhs) { return lhs + (h / 3.0) * rhs; });
		curr = binary_operate(curr, k3, [h](Val lhs, Val rhs) { return lhs + (h / 3.0) * rhs; });
		curr = binary_operate(curr, k4, [h](Val lhs, Val rhs) { return lhs + (h / 6.0) * rhs; });
	}

	return curr;
}

