#pragma once

// cont_helper<...> template specialization for polynomial::coefficients<dim, val_dim> template class, 
// necessary for the latter to work with several algorithms in the project including those in numerical_sim.h and minimize.h,
// which are #included below to allow access to them from the header that will necessarily be 
// included to use them on polynomial::coefficients<...> objects


#include "polynomial.h"
#include "opers.h"
#include "numerical_sim.h"
#include "minimize.h"

template <size_t dim, size_t val_dim>
struct cont_helper<polynomial::coefficients<dim, val_dim>> : cont_helper_base<polynomial::coefficients<dim, val_dim>>
{
	typedef polynomial::coefficients<dim, val_dim> cont_type;
	typedef cont_type structure_type; // a special instance of the container type will be used to store the container structure data

	structure_type str;

	static inline structure_type extract(const cont_type&);

	static inline cont_type value_init(const structure_type&);

	static inline auto& project(const cont_helper<polynomial::coefficients<dim, val_dim>>::reference&);
	static inline const auto& project(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_reference&);
};

template <size_t dim, size_t val_dim>
inline cont_helper<polynomial::coefficients<dim, val_dim>>::structure_type cont_helper<polynomial::coefficients<dim, val_dim>>::extract(const cont_type& ref)
{
	cont_type ret(ref);
	ret.reset();
	return ret;
}

template <size_t dim, size_t val_dim>
inline cont_helper<polynomial::coefficients<dim, val_dim>>::cont_type cont_helper<polynomial::coefficients<dim, val_dim>>::value_init(const structure_type& str) { return str; }

template <size_t dim, size_t val_dim>
inline auto& cont_helper<polynomial::coefficients<dim, val_dim>>::project(const cont_helper<polynomial::coefficients<dim, val_dim>>::reference& ref) { return ref.val().second; }

template <size_t dim, size_t val_dim>
inline const auto& cont_helper<polynomial::coefficients<dim, val_dim>>::project(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_reference& ref) { return ref.val().second; }

