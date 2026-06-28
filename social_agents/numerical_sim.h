#pragma once

#include "aug_array.h"
#include "opers.h"

template <helper_operable_float_container Cont>
struct numerical_sim
{
	using float_type = std::remove_cvref_t<helper_access_t<Cont>>;

	size_t fineness;

	numerical_sim(size_t f = 100);

	template <class Func>
	Cont operator()(Func, const Cont&, float_type) const; // 4th order Runge-Kutta method
	// PROBLEM 010 : copies Func object, can it operate with functions returned by std::bind efficiently?
	// do we need a universal reference (Func&&)?
	// SOLVED : STL method is "if it's expensive to copy, pass it by std::ref()"
	// otherwise, primitive types (including function pointers) may even be cheaper to copy than to use references,
	// and rvalues are directly constructed into the Func parameter, in which case no copying is involved
};


template <helper_operable_float_container Cont>
numerical_sim<Cont>::numerical_sim(size_t f) : fineness(f) {}

template <helper_operable_float_container Cont>
template <class Func>
Cont numerical_sim<Cont>::operator()(Func func, const Cont& init, float_type time_step) const
{
	float_type h = time_step / float_type(fineness);
	Cont curr = init; // requires CopyConstructible Cont

	Cont temp = cont_helper<Cont>::value_init(cont_helper<Cont>::extract(curr)); // requires MoveConstructible Cont
	Cont k1 = temp, k2 = temp, k3 = temp, k4 = temp; // requires CopyConstructible Cont
	for (size_t i = 1; i <= fineness; i++)
	{
		k1 = func(curr);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 2.0) * rhs; },
			curr, k1, temp);
		k2 = func(temp);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 2.0) * rhs; },
			curr, k2, temp);
		k3 = func(temp);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + h * rhs; },
			curr, k3, temp);
		k4 = func(temp);

		// TODO for future : variadic n-ary_operate instead of the four statements below :
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 6.0) * rhs; },
			curr, k1, curr);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 3.0) * rhs; },
			curr, k2, curr);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 3.0) * rhs; },
			curr, k3, curr);
		binary_operate([h](const float_type& lhs, const float_type& rhs) { return lhs + (h / 6.0) * rhs; },
			curr, k4, curr);
	}

	return curr;
}

