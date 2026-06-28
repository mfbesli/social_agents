#pragma once

// TODO : change name of this file to coeffs_helper ?
// also : should polynomial::coefficients<,> see NM_minimize and numerical_sim through this file?

#include "polynomial.h"
#include "opers.h"

template <size_t dim, size_t val_dim>
struct cont_helper<polynomial::coefficients<dim, val_dim>> : cont_helper_base<polynomial::coefficients<dim, val_dim>>
{
	typedef polynomial::coefficients<dim, val_dim> cont_type;
	typedef cont_type structure_type; // a special instance of the container type will be used to store the container structure data

	structure_type str;

	static inline structure_type extract(const cont_type&);

	static inline cont_type value_init(const structure_type&);

	static inline auto& access(const cont_helper<polynomial::coefficients<dim, val_dim>>::iterator&);
	static inline const auto& access(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_iterator&);

	static inline auto& project(const cont_helper<polynomial::coefficients<dim, val_dim>>::reference&);
	static inline const auto& project(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_reference&);

	//template <typename Assignment_Rhs>
	//	requires std::assignable_from<polynomial::float_type, Assignment_Rhs>
	//static inline void assign(const cont_type::iterator&, Assignment_Rhs&&);
};

template <size_t dim, size_t val_dim>
inline cont_helper<polynomial::coefficients<dim, val_dim>>::structure_type cont_helper<polynomial::coefficients<dim, val_dim>>::extract(const cont_type& ref)
{
	cont_type ret(ref);

	//for (typename cont_type::const_reference tc : ret)
	//	ret.set_coeff(polynomial::compound_term<dim>{ tc.comp_no(), tc.val().first }, 0); // inefficient, finds the term to which a reference is already present in tc
	ret.reset();

	return ret;
}

template <size_t dim, size_t val_dim>
inline cont_helper<polynomial::coefficients<dim, val_dim>>::cont_type cont_helper<polynomial::coefficients<dim, val_dim>>::value_init(const structure_type& str) { return str; }

template <size_t dim, size_t val_dim>
inline auto& cont_helper<polynomial::coefficients<dim, val_dim>>::access(const cont_helper<polynomial::coefficients<dim, val_dim>>::iterator& pos) { return (*pos).val().second; }

template <size_t dim, size_t val_dim>
inline const auto& cont_helper<polynomial::coefficients<dim, val_dim>>::access(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_iterator& pos) { return (*pos).val().second; }

template <size_t dim, size_t val_dim>
inline auto& cont_helper<polynomial::coefficients<dim, val_dim>>::project(const cont_helper<polynomial::coefficients<dim, val_dim>>::reference& ref) { return ref.val().second; }

template <size_t dim, size_t val_dim>
inline const auto& cont_helper<polynomial::coefficients<dim, val_dim>>::project(const cont_helper<polynomial::coefficients<dim, val_dim>>::const_reference& ref) { return ref.val().second; }

//template <size_t dim, size_t val_dim>
//template <typename Assignment_Rhs>
//	requires std::assignable_from<polynomial::float_type, Assignment_Rhs>
//inline void cont_helper<polynomial::coefficients<dim, val_dim>>::assign(const cont_type::iterator& pos, Assignment_Rhs&& val)
//{
//	//typename cont_type::reference ref = *pos;
//	//ref.val().second = val;
//	(*pos).val().second = val;
//}

