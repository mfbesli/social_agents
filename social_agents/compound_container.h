#pragma once

// Compound Container specs :
// - A template class (compound_container<dim, Cont>) to hold multiple instances of a container type that 
//		provides a simple way to iterate over all the elements of the component containers in one loop,
//		used as the underlying container for the classes in polynomial.h
// - An iterator (compound_iterator<dim, Cont>) and a const iterator (compound_const_iterator<dim, Cont>) type allowing such looping
// - A special dereference type (compound_reference<Val>) for these iterators


#include "container_traits.h"

#include <array>
#include <utility> // std::pair

template <typename Val>
struct compound_reference : public std::pair<const size_t, Val&>
{
	using std::pair<const size_t, Val&>::pair;

	inline const size_t& comp_no() const { return this->first; }
	inline Val& val() const { return this->second; }

	// below two operators having the same signature if Val is a reference type, causing a redefinition error, will make sure that it is not a reference type
	const compound_reference<Val>& operator=(const Val& v) const
		requires requires(Val& l, const Val& r) { l = r; }
	{ this->val() = v; return *this; }

	const compound_reference<Val>& operator=(Val&& v) const
		requires requires(Val& l, Val&& r) { l = std::move(r); }
	{ this->val() = std::move(v); return *this; }

	operator Val& () const { return this->val(); }
};

template <typename Val>
using compound_const_reference = compound_reference<const Val>;

template <size_t dim, complete_container Cont>
class compound_iterator;

template <size_t dim, complete_container Cont>
class compound_const_iterator;

template <size_t dim, complete_container Cont>
class compound_container
{
	std::array<Cont, dim> contents;

public:
	using size_type			= typename Cont::size_type;
	using difference_type	= typename Cont::difference_type;

	using value_type		= typename Cont::value_type;

	using reference			= compound_reference<typename Cont::value_type>;
	using const_reference	= compound_const_reference<typename Cont::value_type>;

	using iterator			= compound_iterator<dim, Cont>;
	using const_iterator	= compound_const_iterator<dim, Cont>;

	Cont& operator[](size_t);
	const Cont& operator[](size_t) const;
	Cont& at(size_t);
	const Cont& at(size_t) const;

	size_type size() const;
	bool empty() const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	bool operator==(const compound_container<dim, Cont>&) const = default;
};

template <size_t dim, complete_container Cont>
class compound_const_iterator : protected Cont::const_iterator
{
	friend class compound_container<dim, Cont>;

protected:
	const compound_container<dim, Cont>* parent_cont;
	size_t comp_no;

	using base_iterator		= Cont::const_iterator;

public:
	using size_type			= compound_container<dim, Cont>::size_type;
	using difference_type	= compound_container<dim, Cont>::difference_type;
	using value_type		= compound_container<dim, Cont>::value_type;
	using reference			= compound_container<dim, Cont>::const_reference;
	using pointer			= base_iterator::pointer;
	using iterator_concept	= std::forward_iterator_tag;

	compound_const_iterator();

protected:
	compound_const_iterator(const base_iterator&, const compound_container<dim, Cont>&, size_t);
	compound_const_iterator(base_iterator&&, const compound_container<dim, Cont>&, size_t);

public:
	compound_const_iterator<dim, Cont>& operator++();
	compound_const_iterator<dim, Cont> operator++(int);

	inline bool operator==(const compound_const_iterator<dim, Cont>&) const;

	inline reference operator*() const;
	inline pointer operator->() const;
};

template <size_t dim, complete_container Cont>
class compound_iterator : public compound_const_iterator<dim, Cont>
{
	friend class compound_container<dim, Cont>;

protected:
	using counterpart_const_iterator	= compound_const_iterator<dim, Cont>; // alias of the base class type
	using from_iterator					= Cont::iterator; // non-const counterpart to counterpart_const_iterator::base_iterator (which is Cont::const_iterator)

public:
	using size_type			= compound_container<dim, Cont>::size_type;
	using difference_type	= compound_container<dim, Cont>::difference_type;
	using value_type		= compound_container<dim, Cont>::value_type;
	using reference			= compound_container<dim, Cont>::reference;
	using pointer			= from_iterator::pointer;
	using iterator_concept	= std::forward_iterator_tag;

	compound_iterator();

protected:
	compound_iterator(const from_iterator&, compound_container<dim, Cont>&, size_t);
	compound_iterator(from_iterator&&, compound_container<dim, Cont>&, size_t);

public:
	compound_iterator<dim, Cont>& operator++();
	compound_iterator<dim, Cont> operator++(int);

	inline reference operator*() const;
	inline pointer operator->() const;
};


template <size_t dim, complete_container Cont>
Cont& compound_container<dim, Cont>::operator[](size_t i) { return contents[i - 1]; }

template <size_t dim, complete_container Cont>
const Cont& compound_container<dim, Cont>::operator[](size_t i) const { return contents[i - 1]; }

template <size_t dim, complete_container Cont>
Cont& compound_container<dim, Cont>::at(size_t i) { return contents.at(i - 1); }

template <size_t dim, complete_container Cont>
const Cont& compound_container<dim, Cont>::at(size_t i) const { return contents.at(i - 1); }

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::size_type compound_container<dim, Cont>::size() const
{
	size_t ret = 0;
	for (size_t i = 1; i <= dim; i++)
		ret += contents[i].size();
	return ret;
}

template <size_t dim, complete_container Cont>
bool compound_container<dim, Cont>::empty() const
{
	for (size_t i = 1; i <= dim; i++)
		if (!contents[i].empty())
			return false;
	return true;
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::iterator compound_container<dim, Cont>::begin()
{
	if (dim == 0)
		return iterator();
	else
	{
		for (size_t i = 1; i <= dim; ++i)
			if (!contents[i].empty())
				return iterator(contents[i].begin(), *this, i);
		return iterator(contents[dim].end(), *this, dim); // this->empty() case
	}
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::const_iterator compound_container<dim, Cont>::begin() const
{
	if (dim == 0)
		return const_iterator();
	else
	{
		for (size_t i = 1; i <= dim; ++i)
			if (!contents[i].empty())
				return const_iterator(contents[i].begin(), *this, i);
		return const_iterator(contents[dim].end(), *this, dim); // this->empty() case
	}
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::iterator compound_container<dim, Cont>::end()
{
	if (dim == 0)
		return iterator();
	else
		return iterator(contents[dim].end(), *this, dim);
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::const_iterator compound_container<dim, Cont>::end() const
{
	if (dim == 0)
		return const_iterator();
	else
		return const_iterator(contents[dim].end(), *this, dim);
}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator() : base_iterator(), parent_cont(nullptr), comp_no(0) {}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator(const base_iterator& i, const compound_container<dim, Cont>& p, size_t cn)
	: base_iterator(i), parent_cont(&p), comp_no(cn) {}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator(base_iterator&& i, const compound_container<dim, Cont>& p, size_t cn)
	: base_iterator(std::move(i)), parent_cont(&p), comp_no(cn) {}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>& compound_const_iterator<dim, Cont>::operator++()
{
	base_iterator& bas = ++(static_cast<base_iterator&>(*this));

	if (bas == (*parent_cont)[comp_no].end())
	{
		bool nonempty_not_found = true;
		while (comp_no < dim && (nonempty_not_found = (*parent_cont)[++comp_no].empty()))
			; // intentionally empty loop body

		if (nonempty_not_found)
			bas = (*parent_cont)[dim].end(); // necessarily comp_no == dim in this case
		else
			bas = (*parent_cont)[comp_no].begin();
	}

	return *this;
}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont> compound_const_iterator<dim, Cont>::operator++(int)
{
	compound_iterator<dim, Cont> ret = *this;

	++*this;

	return ret;
}

template <size_t dim, complete_container Cont>
inline bool compound_const_iterator<dim, Cont>::operator==(const compound_const_iterator<dim, Cont>& other) const
{
	return parent_cont == other.parent_cont && comp_no == other.comp_no && base_iterator::operator==(other);
}

template <size_t dim, complete_container Cont>
inline compound_const_iterator<dim, Cont>::reference compound_const_iterator<dim, Cont>::operator*() const
{
	return reference{ comp_no, base_iterator::operator*() };
}

template <size_t dim, complete_container Cont>
inline compound_const_iterator<dim, Cont>::pointer compound_const_iterator<dim, Cont>::operator->() const
{
	return std::pointer_traits<pointer>::pointer_to(base_iterator::operator*());
}

template <size_t dim, complete_container Cont>
compound_iterator<dim, Cont>::compound_iterator() : counterpart_const_iterator() {}

template <size_t dim, complete_container Cont>
compound_iterator<dim, Cont>::compound_iterator(const from_iterator& i, compound_container<dim, Cont>& p, size_t cn)
	: counterpart_const_iterator(i, p, cn) {}

template <size_t dim, complete_container Cont>
compound_iterator<dim, Cont>::compound_iterator(from_iterator&& i, compound_container<dim, Cont>& p, size_t cn)
	: counterpart_const_iterator(std::move(i), p, cn) {}

template <size_t dim, complete_container Cont>
compound_iterator<dim, Cont>& compound_iterator<dim, Cont>::operator++()
{
	++(static_cast<counterpart_const_iterator&>(*this));

	return *this;
}

template <size_t dim, complete_container Cont>
compound_iterator<dim, Cont> compound_iterator<dim, Cont>::operator++(int)
{
	compound_iterator<dim, Cont> ret = *this;

	++*this;

	return ret;
}

template <size_t dim, complete_container Cont>
inline compound_iterator<dim, Cont>::reference compound_iterator<dim, Cont>::operator*() const
{
	return reference{ this->comp_no, const_cast<value_type&>(counterpart_const_iterator::operator*().val()) };
}

template <size_t dim, complete_container Cont>
inline compound_iterator<dim, Cont>::pointer compound_iterator<dim, Cont>::operator->() const
{
	return std::pointer_traits<pointer>::pointer_to(const_cast<value_type&>(counterpart_const_iterator::operator*().val()));
}

