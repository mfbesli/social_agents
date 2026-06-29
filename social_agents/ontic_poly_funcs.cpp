#include "ontic_poly_functions.h"

van_der_pol::poly::poly(polynomial::float_type mu)
{
	// C1 == x2
	// C2 == -x1 + mu * x2 - mu * x1^2 * x2
	 
	set_coeff(polynomial::compound_term<2>{ 1, { 0, 1 } }, 1.0);
	set_coeff(polynomial::compound_term<2>{ 2, { 1, 0 } }, -1.0);
	set_coeff(polynomial::compound_term<2>{ 2, { 0, 1 } }, mu);
	set_coeff(polynomial::compound_term<2>{ 2, { 2, 1 } }, -mu);
}

simple_flat_poly::simple_flat_poly(polynomial::float_type K, polynomial::float_type a, polynomial::float_type b)
{
	// C1 == -K * (x1 - a)^3
	// C2 == -x2 + b

	set_coeff(polynomial::compound_term<2>{ 1, { 0, 0 } }, K * a * a * a);
	set_coeff(polynomial::compound_term<2>{ 1, { 1, 0 } }, -3.0 * K * a * a);
	set_coeff(polynomial::compound_term<2>{ 1, { 2, 0 } }, 3.0 * K * a);
	set_coeff(polynomial::compound_term<2>{ 1, { 3, 0 } }, -K);
	set_coeff(polynomial::compound_term<2>{ 2, { 0, 0 } }, b);
	set_coeff(polynomial::compound_term<2>{ 2, { 0, 1 } }, -1.0);
}

