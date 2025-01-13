#pragma once

#include "aug_array.h"

#include <utility>
//#include <functional> // std::reference_wrapper
#include <type_traits>
#include <algorithm>

// should a namespace be used instead of prefix compound_ ? 

template <typename Val>
struct compound_pair : public std::pair<const size_t, Val>
{
	using std::pair<const size_t, Val>::pair;

	template <typename First, typename Second>
	compound_pair(const std::pair<First, Second>&);

	template <typename First, typename Second>
	compound_pair(std::pair<First, Second>&&);

	inline const size_t& comp_no() const { return this->first; }
	inline Val& val() { return this->second; }
	inline const Val& val() const { return this->second; }
};

template <size_t dim, class Container>
class compound_iterator;

template <size_t dim, class Container>
class compound_const_iterator;

template <size_t dim, class Container>
class compound_container : public aug::array_old<dim, Container>
{
public:
	// will these override the inherited iterator, const_iterator, and value_type ?
	typedef compound_iterator<dim, Container> iterator;
	typedef compound_const_iterator<dim, Container> const_iterator;

	typedef compound_pair<typename Container::value_type> value_type;
	typedef compound_pair<typename Container::value_type&> reference; // TODO : use reference_wrapper
	typedef compound_pair<const typename Container::value_type&> const_reference; // is an initial const needed?/**/
	/*typedef compound_pair<std::reference_wrapper<typename Container::value_type>> reference;
	typedef const compound_pair<std::reference_wrapper<const typename Container::value_type>> const_reference;*/

	// these override the inherited begin/end functions :
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	
/*private:
	typedef compound_pair<typename Container::value_type&> arg_ref_type;
	typedef compound_pair<const typename Container::value_type&> const_arg_ref_type;

public:
	iterator find(const_arg_ref_type); // why does const arg_ref_type not work?
	const_iterator find(const_arg_ref_type) const;*/

	iterator find(const_reference);
	const_iterator find(const_reference) const;
};

template <size_t dim, class Container>
class compound_iterator : protected Container::iterator
{
	const compound_container<dim, Container>* parent_cont; // may get invalidated !!
	size_t comp_no;

public:
	typedef Container::iterator base_iterator;

	compound_iterator();
	// is it possible to construct non-const compound_iterator from const base_iterator ?
	compound_iterator(const base_iterator&, const compound_container<dim, Container>&, size_t);
	compound_iterator(base_iterator&&, const compound_container<dim, Container>&, size_t);

	typedef compound_container<dim, Container>::reference dereference_type;

	compound_iterator<dim, Container>& operator++();
	compound_iterator<dim, Container> operator++(int);

	template <class Compound_Iter> // needs to be constrained
	inline bool operator==(const Compound_Iter&) const;
	// TODO : comparison with compound_const_iterator

	// operator compound_const_iterator() const; // is this sufficient for the above TODO ? (IMPORTANT)

	inline dereference_type operator*() const;
};

template <size_t dim, class Container>
class compound_const_iterator : protected Container::const_iterator
{
	const compound_container<dim, Container>* parent_cont; // may get invalidated !!
	size_t comp_no;

public:
	typedef Container::const_iterator base_iterator;

	compound_const_iterator();
	compound_const_iterator(const base_iterator&, const compound_container<dim, Container>&, size_t);
	compound_const_iterator(base_iterator&&, const compound_container<dim, Container>&, size_t);

	typedef compound_container<dim, Container>::const_reference dereference_type;

	compound_const_iterator<dim, Container>& operator++();
	compound_const_iterator<dim, Container> operator++(int);

	template <class Compound_Iter> // needs to be constrained
	inline bool operator==(const Compound_Iter&) const;
	// TODO : comparison with compound_iterator

	inline dereference_type operator*() const;
};


template <typename Val>
template <typename First, typename Second>
compound_pair<Val>::compound_pair(const std::pair<First, Second>& pr) : std::pair<const size_t, Val>(pr) {}

template <typename Val>
template <typename First, typename Second>
compound_pair<Val>::compound_pair(std::pair<First, Second>&& pr) : std::pair<const size_t, Val>(std::move(pr)) {}

template <size_t dim, class Container>
compound_container<dim, Container>::iterator compound_container<dim, Container>::begin()
{
	if (dim == 0)
		return compound_container<dim, Container>::iterator();
	else
		return compound_container<dim, Container>::iterator((*this)[1].begin(), *this, 1);
}

template <size_t dim, class Container>
compound_container<dim, Container>::const_iterator compound_container<dim, Container>::begin() const
{
	if (dim == 0)
		return compound_container<dim, Container>::const_iterator();
	else
		return compound_container<dim, Container>::const_iterator((*this)[1].begin(), *this, 1);
}

template <size_t dim, class Container>
compound_container<dim, Container>::iterator compound_container<dim, Container>::end()
{
	if (dim == 0)
		return compound_container<dim, Container>::iterator();
	else
		return compound_container<dim, Container>::iterator((*this)[dim].end(), *this, dim);
}

template <size_t dim, class Container>
compound_container<dim, Container>::const_iterator compound_container<dim, Container>::end() const
{
	if (dim == 0)
		return compound_container<dim, Container>::const_iterator();
	else
		return compound_container<dim, Container>::const_iterator((*this)[dim].end(), *this, dim);
}

template <class Container>
concept container_with_find = requires
{
	typename Container::value_type;
	typename Container::key_type;
	requires requires (Container cont, typename Container::key_type k)
	{
		cont.find(k);
	};
};

template <class Container>
concept container_with_set_find = container_with_find<Container> && requires
{
	requires std::is_same_v<typename Container::value_type, typename Container::key_type>;
};

template <class Container>
concept container_with_map_find = container_with_find<Container> && requires
{
	typename Container::mapped_type;
	requires std::is_same_v<typename Container::value_type, std::pair<typename Container::key_type, typename Container::mapped_type>>;
};

template <size_t dim, class Container>
compound_container<dim, Container>::iterator compound_container<dim, Container>::find(const_reference v)
{
	if (v.comp_no() == 0 || v.comp_no() > dim)
	{
		return this->end();
	}
	else
	{
		Container& cont_ref = this->at(v.comp_no());

		typename Container::iterator temp_it;

		if constexpr (container_with_set_find<Container>)
		{
			temp_it = cont_ref.find(v.val());
		}
		else if constexpr (container_with_map_find<Container>)
		{
			temp_it = cont_ref.find(v.val().first);
		}
		else
		{
			temp_it = std::find(cont_ref.begin(), cont_ref.end(), v.val());
		}

		if (temp_it == cont_ref.end())
			return this->end();
		else
			return iterator(temp_it, *this, v.comp_no());
	}
}

template <size_t dim, class Container>
compound_container<dim, Container>::const_iterator compound_container<dim, Container>::find(const_reference v) const
{
	if (v.comp_no() == 0 || v.comp_no() > dim)
	{
		return this->end();
	}
	else
	{
		const Container& cont_ref = this->at(v.comp_no());

		typename Container::const_iterator temp_it;

		if constexpr (container_with_set_find<Container>)
		{
			temp_it = cont_ref.find(v.val());
		}
		else if constexpr (container_with_map_find<Container>)
		{
			temp_it = cont_ref.find(v.val().first);
		}
		else
		{
			temp_it = std::find(cont_ref.begin(), cont_ref.end(), v.val());
		}

		if (temp_it == cont_ref.end())
			return this->end();
		else
			return const_iterator(temp_it, *this, v.comp_no());
	}
}

template <size_t dim, class Container>
compound_iterator<dim, Container>::compound_iterator() : base_iterator(), parent_cont(nullptr), comp_no(0) {}

template <size_t dim, class Container>
compound_iterator<dim, Container>::compound_iterator(const base_iterator& i, const compound_container<dim, Container>& p, size_t cn)
	: base_iterator(i), parent_cont(&p), comp_no(cn) {}

template <size_t dim, class Container>
compound_iterator<dim, Container>::compound_iterator(base_iterator&& i, const compound_container<dim, Container>& p, size_t cn)
	: base_iterator(std::move(i)), parent_cont(&p), comp_no(cn) {}

template <size_t dim, class Container>
compound_iterator<dim, Container>& compound_iterator<dim, Container>::operator++()
{
	base_iterator& temp = ++(static_cast<base_iterator&>(*this));

	if (comp_no != dim && temp == (*parent_cont)[comp_no].end())
	{
		comp_no++;
		temp = (*parent_cont)[comp_no].begin();
		// or, *this = compound_iterator<dim, Container>((*parent_cont)[comp_no].begin(), parent_cont, comp_no);
	}

	return *this;
}

template <size_t dim, class Container>
compound_iterator<dim, Container> compound_iterator<dim, Container>::operator++(int)
{
	compound_iterator<dim, Container> ret = *this;

	base_iterator& temp = ++(static_cast<base_iterator&>(*this));

	if (comp_no != dim && temp == (*parent_cont)[comp_no].end())
	{
		comp_no++;
		temp = (*parent_cont)[comp_no].begin();
		// or, *this = compound_iterator<dim, Container>((*parent_cont)[comp_no].begin(), parent_cont, comp_no);
	}

	return ret;
}

template <size_t dim, class Container, class Compound_Iter>
concept Comparable_to_Compound_Iters = std::is_base_of<compound_iterator<dim, Container>, Compound_Iter>::value ||
	std::is_base_of<compound_const_iterator<dim, Container>, Compound_Iter>::value;

template <size_t dim, class Container>
template <class Compound_Iter> 
inline bool compound_iterator<dim, Container>::operator==(const Compound_Iter& other) const
{
	return parent_cont == other.parent_cont && comp_no == other.comp_no && base_iterator::operator==(other); // TODO : guarantee that Compound_Iter's members are accessible
}

template <size_t dim, class Container>
inline compound_iterator<dim, Container>::dereference_type compound_iterator<dim, Container>::operator*() const
{
	return dereference_type{ comp_no, base_iterator::operator*() }; // *(static_cast<const base_iterator&>(*this)) or base_iterator::operator*(); which is better?
}

template <size_t dim, class Container>
compound_const_iterator<dim, Container>::compound_const_iterator() : base_iterator(), parent_cont(nullptr), comp_no(0) {}

template <size_t dim, class Container>
compound_const_iterator<dim, Container>::compound_const_iterator(const base_iterator& i, const compound_container<dim, Container>& p, size_t cn)
	: base_iterator(i), parent_cont(&p), comp_no(cn) {}

template <size_t dim, class Container>
compound_const_iterator<dim, Container>::compound_const_iterator(base_iterator&& i, const compound_container<dim, Container>& p, size_t cn)
	: base_iterator(std::move(i)), parent_cont(&p), comp_no(cn) {}

template <size_t dim, class Container>
compound_const_iterator<dim, Container>& compound_const_iterator<dim, Container>::operator++()
{
	base_iterator& temp = ++(static_cast<base_iterator&>(*this));

	if (comp_no != dim && temp == (*parent_cont)[comp_no].end())
	{
		comp_no++;
		temp = (*parent_cont)[comp_no].begin();
		// or, *this = compound_const_iterator<dim, Container>((*parent_cont)[comp_no].begin(), parent_cont, comp_no);
	}

	return *this;
}

template <size_t dim, class Container>
compound_const_iterator<dim, Container> compound_const_iterator<dim, Container>::operator++(int)
{
	compound_iterator<dim, Container> ret = *this;

	base_iterator& temp = ++(static_cast<base_iterator&>(*this));

	if (comp_no != dim && temp == (*parent_cont)[comp_no].end())
	{
		comp_no++;
		temp = (*parent_cont)[comp_no].begin();
		// or, *this = compound_const_iterator<dim, Container>((*parent_cont)[comp_no].begin(), parent_cont, comp_no);
	}

	return ret;
}

template <size_t dim, class Container>
template <class Compound_Iter>
inline bool compound_const_iterator<dim, Container>::operator==(const Compound_Iter& other) const
{
	return parent_cont == other.parent_cont && comp_no == other.comp_no && base_iterator::operator==(other);
}

template <size_t dim, class Container>
inline compound_const_iterator<dim, Container>::dereference_type compound_const_iterator<dim, Container>::operator*() const
{
	return dereference_type{ comp_no, base_iterator::operator*() }; // *(static_cast<const base_iterator&>(*this)) or base_iterator::operator*(); which is better?
}

