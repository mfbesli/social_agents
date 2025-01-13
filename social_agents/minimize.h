#pragma once

#include "container_traits.h"
#include "aug_array.h"
#include "change_func.h"
#include "opers.h"

#include <list>
#include <deque>
#include <utility> // std::pair
#include <iterator> // std::advance

// TODO : should be sequence container instead of indexable
template <class Cont>
concept float_seq_container = sequence_container<Cont> && std::is_floating_point_v<typename Cont::value_type>;

template <float_seq_container Cont>
struct NM_minimize
{
	typedef Cont::value_type float_type;

	float_type rho, chi, gamma, sigma; // rho > 0, chi > 1, chi > rho, 0 < gamma < 1, 0 < sigma < 1

	size_t length_considered, min_iter; // these must be > 0

	float_type change_threshold, distance_threshold;

	const change_func_base<float_type>* /*const*/ change_func;

	NM_minimize(const change_func_base<float_type>&, aug::array_old<4, float_type> alg_params = aug::array_old<4, float_type>{ 1.0, 2.0, 0.5, 0.5 },
		size_t len = 10, size_t its = 11, aug::array_old<2, float_type> thresholds = aug::array_old<2, float_type>{ 1e-4, 1e-4 });

private:
	size_t siz; // size of the container for the current minimization operation

	struct {
		cont_structure_helper<Cont> helper;
		cont_structure_helper<Cont>::structure_type str;
	} cont_str; // initialization and unary/binary_operate helper for the current minimization operation

	typedef std::pair<Cont, float_type> pt_val_pair; // struct to hold a point and the result of evaluating the function at that point

	inline Cont find_centroid(const std::list<pt_val_pair>::const_iterator&, const std::list<pt_val_pair>::const_iterator&) const;

	inline static void sort(std::list<pt_val_pair>&);

	//inline void add_record(const std::list<pt_val_pair>&, std::deque<std::list<pt_val_pair>>&, std::deque<float_type>&) const; // no need to keep records in this implementation
	inline void add_record(const std::list<pt_val_pair>&, std::list<pt_val_pair>&, std::deque<float_type>&) const;

	template <class Func>
	inline void step(Func, std::list<pt_val_pair>&) const;

public:
	template <class Func>
	Cont operator()(Func, const Cont&); // Nelder-Mead minimization
};

template <class Cont>
	requires container<Cont> && std::is_floating_point_v<typename Cont::value_type>
typename Cont::value_type distance_squared(const Cont& ct1, const Cont& ct2) // move to another file (e.g., together with binomial_coeff)?
{
	typedef Cont::value_type float_type;

	Cont temp = binary_operate(ct1, ct2, [](float_type lhs, float_type rhs) { return lhs - rhs; });

	float_type ret = 0.0;
	for (const float_type& f : temp)
		ret += f * f;

	return ret;
}


template <float_seq_container Cont>
NM_minimize<Cont>::NM_minimize(const change_func_base<float_type>& chg_f, aug::array_old<4, float_type> alg_params, size_t len, size_t its, aug::array_old<2, float_type> thresholds) :
	rho(alg_params[1]), chi(alg_params[2]), gamma(alg_params[3]), sigma(alg_params[4]),
	length_considered(len), min_iter(its), change_threshold(thresholds[1]), distance_threshold(thresholds[2]), change_func(&chg_f) {}

template <float_seq_container Cont>
inline Cont NM_minimize<Cont>::find_centroid(const std::list<pt_val_pair>::const_iterator& from, const std::list<pt_val_pair>::const_iterator& to) const // range : [from, to)
{
	Cont centroid = cont_structure_helper<Cont>::explicit_init(siz);
	
	size_t len = 0;
	for (typename std::list<pt_val_pair>::const_iterator lit = from; lit != to; ++lit, len++)
	{
		typename Cont::const_iterator cit = lit->first.begin();
		for (float_type& v : centroid)
			v += *cit++;
	}

	float_type len_f = float_type(len);
	for (float_type& v : centroid)
		v /= len_f;

	return centroid;
}

template <float_seq_container Cont>
inline void NM_minimize<Cont>::sort(std::list<pt_val_pair>& l)
{
	l.sort([](const pt_val_pair& lhs, const pt_val_pair& rhs) { return lhs.second < rhs.second; }); // is there a better sorting method (for a list that is already mostly sorted)?
}

template <float_seq_container Cont>
inline void NM_minimize<Cont>::add_record(const std::list<pt_val_pair>& curr_simplex, std::list<pt_val_pair>& last_simplex, std::deque<float_type>& changes) const
{
	float_type last = last_simplex.front().second;
	float_type curr = curr_simplex.front().second;

	changes.push_back((*change_func)(last, curr));

	if (changes.size() > length_considered)
		changes.pop_front();

	last_simplex = curr_simplex;
}

/*template <class Cont>
inline void NM_minimize<Cont>::add_record(const std::list<pt_val_pair>& curr_simplex, std::deque<std::list<pt_val_pair>>& records, std::deque<float_type>& changes) const
{
	float_type before = records.back().front().second;

	records.push_back(curr_simplex);

	float_type curr = curr_simplex.front().second;
	changes.push_back((*change_func)(before, curr));

	if (records.size() > length_considered + 1)
	{
		records.pop_front();
		changes.pop_front();
	}
}*/

template <float_seq_container Cont>
template <class Func>
inline void NM_minimize<Cont>::step(Func func, std::list<pt_val_pair>& simplex) const
{
	const typename std::list<pt_val_pair>::iterator first_it = simplex.begin();
	typename std::list<pt_val_pair>::iterator temp_it = simplex.end();
	const typename std::list<pt_val_pair>::iterator worst_it = --temp_it;
	const typename std::list<pt_val_pair>::iterator penultimate_it = --temp_it;

	Cont centroid = find_centroid(first_it, worst_it);

	/*aug::vector<float_type> centroid(siz);
	for (size_t i = 1; i <= siz; i++)
		centroid[i] = 0.0;

	typename std::list<pt_val_pair>::iterator it;
	for (it = first_it; it != worst_it; it++)
		for (size_t i = 1; i <= siz; i++)
			centroid[i] += (it->first)[i];

	float_type simplex_siz = float_type(simplex.size()); // must be siz + 1
	centroid = unary_operate(centroid, [simplex_siz](float_type v) { return v / (simplex_siz - 1.0); });*/

	Cont& worst = worst_it->first;

	Cont reflect = binary_operate(centroid, worst,
		[this](float_type lhs, float_type rhs) { return (this->rho + 1.0) * lhs - this->rho * rhs; });
	float_type reflect_fv = func(reflect);

	bool s_accept = false;
	Cont accept;
	float_type accept_fv;
	bool s_shrink = false;
	if (reflect_fv < first_it->second)
	{
		Cont expand = binary_operate(centroid, reflect,
			[this](float_type lhs, float_type rhs) { return (1.0 - this->chi) * lhs + this->chi * rhs; });
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
		Cont outside_contract = binary_operate(centroid, reflect,
			[this](float_type lhs, float_type rhs) { return (1.0 - this->gamma) * lhs + this->gamma * rhs; });
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
		Cont inside_contract = binary_operate(centroid, worst,
			[this](float_type lhs, float_type rhs) { return (1.0 - this->gamma) * lhs + this->gamma * rhs; });
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

	if (s_shrink) // is s_shrink unnecessary?
	{
		typename std::list<pt_val_pair>::iterator it = first_it;
		for (++it; it != simplex.end(); ++it)
		{
			it->first = binary_operate(first_it->first, it->first,
				[this](float_type lhs, float_type rhs) { return (1.0 - this->sigma) * lhs + this->sigma * rhs; });
			it->second = func(it->first);
		}
	}

	sort(simplex);
}

template <float_seq_container Cont>
template <class Func>
Cont NM_minimize<Cont>::operator()(Func func, const Cont& init)
{
	siz = init.size();
	if (siz == 0)
		return init;

	if (min_iter < length_considered)
		min_iter = length_considered;

	std::list<pt_val_pair> curr_simplex;
	curr_simplex.push_back(pt_val_pair{ init, func(init) });
	for (size_t a = 0; a < siz; a++)
	{
		Cont temp_pt = init;

		typename Cont::iterator it = temp_pt.begin();
		std::advance(it, a);

		if (*it == 0.0)
			*it = 0.00025;
		else
			*it *= 1.05;

		curr_simplex.push_back(pt_val_pair{ temp_pt, func(temp_pt) });
	}
	sort(curr_simplex);

	float_type simplex_size = float_type(curr_simplex.size()); // must be siz + 1

	/*std::list<pt_val_pair> curr_simplex;
	curr_simplex.push_back(pt_val_pair{ init, func(init) });
	for (size_t i = 1; i <= siz; i++)
	{
		curr_simplex.push_back(pt_val_pair{ init, 0.0 });

		pt_val_pair& temp = curr_simplex.back();

		if (temp.first[i] == 0.0)
			temp.first[i] = 0.00025;
		else
			temp.first[i] *= 1.05;

		temp.second = func(temp.first);
	}
	sort(curr_simplex);*/

	Cont init_centroid = find_centroid(curr_simplex.begin(), curr_simplex.end());

	/*aug::vector<float_type> init_centroid(siz);
	for (size_t i = 1; i <= siz; i++)
		init_centroid[i] = 0.0;

	for (const pt_val_pair& pt_val : curr_simplex)
		for (size_t i = 1; i <= siz; i++)
			init_centroid[i] += (pt_val.first)[i];

	float_type simplex_siz = float_type(curr_simplex.size()); // must be siz + 1
	init_centroid = unary_operate(init_centroid, [simplex_siz](float_type v) { return v / simplex_siz; });*/

	float_type avg_init_distance_squared = 0.0;
	for (const pt_val_pair& pt_val : curr_simplex)
		avg_init_distance_squared += distance_squared(init_centroid, pt_val.first);
	avg_init_distance_squared /= simplex_size;

	// std::deque<std::list<pt_val_pair>> records; // is keeping records of recent simplices beneficial?
	std::list<pt_val_pair> last_simplex = curr_simplex;
	std::deque<float_type> changes;

	// records.push_back(curr_simplex);

	size_t iter = 0;

	bool s_assure = false;
	while (true)
	{
		step(func, curr_simplex);

		//add_record(curr_simplex, records, changes);
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

			/*aug::vector<float_type> centroid(siz);
			for (size_t i = 1; i <= siz; i++)
				centroid[i] = 0.0;

			for (const pt_val_pair& pt_val : curr_simplex)
				for (size_t i = 1; i <= siz; i++)
					centroid[i] += (pt_val.first)[i];

			simplex_siz = float_type(curr_simplex.size()); // must be siz + 1
			centroid = unary_operate(centroid, [simplex_siz](float_type v) { return v / simplex_siz; });*/

			bool s_approach = true;
			for (const pt_val_pair& pt_val : curr_simplex)
				if (distance_squared(centroid, pt_val.first) / avg_init_distance_squared > distance_threshold * distance_threshold) // keep in mind that the relative precision specified by the value of distance_threshold is not independent of the number of dimensions (siz) 
				{
					s_approach = false;
					break;
				}

			if (s_approach)
				break;
		}
	}

	return curr_simplex.front().first;
}

