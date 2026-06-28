#pragma once

// NEW

#include <type_traits> // std::is_aggregate_v
#include <utility> // std::size_t
#include <iterator> // std::indirectly_writable
using std::size_t;

// Concepts:

template <class Cont>
concept basic_container = requires(Cont ct) // simple container that can be used in a range-based for loop
{
	typename Cont::value_type;
	{ ct.size() } -> std::convertible_to<size_t>;
	requires std::equality_comparable_with<decltype(ct.begin()), decltype(ct.end())>; 
};

template <class Cont>
concept basic_writable_container = basic_container<Cont> && requires(Cont ct)
{
	requires std::indirectly_writable<decltype(ct.begin()), typename Cont::value_type>;
};

template <class Cont>
concept complete_container = basic_container<Cont> && requires(Cont ct, const Cont c_ct)
{
	typename Cont::size_type;
	typename Cont::difference_type;
	{ c_ct.size() } -> std::convertible_to<size_t>;
	{ ct.begin() } -> std::same_as<typename Cont::iterator>; // std::convertible_to instead ?
	{ ct.end() } -> std::sentinel_for<typename Cont::iterator>;
	{ c_ct.begin() } -> std::same_as<typename Cont::const_iterator>; // std::convertible_to instead ?
	{ c_ct.end() } -> std::sentinel_for<typename Cont::const_iterator>;
	requires std::common_reference_with<std::iter_reference_t<typename Cont::iterator>, typename Cont::value_type&>;
	requires std::common_reference_with<std::iter_reference_t<typename Cont::const_iterator>, const typename Cont::value_type&>;
	// TODO (old) : returned iterator types of begin() and end() must be compatible with value_type etc. (iter_value_t<...> ~ value_type)
	// DONE : above, but with references (probably not strict enough)
};

template <class Cont>
concept writable_container = complete_container<Cont> && basic_writable_container<Cont>;

template <class Cont>
concept sizable_container = complete_container<Cont> && requires { Cont(std::declval<size_t>()); }; // container that can be constructed by specifying a desired size
/*
template <class Cont>
concept sizable_container = complete_container<Cont> && requires (void fn(Cont)) { fn({ std::declval<size_t>() }); }; // doesn't work if the relevant constructor is explicit
*/
template <class Cont>
concept aggregate_container = complete_container<Cont> && std::is_aggregate_v<Cont>;
/*
template <class Cont, typename Index>
concept indexable_container = complete_container<Cont> && requires(Cont ct, Index i) { { ct[i] } -> std::common_reference_with<Cont::value_type&>; };
*/
template <class Cont>
concept sequencelike_container = sizable_container<Cont> || aggregate_container<Cont>;

// Below part is moved elsewhere, will be erased
/* 
// Helper Struct:

template <basic_container Cont>
struct cont_helper;

template <sequencelike_container Cont>
struct cont_helper<Cont> // template specialization for the sequencelike_container case
{
	typedef size_t structure_type;

	structure_type str;

	// All cont_helper structs must have below utility functions (extract, value_init, assign)
	static inline structure_type extract(const Cont&);

	static inline Cont value_init(const structure_type&);

	//static inline Cont default_init(const structure_type&); // initializes all elements through their default constructors, except when Cont is an aggregate type
	//static inline Cont explicit_init(const structure_type&); // explicitly initializes all elements to default connstructed values, therefore also works if Cont is an aggregate type

	static inline auto& access(const Cont::iterator&);
	static inline const auto& access(const Cont::const_iterator&);

	// const and non-const access instead of assign ?
	//template <typename Assignment_Rhs>
	//	requires std::indirectly_writable<typename Cont::iterator, Assignment_Rhs>
	//static inline void assign(const Cont::iterator& pos, Assignment_Rhs&& val);
};

template <sequencelike_container Cont>
inline cont_helper<Cont>::structure_type cont_helper<Cont>::extract(const Cont& ref) { return ref.size(); }

template <sequencelike_container Cont>
inline Cont cont_helper<Cont>::value_init(const structure_type& siz)
{
	if constexpr (sizable_container<Cont>)
		return Cont(siz);
	else // aggregate_container<Cont>
		return Cont{}; // value initialization includes zero initialization of each element for aggregates
}

template <sequencelike_container Cont>
inline Cont cont_helper<Cont>::default_init(const structure_type& siz)
{
	if constexpr (sizable_container<Cont>)
		return Cont(siz);
	else // aggregate_container<Cont>
	{
		Cont ret;
		
		typename Cont::iterator it = ret.begin();
		for (size_t i = 1; i <= siz; i++, ++it)
			;
		// error/exception if siz and size of ret don't match

		return ret;
	}
}

template <sequencelike_container Cont>
inline Cont cont_helper<Cont>::explicit_init(const structure_type& siz)
{
	if constexpr (sizable_container<Cont>)
		return Cont(siz);
	else // aggregate_container<Cont>
	{
		Cont ret;
		
		typename Cont::iterator it = ret.begin();
		for (size_t i = 1; i <= siz; i++, ++it)
			*it = typename Cont::value_type();
		// error/exception if siz and size of ret don't match

		return ret;
	}
}


template <sequencelike_container Cont>
inline auto& cont_helper<Cont>::access(const Cont::iterator& pos) { return *pos; }

template <sequencelike_container Cont>
inline const auto& cont_helper<Cont>::access(const Cont::const_iterator& pos) { return *pos; }

//template <sequencelike_container Cont>
//template <typename Assignment_Rhs>
//	requires std::indirectly_writable<typename Cont::iterator, Assignment_Rhs>
//inline void cont_helper<Cont>::assign(const Cont::iterator& pos, Assignment_Rhs&& val) { *pos = val; };
*/
