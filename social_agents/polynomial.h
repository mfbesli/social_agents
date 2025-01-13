#pragma once
/*
namespace polynomial
{
	template <size_t dim, size_t val_dim = dim>
	class coefficients;
}

template <size_t dim, size_t val_dim>
void main_poly_util_sup(const polynomial::coefficients<dim, val_dim>&); // for debugging purposes, will be removed
*/

#include "compound_container.h"
#include "opers.h"
#include "binomial_coeff.h"

#include <unordered_map>
#include <unordered_set>
#include <cmath>

namespace polynomial
{
	//typedef long double float_type;
	typedef double float_type;

	typedef int power_type;

	template <size_t dim>
	struct term : aug::array_old<dim, power_type>
	{
		bool operator==(const term<dim>&) const = default;

		inline power_type degree() const;
	};

	template <size_t dim>
	struct term_hasher
	{
		size_t operator()(const term<dim>&) const;
	};

	template <size_t dim, typename Val>
	using term_map = std::unordered_map<term<dim>, Val, term_hasher<dim>>;

	/*
	template <size_t dim, size_t val_dim = dim>
	using term_cont = aug::array<val_dim, std::unordered_set<term<dim>, term_hasher<dim>>>;*/

	template <size_t dim, size_t val_dim = dim>
	using term_cont = compound_container<val_dim, std::unordered_set<term<dim>, term_hasher<dim>>>;

	template <typename Val, size_t dim, size_t val_dim = dim>
	using term_map_cont = aug::array_old<val_dim, term_map<dim, Val>>; // TODO : use compound_container (?)

	template <size_t dim>
	inline float_type power(const aug::array_old<dim, float_type>&, const term<dim>&);

	template <size_t dim>
	using term_refer = std::pair<size_t, term<dim>>; // TODO : resolve if this may be made into compound_pair<term<dim>> for efficiency
	//using term_refer = compound_pair<term<dim>>;

	template <size_t dim, size_t val_dim = dim/**/> // default template argument to be reinstated when main_poly_util_sup(...) is removed
	class coefficients
	{
		typedef term_map<dim, float_type> term_coeff_map;
		typedef compound_container<val_dim, term_coeff_map> coeff_cont; // is private or public better?
		
	public:
		typedef coeff_cont::value_type value_type;
		typedef coeff_cont::reference reference;
		typedef coeff_cont::const_reference const_reference;

	private:
		static constexpr power_type No_Limit = -1;

		power_type max_deg, deg_limit;

		// aug::array<val_dim, term_coeff_map> coeffs; // TODO : use compound_container

		coeff_cont coeffs;

	public:
		coefficients(power_type lim = No_Limit);

		bool set_coeff(const term_refer<dim>&, float_type);
		float_type get_coeff(const term_refer<dim>&) const;

		void clear();
		void expand(const term_cont<dim, val_dim>&);

		inline power_type get_max_deg() const;
		inline power_type get_deg_limit() const;
		// inline const aug::array<val_dim, term_coeff_map>& get_coeff_arr() const; // removed as coefficients<,> can be looped over

		// only read-only looping allowed (to prevent breaking max_deg - coeffs correspondence)
		inline coeff_cont::const_iterator begin() const;
		inline coeff_cont::const_iterator end() const;

		inline size_t size() const;

		aug::array_old<val_dim, float_type> evaluate(const aug::array_old<dim, float_type>&) const;

		void fuse(const coefficients<dim, val_dim>&);
		coefficients<dim, val_dim> sieve(const term_cont<dim, val_dim>&) const;

		inline auto get_func() const;

		//friend void ::main_poly_util_sup<dim, val_dim>(const coefficients<dim, val_dim>&); // for debugging purposes, will be removed
	};

	constexpr power_type No_Max = -1;

	template <size_t dim>
	struct function
	{
		coefficients<dim> coeffs;

		term_cont<dim> actives;

		inline coefficients<dim> sieve_acts() const;
		
		inline bool in_actives(const term_refer<dim>&) const;
		inline void activate(const term_refer<dim>&);

		aug::array_old<dim, float_type> operator()(const aug::array_old<dim, float_type>&, const coefficients<dim>&) const;
		aug::array_old<dim, float_type> operator()(const aug::array_old<dim, float_type>&) const;
	};
}

/*
template <size_t dim, size_t val_dim>
void main_poly_util_sup(const polynomial::coefficients<dim, val_dim>& f) { f.coeffs; } // for debugging purposes, will be removed
*/
template <size_t dim>
inline polynomial::power_type polynomial::term<dim>::degree() const
{
	power_type s = 0;

	for (const power_type& p : *this)
		s += p;

	return s;
}

template <size_t dim>
size_t polynomial::term_hasher<dim>::operator()(const term<dim>& t) const
{
	size_t ret = 1;
	size_t sum = 0;

	for (size_t i = 1; i < dim; i++)
	{
		sum += t[i];
		ret += binomial_coeff(sum + i - 1, i); // overflow behaviour as expected?
	}

	return ret;
}

template <size_t dim>
inline polynomial::float_type polynomial::power(const aug::array_old<dim, float_type>& pt, const term<dim>& t) // TODO : change name to arr_power (?)
{
	float_type ret = 1.0;

	for (size_t i = 1; i <= dim; i++)
	{
		if (t[i] == 0)
			continue;

		ret *= pow(pt[i], t[i]);
	}

	return ret;
}

template <size_t dim, size_t val_dim>
polynomial::coefficients<dim, val_dim>::coefficients(power_type lim) : max_deg(0), deg_limit(lim) {}

template <size_t dim, size_t val_dim>
bool polynomial::coefficients<dim, val_dim>::set_coeff(const term_refer<dim>& t, float_type c)
{
	power_type deg = t.second.degree();
	if (t.first == 0 || t.first > val_dim || (deg_limit != No_Limit && deg > deg_limit))
		return false;
	else
	{
		coeffs[t.first][t.second] = c;

		if (deg > max_deg)
			max_deg = deg;

		return true;
	}
}

template <size_t dim, size_t val_dim>
polynomial::float_type polynomial::coefficients<dim, val_dim>::get_coeff(const term_refer<dim>& t) const
{
	if (t.first == 0 || t.first > val_dim)
		return 0; // exception case

	typename term_coeff_map::const_iterator it = coeffs[t.first].find(t.second);

	if (it == coeffs[t.first].end())
		return 0;
	else
		return (*it).second;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::clear()
{
	for (size_t i = 1; i <= val_dim; i++)
		coeffs[i].clear();

	max_deg = 0;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::expand(const term_cont<dim, val_dim>& terms)
{
	for (typename term_cont<dim, val_dim>::const_reference t : terms)
	{
		polynomial::power_type deg = t.val().degree();
		if ((deg_limit == No_Limit || deg <= deg_limit) && coeffs[t.comp_no()].find(t.val()) == coeffs[t.comp_no()].end())
		{
			coeffs[t.comp_no()][t.val()] = 0.0; // default constructor instead of assignment?
			
			if (deg > max_deg)
				max_deg = deg;
		}
	}
}

template <size_t dim, size_t val_dim>
inline polynomial::power_type polynomial::coefficients<dim, val_dim>::get_max_deg() const { return max_deg; }

template <size_t dim, size_t val_dim>
inline polynomial::power_type polynomial::coefficients<dim, val_dim>::get_deg_limit() const { return deg_limit; }
/*
template <size_t dim, size_t val_dim>
inline const aug::array<val_dim, typename polynomial::coefficients<dim, val_dim>::term_coeff_map>& polynomial::coefficients<dim, val_dim>::get_coeff_arr() const { return coeffs; }
*/
template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::const_iterator polynomial::coefficients<dim, val_dim>::begin() const { return coeffs.begin(); }

template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::const_iterator polynomial::coefficients<dim, val_dim>::end() const { return coeffs.end(); }

template <size_t dim, size_t val_dim>
inline size_t polynomial::coefficients<dim, val_dim>::size() const
{
	size_t ret = 0;
	for (size_t i = 1; i <= val_dim; i++)
		ret += coeffs[i].size();
	return ret;
}

template <size_t dim, size_t val_dim>
aug::array_old<val_dim, polynomial::float_type> polynomial::coefficients<dim, val_dim>::evaluate(const aug::array_old<dim, float_type>& pt) const
{
	aug::array_old<val_dim, float_type> ret;

	for (size_t j = 1; j <= val_dim; j++)
	{
		float_type sum = 0.0;

		for (const typename term_coeff_map::value_type& c : coeffs[j])
			sum += c.second * power(pt, c.first);

		ret[j] = sum;
	}

	return ret;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::fuse(const coefficients<dim, val_dim>& other)
{
	for (const_reference tc : other)
		set_coeff(term_refer<dim>{ tc.comp_no(), tc.val().first }, tc.val().second);
	/*
	for (size_t i = 1; i <= val_dim; i++)
		for (const typename term_coeff_map::value_type& t : other.coeffs[i])
			set_coeff(term_refer<dim>{ i, t.first }, t.second);*/
}

template <size_t dim, size_t val_dim>
polynomial::coefficients<dim, val_dim> polynomial::coefficients<dim, val_dim>::sieve(const term_cont<dim, val_dim>& acts) const
{
	coefficients<dim, val_dim> ret;

	for (typename term_cont<dim, val_dim>::const_reference t : acts)
	{
		term_refer<dim> temp_term{ t.comp_no(), t.val() }; // construct directly from t ?
		ret.set_coeff(temp_term, get_coeff(temp_term));
	}

	return ret;
}

template <size_t dim, size_t val_dim>
inline auto polynomial::coefficients<dim, val_dim>::get_func() const
{
	return std::bind(&polynomial::coefficients<dim, val_dim>::evaluate, std::cref(*this), std::placeholders::_1);
}

template <size_t dim>
inline polynomial::coefficients<dim> polynomial::function<dim>::sieve_acts() const { return coeffs.sieve(actives); }

template <size_t dim>
inline bool polynomial::function<dim>::in_actives(const term_refer<dim>& t) const
{
	/*//if (t.first == 0 || t.first > dim || actives[t.first].find(t.second) == actives[t.first].end())
	if (t.first == 0 || t.first > dim || actives.find(t) == actives.end())
		return false;
	else
		return true;*/
	return !(t.first == 0 || t.first > dim || actives.find(t) == actives.end());
}

template <size_t dim>
inline void polynomial::function<dim>::activate(const term_refer<dim>& t) { actives[t.first].insert(t.second); }

template <size_t dim>
aug::array_old<dim, polynomial::float_type> polynomial::function<dim>::operator()(const aug::array_old<dim, float_type>& eval_pt, const coefficients<dim>& act_coeffs) const
{
	coefficients<dim> temp_coeffs(coeffs);

	temp_coeffs.fuse(act_coeffs.sieve(actives));

	return temp_coeffs.evaluate(eval_pt);
}

template <size_t dim>
aug::array_old<dim, polynomial::float_type> polynomial::function<dim>::operator()(const aug::array_old<dim, float_type>& eval_pt) const
{
	return coeffs.evaluate(eval_pt);
}

