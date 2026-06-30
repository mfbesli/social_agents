#pragma once

// TODO NEXT (1st) : Comments/descriptions in this file 

#include "aug_array.h"
#include "compound_container.h"
#include "binomial_coeff.h"

#include <unordered_map>
#include <unordered_set>
#include <cmath> // pow
#include <functional> // std::bind, std::cref

namespace polynomial
{
	typedef double float_type;

	typedef int power_type;

	template <size_t dim>
	using point = aug::array<float_type, dim>;

	template <size_t dim>
	using term = aug::array<power_type, dim>;

	template <size_t dim>
	inline power_type degree(const term<dim>&);

	template <size_t dim>
	struct term_hasher
	{
		// desired behaviour : relates each term to a unique number up to the effect of overflow
		size_t operator()(const term<dim>&) const;
	};

	template <size_t dim, size_t val_dim = dim>
	using term_cont = compound_container<val_dim, std::unordered_set<term<dim>, term_hasher<dim>>>;

	template <size_t dim>
	inline float_type power_product(const point<dim>&, const term<dim>&);

	template <size_t dim>
	using compound_term = std::pair<size_t, term<dim>>;

	template <size_t dim, size_t val_dim = dim>
	class coefficients
	{
		using term_coeff_map	= std::unordered_map<term<dim>, float_type, term_hasher<dim>>;
		using coeff_cont		= compound_container<val_dim, term_coeff_map>;

	public:
		using value_type		= coeff_cont::value_type;

		using reference			= coeff_cont::reference;
		using const_reference	= coeff_cont::const_reference;

		using iterator			= coeff_cont::iterator;
		using const_iterator	= coeff_cont::const_iterator;

		static constexpr power_type No_Limit = -1;

	private:
		power_type max_deg, deg_limit;

		coeff_cont coeffs;

	public:
		explicit coefficients(power_type lim = No_Limit); // PROBLEM : polynomial::coefficients<,> satisfy second requirement of sizable_container because of this constructor

		bool set_coeff(const compound_term<dim>&, float_type);
		float_type get_coeff(const compound_term<dim>&) const;

		void clear();
		void reset();

		inline power_type get_max_deg() const;
		inline power_type get_deg_limit() const;

		inline coeff_cont::iterator begin();
		inline coeff_cont::iterator end();
		inline coeff_cont::const_iterator begin() const;
		inline coeff_cont::const_iterator end() const;

		inline size_t size() const;

		inline bool operator==(const coefficients<dim, val_dim>&) const = default;

		point<val_dim> evaluate(const point<dim>&) const;

		void expand(const term_cont<dim, val_dim>&);
		void fuse(const coefficients<dim, val_dim>&);
		coefficients<dim, val_dim> sieve(const term_cont<dim, val_dim>&) const;

		inline auto get_func() const;
	};

	constexpr power_type No_Max = -1;

	template <size_t dim>
	struct function
	{
		coefficients<dim> coeffs;

		term_cont<dim> actives;

		inline coefficients<dim> sieve_acts() const;

		inline bool in_actives(const compound_term<dim>&) const;
		inline void activate(const compound_term<dim>&);

		point<dim> operator()(const point<dim>&, const coefficients<dim>&) const;
		point<dim> operator()(const point<dim>&) const;
	};
}


template <size_t dim>
inline polynomial::power_type polynomial::degree(const term<dim>& t)
{
	power_type s = 0;

	for (const power_type& p : t)
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
		ret += binomial_coeff(sum + i - 1, i);
	}

	return ret;
}

template <size_t dim>
inline polynomial::float_type polynomial::power_product(const point<dim>& pt, const term<dim>& t)
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
bool polynomial::coefficients<dim, val_dim>::set_coeff(const compound_term<dim>& t, float_type c)
{
	power_type deg = degree(t.second);
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
polynomial::float_type polynomial::coefficients<dim, val_dim>::get_coeff(const compound_term<dim>& t) const
{
	if (t.first == 0 || t.first > val_dim)
		return 0; // exception case: no such term can exist

	typename term_coeff_map::const_iterator it = coeffs[t.first].find(t.second);

	if (it == coeffs[t.first].end())
		return 0;
	else
		return (*it).second;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::clear()
{
	for (size_t j = 1; j <= val_dim; j++)
		coeffs[j].clear();

	max_deg = 0;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::reset()
{
	for (reference tc : coeffs)
		tc.val().second = 0;
}

template <size_t dim, size_t val_dim>
inline polynomial::power_type polynomial::coefficients<dim, val_dim>::get_max_deg() const { return max_deg; }

template <size_t dim, size_t val_dim>
inline polynomial::power_type polynomial::coefficients<dim, val_dim>::get_deg_limit() const { return deg_limit; }

template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::iterator polynomial::coefficients<dim, val_dim>::begin() { return coeffs.begin(); }

template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::iterator polynomial::coefficients<dim, val_dim>::end() { return coeffs.end(); }

template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::const_iterator polynomial::coefficients<dim, val_dim>::begin() const { return coeffs.begin(); }

template <size_t dim, size_t val_dim>
inline polynomial::coefficients<dim, val_dim>::coeff_cont::const_iterator polynomial::coefficients<dim, val_dim>::end() const { return coeffs.end(); }

template <size_t dim, size_t val_dim>
inline size_t polynomial::coefficients<dim, val_dim>::size() const { return coeffs.size(); }

template <size_t dim, size_t val_dim>
polynomial::point<val_dim> polynomial::coefficients<dim, val_dim>::evaluate(const point<dim>& pt) const
{
	point<val_dim> ret;

	for (size_t j = 1; j <= val_dim; j++)
	{
		float_type sum = 0.0;

		for (const typename term_coeff_map::value_type& tc : coeffs[j])
			sum += tc.second * power_product(pt, tc.first);

		ret[j] = sum;
	}

	return ret;
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::expand(const term_cont<dim, val_dim>& terms)
{
	for (typename term_cont<dim, val_dim>::const_reference t : terms)
	{
		polynomial::power_type deg = degree(t.val());
		if ((deg_limit == No_Limit || deg <= deg_limit) && coeffs[t.comp_no()].find(t.val()) == coeffs[t.comp_no()].end())
		{
			coeffs[t.comp_no()][t.val()] = {};

			if (deg > max_deg)
				max_deg = deg;
		}
	}
}

template <size_t dim, size_t val_dim>
void polynomial::coefficients<dim, val_dim>::fuse(const coefficients<dim, val_dim>& other)
{
	for (const_reference tc : other)
		set_coeff(compound_term<dim>{ tc.comp_no(), tc.val().first }, tc.val().second);
}

template <size_t dim, size_t val_dim>
polynomial::coefficients<dim, val_dim> polynomial::coefficients<dim, val_dim>::sieve(const term_cont<dim, val_dim>& acts) const
{
	coefficients<dim, val_dim> ret;

	for (typename term_cont<dim, val_dim>::const_reference t : acts)
	{
		compound_term<dim> temp_term{ t };
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
inline bool polynomial::function<dim>::in_actives(const compound_term<dim>& t) const
{
	return !(t.first == 0 || t.first > dim || actives[t.first].find(t.second) == actives[t.first].end());
}

template <size_t dim>
inline void polynomial::function<dim>::activate(const compound_term<dim>& t) { actives[t.first].insert(t.second); }

template <size_t dim>
polynomial::point<dim> polynomial::function<dim>::operator()(const point<dim>& eval_pt, const coefficients<dim>& act_coeffs) const
{
	coefficients<dim> temp_coeffs(coeffs);

	temp_coeffs.fuse(act_coeffs.sieve(actives));

	return temp_coeffs.evaluate(eval_pt);
}

template <size_t dim>
polynomial::point<dim> polynomial::function<dim>::operator()(const point<dim>& eval_pt) const
{
	return coeffs.evaluate(eval_pt);
}

