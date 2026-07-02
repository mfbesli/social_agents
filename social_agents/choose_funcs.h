#pragma once

// Functions that choose a given number of elements (or pairs of elements) from given containers
//	(a) according to given predicates or functions (most_unary, most_binary)
//	(b) randomly (choose, sample_reservoir, sample_pairs_from_distinct_reservoirs, sample_pairs_from_single_reservoir)


#include <vector>
#include <concepts> // std::semiregular
#include <type_traits> // std::remove_cv_t
#include <utility> // std::declval
#include <random>
#include <iterator> // std::advance
#include <cmath> // ceil, sqrt

// function template that returns the greatest element(s) in a given container under projection by a given unary function
template <class Container, class Unary_Function>
std::vector<typename Container::value_type> most_unary(const Container& cont, Unary_Function func)
{
	std::vector<typename Container::value_type> ret;
	if (!cont.empty())
	{
		for (const typename Container::value_type& val : cont)
		{
			if (ret.empty())
			{
				ret.push_back(val);
			}
			else
			{
				auto f_max = func(ret.front()), f_new = func(val);
				if (f_max < f_new)
				{
					ret.clear();
					ret.push_back(val);
				}
				else if (!(f_new < f_max))
				{
					ret.push_back(val);
				}
			}
		}
	}
	return ret;
}

// more efficient specialization of most_unary<> for the case where returned values of func is default constructible and copy assignable
template <class Container, class Unary_Function>
	requires std::semiregular<std::remove_cv_t<decltype(func(std::declval<const typename Container::value_type&>()))>>
std::vector<typename Container::value_type> most_unary<Container, Unary_Function>(const Container& cont, Unary_Function func)
{
	using func_eval_t = std::remove_cv_t<decltype(func(std::declval<const typename Container::value_type&>()))>;

	std::vector<typename Container::value_type> ret;
	if (!cont.empty())
	{
		func_eval_t f_max, f_new;
		for (const typename Container::value_type& val : cont)
		{
			if (ret.empty())
			{
				ret.push_back(val);
				f_max = func(val);
			}
			else
			{
				f_new = func(val);
				if (f_max < f_new)
				{
					ret.clear();
					ret.push_back(val);
					f_max = f_new;
				}
				else if (!(f_new < f_max))
				{
					ret.push_back(val);
				}
			}
		}
	}
	return ret;
}

// function template that returns the greatest element(s) in a given container compared according to a given binary predicate  
template <class Container, class Binary_Predicate>
std::vector<typename Container::value_type> most_binary(const Container& cont, Binary_Predicate pred)
{
	std::vector<typename Container::value_type> ret;
	if (!cont.empty())
	{
		for (const typename Container::value_type& val : cont)
		{
			if (ret.empty())
			{
				ret.push_back(val);
			}
			else if (pred(ret.front(), val))
			{
				ret.clear();
				ret.push_back(val);
			}
			else if (!pred(val, ret.front()))
			{
				ret.push_back(val);
			}
			else
				continue;
		}
	}
	return ret;
}

// function template to randomly choose a single value from a given container
template <class Container>
inline typename Container::value_type choose(const Container& from)
{
	std::random_device generator;
	std::uniform_int_distribution<size_t> rands(0, from.size() - 1);

	typename Container::const_iterator cit = from.begin();
	std::advance(cit, rands(generator));
	return *cit;
}

// function template to randomly choose a given number of values from a given container (reservoir sampling)
template <class Container>
std::vector<typename Container::value_type> sample_reservoir(const Container& cont, size_t num)
{
	std::vector<typename Container::value_type> ret;

	if (cont.size() >= num)
	{
		typename Container::const_iterator i = cont.begin();
		size_t ord;

		for (ord = 0; ord < num; ord++)
			ret.push_back(*i++);

		std::random_device generator;
		while (i != cont.end())
		{
			std::uniform_int_distribution<size_t> rands(0, ord);
			size_t temp = rands(generator);
			if (temp < num)
				ret[temp] = *i;
			i++;
			ord++;
		}
	}

	return ret;
}

// function template to sample pairs from two different reservoirs (one element from each reservoir for each returned pair)
template <class Container1, class Container2>
std::vector<std::pair<typename Container1::value_type, typename Container2::value_type>>
	sample_pairs_from_distinct_reservoirs(const Container1& cont1, const Container2& cont2, size_t num)
{
	std::vector<std::pair<typename Container1::value_type, typename Container2::value_type>> ret;

	size_t s1 = cont1.size();
	size_t s2 = cont2.size();

	if (num <= s1 * s2)
	{
		std::vector<typename Container1::const_iterator> vi1;
		vi1.reserve(s1);
		for (typename Container1::const_iterator i1 = cont1.begin(); i1 != cont1.end(); i1++)
			vi1.push_back(i1);

		std::vector<typename Container2::const_iterator> vi2;
		vi2.reserve(s2);
		for (typename Container2::const_iterator i2 = cont2.begin(); i2 != cont2.end(); i2++)
			vi2.push_back(i2);

		std::vector<size_t> chosen;
		chosen.reserve(num);

		size_t s;
		for (s = 0; s < num; s++)
			chosen.push_back(s);

		std::random_device generator;
		while (s < s1 * s2)
		{
			std::uniform_int_distribution<size_t> rands(0, s);
			size_t temp = rands(generator);
			if (temp < num)
				chosen[temp] = s;
			s++;
		}

		ret.reserve(num);
		for (s = 0; s < num; s++)
		{
			size_t ix1 = chosen[s] % s1;
			size_t ix2 = (chosen[s] - ix1) / s2;

			ret.push_back(std::make_pair(*vi1[ix1], *vi2[ix2]));
		}
	}

	return ret;
}

// function template to sample pairs from a single reservoir
template <class Container>
std::vector<std::pair<typename Container::value_type, typename Container::value_type>>
	sample_pairs_from_single_reservoir(const Container& cont, size_t num)
{
	std::vector<std::pair<typename Container::value_type, typename Container::value_type>> ret;

	size_t siz = cont.size();
	size_t ps = (siz == 0) ? 0 : (siz * (siz - 1)) / 2;

	if (ps >= num)
	{
		std::vector<typename Container::const_iterator> vi;
		vi.reserve(siz);
		for (typename Container::const_iterator i = cont.begin(); i != cont.end(); i++)
			vi.push_back(i);

		std::vector<size_t> chosen;
		chosen.reserve(num);

		size_t s;
		for (s = 1; s <= num; s++)
			chosen.push_back(s);

		std::random_device generator;
		while (s <= ps)
		{
			std::uniform_int_distribution<size_t> rands(0, s - 1);
			size_t temp = rands(generator);
			if (temp < num)
				chosen[temp] = s;
			s++;
		}

		ret.reserve(num);
		for (s = 0; s < num; s++)
		{
			size_t ix2 = size_t(ceil((sqrt(double(8 * chosen[s] + 1)) - 1.0) / 2.0));
			size_t ix1 = chosen[s] - ((ix2 * (ix2 - 1)) / 2) - 1;

			ret.push_back(std::make_pair(*vi[ix1], *vi[ix2]));
		}
	}

	return ret;
}

