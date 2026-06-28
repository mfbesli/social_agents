#pragma once

// NEWER : greatly improved version of compound_container etc. (with newer iterator design : iterator inheriting from const_iterator)

#include "aug_array.h"
#include "container_traits.h"

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
class compound_container : protected aug::array<Cont, dim>
{
	using base_array = aug::array<Cont, dim>;

public:
	using size_type			= typename Cont::size_type;
	using difference_type	= typename Cont::difference_type;

	using value_type		= typename Cont::value_type;

	using reference			= compound_reference<typename Cont::value_type>;
	using const_reference	= compound_const_reference<typename Cont::value_type>;

	using iterator			= compound_iterator<dim, Cont>;
	using const_iterator	= compound_const_iterator<dim, Cont>;

	using base_array::operator[];
	using base_array::at;

	size_type size() const;
	bool empty() const;

	// these override the inherited begin/end functions :
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	bool operator==(const compound_container<dim, Cont>&) const = default;

	// cbegin(), cend() // no need for now

	// functions specializing find() according to the type of Cont (probably unnecessary)
	//iterator find(const_reference);
	//const_iterator find(const_reference) const;
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
compound_container<dim, Cont>::size_type compound_container<dim, Cont>::size() const
{
	size_t ret = 0;
	for (size_t i = 1; i <= dim; i++)
		ret += (*this)[i].size();
	return ret;
}

template <size_t dim, complete_container Cont>
bool compound_container<dim, Cont>::empty() const
{
	for (size_t i = 1; i <= dim; i++)
		if (!(*this)[i].empty())
			return false;
	return true;
}

// TODO (old) 004 : for object c of type compound_container<,>, c.begin() == c.end() must be true if and only if c.empty() is
// DONE
template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::iterator compound_container<dim, Cont>::begin()
{
	if (dim == 0)
		return iterator();
	else
	{
		for (size_t i = 1; i <= dim; ++i)
			if (!(*this)[i].empty())
				return iterator((*this)[i].begin(), *this, i);
		return iterator((*this)[dim].end(), *this, dim); // this->empty() case
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
			if (!(*this)[i].empty())
				return const_iterator((*this)[i].begin(), *this, i);
		return const_iterator((*this)[dim].end(), *this, dim); // this->empty() case
	}
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::iterator compound_container<dim, Cont>::end()
{
	if (dim == 0)
		return iterator();
	else
		return iterator((*this)[dim].end(), *this, dim);
}

template <size_t dim, complete_container Cont>
compound_container<dim, Cont>::const_iterator compound_container<dim, Cont>::end() const
{
	if (dim == 0)
		return const_iterator();
	else
		return const_iterator((*this)[dim].end(), *this, dim);
}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator() : base_iterator(), parent_cont(nullptr), comp_no(0) {}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator(const base_iterator& i, const compound_container<dim, Cont>& p, size_t cn)
	: base_iterator(i), parent_cont(&p), comp_no(cn) {}

template <size_t dim, complete_container Cont>
compound_const_iterator<dim, Cont>::compound_const_iterator(base_iterator&& i, const compound_container<dim, Cont>& p, size_t cn)
	: base_iterator(std::move(i)), parent_cont(&p), comp_no(cn) {}

// DONE : change incrementation according to the requirements of PROBLEM 004
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
	return reference{ comp_no, base_iterator::operator*() }; // *(static_cast<const base_iterator&>(*this)) or base_iterator::operator*(); which is better?
}

template <size_t dim, complete_container Cont>
inline compound_const_iterator<dim, Cont>::pointer compound_const_iterator<dim, Cont>::operator->() const
{ // this may not mimic base_iterator::operator->() (e.g. may not have the same return type)
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
	// apparently this sort of const_cast (where the object referred to is otherwise known to be mutable) does not result in undefined behaviour
}

template <size_t dim, complete_container Cont>
inline compound_iterator<dim, Cont>::pointer compound_iterator<dim, Cont>::operator->() const
{ // this may not mimic from_iterator::operator->() (e.g. may not have the same return type)
	return std::pointer_traits<pointer>::pointer_to(const_cast<value_type&>(counterpart_const_iterator::operator*().val()));
}

