#pragma once

// - A function object base class to implement :
//		Finite Difference Method : algorithm to approximate the time derivative of a multi-dimensional function at a point
//		given function values at fixed time distances (nodes) to the point
// 
// - Classes derived from this base class implementing the method with different coefficients,
//		which are determined by the number and positions of nodes


#include "polynomial.h"
#include "opers.h"

#include <deque>

template <size_t dim>
struct fdc_base
{
	virtual polynomial::point<dim> operator()(const std::deque<polynomial::point<dim>>&, polynomial::float_type) const = 0;
												// parameters: deque of points specifying the nodes, time step

	virtual constexpr size_t length() const = 0;

	virtual ~fdc_base() = default;
};

template <size_t dim, size_t fdc_len = 5>
struct backward_fdc : public fdc_base<dim> // finite difference coefficients (FDC) with backward node positions
{										   // (generic order (fdc_len) case not implemented yet)
	virtual polynomial::point<dim> operator()(const std::deque<polynomial::point<dim>>&, polynomial::float_type) const;

	constexpr size_t length() const;
};

template <size_t dim>
struct backward_fdc<dim, 3> : public fdc_base<dim> // 3rd order backward FDC
{
	virtual polynomial::point<dim> operator()(const std::deque<polynomial::point<dim>>&, polynomial::float_type) const;

	constexpr size_t length() const { return 4; }
};

template <size_t dim>
struct backward_fdc<dim, 5> : public fdc_base<dim> // 5th order backward FDC
{
	virtual polynomial::point<dim> operator()(const std::deque<polynomial::point<dim>>&, polynomial::float_type) const;

	constexpr size_t length() const { return 6; }
};


template <size_t dim>
polynomial::point<dim> backward_fdc<dim, 3>::operator()(const std::deque<polynomial::point<dim>>& d, polynomial::float_type time_step) const
{
	constexpr size_t len = 4;//this->length();
	size_t siz = d.size();

	aug::array<polynomial::float_type, len> fdc_factors = { -2.0, 9.0, -18.0, 11.0 }; 
	unary_operate([time_step](polynomial::float_type c) { return c / (6.0 * time_step); },
		fdc_factors, fdc_factors);

	polynomial::point<dim> ret = {};

	// size check? (siz >= len must hold)

	for (int i = 0; i < len; i++)
		binary_operate([&len, &fdc_factors, i](polynomial::float_type lhs, polynomial::float_type rhs) { return lhs + fdc_factors[len - i] * rhs; },
			ret, d[siz - 1 - i], ret);

	return ret;
}

template <size_t dim>
polynomial::point<dim> backward_fdc<dim, 5>::operator()(const std::deque<polynomial::point<dim>>& d, polynomial::float_type time_step) const
{
	constexpr size_t len = 6;//this->length();
	size_t siz = d.size();

	aug::array<polynomial::float_type, len> fdc_factors = { -12.0, 75.0, -200.0, 300.0, -300.0, 137.0 };
	unary_operate([time_step](polynomial::float_type c) { return c / (60.0 * time_step); },
		fdc_factors, fdc_factors);

	polynomial::point<dim> ret = {};

	// size check? (siz >= len must hold)

	for (int i = 0; i < len; i++)
		binary_operate([&len, &fdc_factors, i](polynomial::float_type lhs, polynomial::float_type rhs) { return lhs + fdc_factors[len - i] * rhs; },
			ret, d[siz - 1 - i], ret);

	return ret;
}

