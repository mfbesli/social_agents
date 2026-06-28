#pragma once

#include "polynomial.h"

namespace van_der_pol
{
	struct poly : public polynomial::coefficients<2> // TODO : reconsider inheritance from polynomial::coefficients<>
	{
		poly(polynomial::float_type mu = 1.0);
	};
}

namespace type_test1_func
{
	struct simple_flat_poly : public polynomial::coefficients<2>
	{
		static constexpr polynomial::float_type K_def = 1.0e5;
		static constexpr polynomial::float_type a_def = 10.0 + K_def / 100.0;
		static constexpr polynomial::float_type b_def = 10.0;

		simple_flat_poly(polynomial::float_type K = K_def, polynomial::float_type a = a_def, polynomial::float_type b = b_def);
	};
}

