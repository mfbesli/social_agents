#pragma once

#include "polynomial.h"

#include <deque>

template <size_t dim>
struct fdc_base
{
	virtual aug::array_old<dim, polynomial::float_type> operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>&, polynomial::float_type) const = 0;

	virtual constexpr size_t length() const = 0; // is this really meaningful?

	virtual ~fdc_base() = default;
};

template <size_t dim, size_t fdc_len = 5>
struct backwards_fdc : public fdc_base<dim>
{
	aug::array_old<dim, polynomial::float_type> operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>&, polynomial::float_type) const;

	constexpr size_t length() const;
};

template <size_t dim>
struct backwards_fdc<dim, 3> : public fdc_base<dim>
{
	aug::array_old<dim, polynomial::float_type> operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>&, polynomial::float_type) const;

	constexpr size_t length() const { return 4; }
};

template <size_t dim>
struct backwards_fdc<dim, 5> : public fdc_base<dim>
{
	aug::array_old<dim, polynomial::float_type> operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>&, polynomial::float_type) const;

	constexpr size_t length() const { return 6; }
};


template <size_t dim>
aug::array_old<dim, polynomial::float_type> backwards_fdc<dim, 3>::operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>& d, polynomial::float_type time_step) const
{
	constexpr size_t len = 4;//this->length();
	size_t siz = d.size();

	aug::array_old<len, polynomial::float_type> fdc_factors = { -2.0, 9.0, -18.0, 11.0 };
	fdc_factors = unary_operate(fdc_factors, [time_step](polynomial::float_type c) { return c / (6.0 * time_step); });

	aug::array_old<dim, polynomial::float_type> ret;
	for (size_t i = 1; i <= dim; i++)
		ret[i] = 0.0;

	// size check? (siz >= 5 must hold)

	for (int i = 0; i < len; i++)
		ret = binary_operate(ret, d[siz - 1 - i], [this, &fdc_factors, i](polynomial::float_type lhs, polynomial::float_type rhs)
			{ return lhs + fdc_factors[len - i] * rhs; });

	return ret;
}

template <size_t dim>
aug::array_old<dim, polynomial::float_type> backwards_fdc<dim, 5>::operator()(const std::deque<aug::array_old<dim, polynomial::float_type>>& d, polynomial::float_type time_step) const
{
	constexpr size_t len = 6;//this->length();
	size_t siz = d.size();

	aug::array_old<len, polynomial::float_type> fdc_factors = { -12.0, 75.0, -200.0, 300.0, -300.0, 137.0 };
	fdc_factors = unary_operate(fdc_factors, [time_step](polynomial::float_type c) { return c / (60.0 * time_step); });

	aug::array_old<dim, polynomial::float_type> ret;
	for (size_t i = 1; i <= dim; i++)
		ret[i] = 0.0;

	// size check? (siz >= 5 must hold)

	for (int i = 0; i < len; i++)
		ret = binary_operate(ret, d[siz - 1 - i], [&len, &fdc_factors, i](polynomial::float_type lhs, polynomial::float_type rhs)
			{ return lhs + fdc_factors[len - i] * rhs; });

	return ret;
}
/*
template <size_t dim>
static constexpr size_t backwards_fdc<dim, 5>::length() const { return 6; }
*/
