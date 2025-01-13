#pragma once

#include <type_traits>
#include <utility>
#include <iterator>
using std::size_t;

template <class Cont>
concept container = requires(Cont ct)
{
	typename Cont::value_type;
	typename Cont::iterator;
	typename Cont::const_iterator;
	ct.size();
	ct.begin();
	ct.end();
	// TODO : return values of begin() and end() must be a compatible iterator (iter_value_t<...> ~ value_type)
};

template <container Cont>
struct cont_structure_helper;

template <class Cont>
concept sequence_container = container<Cont> && std::indirectly_writable<typename Cont::iterator, typename Cont::value_type>; // sufficient for our purposes

template <sequence_container Cont>
struct cont_structure_helper<Cont>
{
	typedef size_t structure_type;

	structure_type str;

	static inline structure_type extract(const Cont&);

	static inline Cont default_init(const structure_type&); // initializes all elements through their default constructors, except when Cont is an aggregate type
	static inline Cont explicit_init(const structure_type&); // explicitly initializes all elements to default connstructed values, therefore also works if Cont is an aggregate type

	static inline void assign(const Cont::iterator&, const Cont::value_type&);
};

template <class Cont, typename Index>
concept indexable = container<Cont> && requires(Cont ct, Index i) { ct[i]; };

template <class Cont>
concept sizable = container<Cont> && requires { Cont(size_t()); };

template <class Cont>
concept aggregate = container<Cont> && std::is_aggregate_v<Cont>;

template <sequence_container Cont>
inline cont_structure_helper<Cont>::structure_type cont_structure_helper<Cont>::extract(const Cont& ref) { return ref.size(); }

template <sequence_container Cont>
inline Cont cont_structure_helper<Cont>::default_init(const structure_type& siz)
{
	if constexpr (sizable<Cont>)
		return Cont(siz);
	else // probably aggregate<Cont>
	{
		Cont ret;
		/**/
		typename Cont::iterator it = ret.begin();
		for (size_t i = 1; i <= siz; i++, ++it)
			;
		// error if siz and size of ret don't match

		return ret;
	}
}

template <sequence_container Cont>
inline Cont cont_structure_helper<Cont>::explicit_init(const structure_type& siz)
{
	if constexpr (sizable<Cont>)
		return Cont(siz);
	else // probably aggregate<Cont>
	{
		Cont ret;
		/**/
		typename Cont::iterator it = ret.begin();
		for (size_t i = 1; i <= siz; i++, ++it)
			*it = typename Cont::value_type();
		// error if siz and size of ret don't match

		return ret;
	}
}

template <sequence_container Cont>
inline void cont_structure_helper<Cont>::assign(const Cont::iterator& pos, const Cont::value_type& val) { *pos = val; }

