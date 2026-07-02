#pragma once

// Concepts for containers to help constrain which are eligible to be used by the algorithms and classes in the project


#include <concepts>
#include <type_traits>
#include <utility>
#include <iterator>
using std::size_t;

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
	{ ct.begin() } -> std::same_as<typename Cont::iterator>;
	{ ct.end() } -> std::sentinel_for<typename Cont::iterator>;
	{ c_ct.begin() } -> std::same_as<typename Cont::const_iterator>;
	{ c_ct.end() } -> std::sentinel_for<typename Cont::const_iterator>;
	requires std::common_reference_with<std::iter_reference_t<typename Cont::iterator>, typename Cont::value_type&>;
	requires std::common_reference_with<std::iter_reference_t<typename Cont::const_iterator>, const typename Cont::value_type&>;
};

template <class Cont>
concept writable_container = complete_container<Cont> && basic_writable_container<Cont>;

template <class Cont>
concept sizable_container = complete_container<Cont> && requires { Cont(std::declval<size_t>()); }; // container that can be constructed by specifying a desired size

template <class Cont>
concept aggregate_container = complete_container<Cont> && std::is_aggregate_v<Cont>;
/*
template <class Cont, typename Index>
concept indexable_container = complete_container<Cont> && requires(Cont ct, Index i) { { ct[i] } -> std::common_reference_with<Cont::value_type&>; };
*/
template <class Cont>
concept sequencelike_container = sizable_container<Cont> || aggregate_container<Cont>;

