#pragma once

// A function object class implementing :
// Nelder-Mead Minimization Method : algorithm to find a local minimum of a scalar function of a multi-dimensional quantity
// without having access to information on the partial derivatives (gradient) of the function (in contrast with methods like steepest descent)


#include "aug_array.h"
#include "change_func.h"
#include "opers.h"

#include <list>
#include <deque>
#include <utility> // std::pair
#include <iterator> // std::advance
#include <numeric> // std::accumulate

template <helper_operable_float_container Cont>
struct NM_minimize
{
	using float_type = std::remove_cvref_t<helper_access_t<Cont>>;

	template <class Func>
	Cont operator()(Func, const Cont&); // Nelder-Mead minimization
										// parameters: scalar function of which to find a local minimum, container holding the point from which to start the search

	// algorithmic parameters : 
	float_type rho, chi, gamma, sigma; // rho > 0, chi > 1, chi > rho, 0 < gamma < 1, 0 < sigma < 1

	float_type change_threshold, distance_threshold;

	size_t length_considered, min_iter; // these must be > 0

	const change_func_base<float_type>* change_func;

	NM_minimize(const change_func_base<float_type>&, aug::array<float_type, 4> alg_params = aug::array<float_type, 4>{ 1.0, 2.0, 0.5, 0.5 },
		size_t len = 10, size_t its = 11, aug::array<float_type, 2> thresholds = aug::array<float_type, 2>{ 1e-4, 1e-4 });

	// explicitly deleted constructor that binds to rvalue change_func arguments, to prevent change_func becoming a dangling pointer
	NM_minimize(change_func_base<float_type>&&, aug::array<float_type, 4> alg_params = aug::array<float_type, 4>{ 1.0, 2.0, 0.5, 0.5 },
		size_t len = 10, size_t its = 11, aug::array<float_type, 2> thresholds = aug::array<float_type, 2>{ 1e-4, 1e-4 }) = delete;

private:
	size_t siz; // size of the container for the current minimization operation

	cont_helper<Cont> helper; // initialization and unary/binary_operate helper for the current minimization operation

	// private auxiliary functions : 
	inline void clear_helper();

	typedef std::pair<Cont, float_type> pt_val_pair; // struct to hold a point and the result of evaluating the function at that point

	inline Cont find_centroid(const std::list<pt_val_pair>::const_iterator&, const std::list<pt_val_pair>::const_iterator&) const;

	inline float_type distance_squared(const Cont& ct1, const Cont& ct2) const;

	inline static void sort(std::list<pt_val_pair>&);

	inline void add_record(const std::list<pt_val_pair>&, std::list<pt_val_pair>&, std::deque<float_type>&) const;

	template <class Func>
	inline void step(Func, std::list<pt_val_pair>&) const;
};


template <helper_operable_float_container Cont>
NM_minimize<Cont>::NM_minimize(const change_func_base<float_type>& chg_f, aug::array<float_type, 4> alg_params, size_t len, size_t its, aug::array<float_type, 2> thresholds) :
	rho(alg_params[1]), chi(alg_params[2]), gamma(alg_params[3]), sigma(alg_params[4]),
	length_considered(len), min_iter(its), change_threshold(thresholds[1]), distance_threshold(thresholds[2]), change_func(&chg_f) {}

template <helper_operable_float_container Cont>
inline void NM_minimize<Cont>::clear_helper() { helper.str = typename cont_helper<Cont>::structure_type(); }

template <helper_operable_float_container Cont>
inline Cont NM_minimize<Cont>::find_centroid(const std::list<pt_val_pair>::const_iterator& from, const std::list<pt_val_pair>::const_iterator& to) const // range : [from, to)
{
	Cont centroid = helper.value_init(helper.str);

	size_t len = 0;
	for (typename std::list<pt_val_pair>::const_iterator lit = from; lit != to; ++lit, ++len)
	{
		binary_operate([](const float_type& cen, const float_type& pt) { return cen + pt; },
			centroid, lit->first, centroid);
	}

	float_type len_f = float_type(len);

	unary_operate([&len_f](const float_type& f) { return f / len_f; },
		centroid, centroid);

	return centroid;
}

template <helper_operable_float_container Cont>
inline NM_minimize<Cont>::float_type NM_minimize<Cont>::distance_squared(const Cont& ct1, const Cont& ct2) const
{
	Cont temp = helper.value_init(helper.str);
	binary_operate([](float_type lhs, float_type rhs) { return lhs - rhs; },
		ct1, ct2, temp);

	return std::accumulate(temp.begin(), temp.end(), float_type(0),
		[this](const float_type& acc, const cont_helper<Cont>::reference& ref) { auto& v = this->helper.project(ref); return acc + v * v; });
}

template <helper_operable_float_container Cont>
inline void NM_minimize<Cont>::sort(std::list<pt_val_pair>& l)
{
	l.sort([](const pt_val_pair& lhs, const pt_val_pair& rhs) { return lhs.second < rhs.second; }); // a better sorting algorithm for a list that is already mostly sorted?
}

template <helper_operable_float_container Cont>
inline void NM_minimize<Cont>::add_record(const std::list<pt_val_pair>& curr_simplex, std::list<pt_val_pair>& last_simplex, std::deque<float_type>& changes) const
{
	float_type last = last_simplex.front().second;
	float_type curr = curr_simplex.front().second;

	changes.push_back((*change_func)(last, curr));

	if (changes.size() > length_considered)
		changes.pop_front();

	last_simplex = curr_simplex;
}

template <helper_operable_float_container Cont>
template <class Func>
inline void NM_minimize<Cont>::step(Func func, std::list<pt_val_pair>& simplex) const
{
	const typename std::list<pt_val_pair>::iterator first_it = simplex.begin();
	typename std::list<pt_val_pair>::iterator temp_it = simplex.end();
	const typename std::list<pt_val_pair>::iterator worst_it = --temp_it;
	const typename std::list<pt_val_pair>::iterator penultimate_it = --temp_it;

	Cont centroid = find_centroid(first_it, worst_it);

	Cont& worst = worst_it->first;

	Cont reflect = helper.value_init(helper.str);
	binary_operate([this](float_type lhs, float_type rhs) { return (this->rho + 1.0) * lhs - this->rho * rhs; },
		centroid, worst, reflect);
	float_type reflect_fv = func(reflect);

	bool s_accept = false;
	Cont accept;
	float_type accept_fv;
	bool s_shrink = false;
	if (reflect_fv < first_it->second)
	{
		Cont expand = helper.value_init(helper.str);
		binary_operate([this](float_type lhs, float_type rhs) { return (1.0 - this->chi) * lhs + this->chi * rhs; },
			centroid, reflect, expand);
		float_type expand_fv = func(expand);

		s_accept = true;
		if (expand_fv < reflect_fv)
		{
			accept = expand;
			accept_fv = expand_fv;
		}
		else
		{
			accept = reflect;
			accept_fv = reflect_fv;
		}
	}
	else if (reflect_fv < penultimate_it->second)
	{
		s_accept = true;
		accept = reflect;
		accept_fv = reflect_fv;
	}
	else if (reflect_fv < worst_it->second)
	{
		Cont outside_contract = helper.value_init(helper.str);
		binary_operate([this](float_type lhs, float_type rhs) { return (1.0 - this->gamma) * lhs + this->gamma * rhs; },
			centroid, reflect, outside_contract);
		float_type outside_contract_fv = func(outside_contract);

		if (outside_contract_fv < reflect_fv)
		{
			s_accept = true;
			accept = outside_contract;
			accept_fv = outside_contract_fv;
		}
		else
			s_shrink = true;
	}
	else
	{
		Cont inside_contract = helper.value_init(helper.str);
		binary_operate([this](float_type lhs, float_type rhs) { return (1.0 - this->gamma) * lhs + this->gamma * rhs; },
			centroid, worst, inside_contract);
		float_type inside_contract_fv = func(inside_contract);

		if (inside_contract_fv < worst_it->second)
		{
			s_accept = true;
			accept = inside_contract;
			accept_fv = inside_contract_fv;
		}
		else
			s_shrink = true;
	}

	if (s_accept)
	{
		worst_it->first = accept;
		worst_it->second = accept_fv;
	}

	if (s_shrink)
	{
		typename std::list<pt_val_pair>::iterator it = first_it;
		for (++it; it != simplex.end(); ++it)
		{
			binary_operate([this](float_type lhs, float_type rhs) { return (1.0 - this->sigma) * lhs + this->sigma * rhs; },
				first_it->first, it->first, it->first);
			it->second = func(it->first);
		}
	}

	sort(simplex);
}

template <helper_operable_float_container Cont>
template <class Func>
Cont NM_minimize<Cont>::operator()(Func func, const Cont& init)
{
	siz = init.size();
	if (siz == 0)
		return init;

	if (min_iter < length_considered)
		min_iter = length_considered;

	helper.str = helper.extract(init);

	std::list<pt_val_pair> curr_simplex;
	curr_simplex.push_back(pt_val_pair{ init, func(init) });
	for (size_t a = 0; a < siz; a++)
	{
		curr_simplex.push_back(pt_val_pair{ init, 0.0 });

		typename Cont::iterator it = curr_simplex.back().first.begin();
		std::advance(it, a);

		auto& v = helper.project(*it);
		if (v == 0.0)
			v = 0.00025;
		else
			v *= 1.05;

		curr_simplex.back().second = func(curr_simplex.back().first);
	}
	sort(curr_simplex);

	float_type simplex_size = float_type(curr_simplex.size()); // must be siz + 1

	Cont init_centroid = find_centroid(curr_simplex.begin(), curr_simplex.end());

	float_type avg_init_distance_squared = std::accumulate(curr_simplex.begin(), curr_simplex.end(), float_type(0),
		[&](const float_type& acc, const pt_val_pair& pt_val) { return acc + this->distance_squared(init_centroid, pt_val.first); }
		) / simplex_size;

	std::list<pt_val_pair> last_simplex = curr_simplex;
	std::deque<float_type> changes;

	size_t iter = 0;

	while (true)
	{
		step(func, curr_simplex);

		add_record(curr_simplex, last_simplex, changes);

		iter++;

		if (iter >= min_iter)
		{
			bool s_converge = true;
			for (const float_type& v : changes)
				if (v > change_threshold)
				{
					s_converge = false;
					break;
				}

			if (s_converge)
				break;

			Cont centroid = find_centroid(curr_simplex.begin(), curr_simplex.end());

			bool s_approach = true;
			for (const pt_val_pair& pt_val : curr_simplex)
				if (distance_squared(centroid, pt_val.first) / avg_init_distance_squared > distance_threshold * distance_threshold)
				{ // keep in mind that the relative precision specified by the value of distance_threshold is not independent of the number of dimensions (siz)
					s_approach = false;
					break;
				}

			if (s_approach)
				break;
		}
	}

	clear_helper();
	siz = 0;

	return curr_simplex.front().first;
}

