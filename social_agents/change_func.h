#pragma once

#include <cmath>

template <typename Flt>
struct change_func_base
{
	virtual Flt operator()(Flt, Flt) const = 0;

	virtual ~change_func_base() = default;
};

template <typename Flt>
struct diff_rate_func : public change_func_base<Flt>
{
	Flt operator()(Flt before, Flt after) const { return fabs(after - before) / fabs(before); }
};

template <typename Flt>
struct exp_chg_func : public change_func_base<Flt>
{
	Flt gamma;

	exp_chg_func(Flt gamma_ = 1.0) : gamma(gamma_) {}

	Flt operator()(Flt before, Flt after) const { return expm1(fabs(gamma) * fabs(after - before)); }
};

