#pragma once

#include "container_traits.h"

// TODO 015 : uses of unary/binary_operate are no longer different from std::transform, consider replacing

// TODO (old) 010 : unary/binary_operate should use universal (forwarding) references for function/operator arguments (e.g. Unary_Oper&&)
// DONE : apparently it is preferred to pass by value and use std::ref() for expensive function objects, see PROBLEM 010

// HELPER TEMPLATE & RELATED CONCEPTS

template <basic_container Cont>
struct cont_helper;

namespace helper_impl
{
	template <class Cont>
	concept cont_has_iter = basic_container<Cont> && requires { typename Cont::iterator; };

	template <class Cont>
	concept cont_has_const_iter = basic_container<Cont> && requires { typename Cont::const_iterator; };

	template <basic_container Cont>
	struct iter_type { using type = decltype(std::declval<Cont&>().begin()); };

	template <cont_has_iter Cont>
	struct iter_type<Cont> { using type = Cont::iterator; };

	template <basic_container Cont>
	using iter_t = iter_type<Cont>::type;

	template <basic_container Cont>
	struct const_iter_type { using type = decltype(std::declval<const Cont&>().begin()); };

	template <cont_has_iter Cont>
	struct const_iter_type<Cont> { using type = Cont::const_iterator; };

	template <basic_container Cont>
	using const_iter_t = const_iter_type<Cont>::type;
}

template <basic_container Cont>
struct cont_helper_base
{
	using iterator			= helper_impl::iter_t<Cont>;
	using const_iterator	= helper_impl::const_iter_t<Cont>;
	using reference			= std::iter_reference_t<iterator>;
	using const_reference	= std::iter_reference_t<const_iterator>;
};

template <sequencelike_container Cont> // template specialization for the sequencelike_container case
struct cont_helper<Cont> : cont_helper_base<Cont>
{
	typedef size_t structure_type;

	structure_type str;

	static inline structure_type extract(const Cont&);

	static inline Cont value_init(const structure_type&);

	static inline auto& access(const cont_helper<Cont>::iterator&);
	static inline const auto& access(const cont_helper<Cont>::const_iterator&);

	static inline auto& project(const cont_helper<Cont>::reference&);
	static inline const auto& project(const cont_helper<Cont>::const_reference&);
};

namespace helper_impl
{
	template <class Cont>
	concept container_has_helper = basic_container<Cont> && requires(Cont ct)
	{
		cont_helper<Cont>::value_init(cont_helper<Cont>::extract(ct));
		//cont_helper<Cont>::access(ct.begin());
		cont_helper<Cont>::project(*ct.begin());
	};

	template <basic_container Cont>
	struct access_type {};

	template <container_has_helper Cont>
	struct access_type<Cont>
	{
		//using type = decltype(cont_helper<Cont>::access(std::declval<Cont>().begin()));
		using type = decltype(cont_helper<Cont>::project(*std::declval<Cont>().begin()));
	};
}

template <basic_container Cont>
using helper_access_t = helper_impl::access_type<Cont>::type;

template <class Cont>
concept container_has_readable_helper = helper_impl::container_has_helper<Cont> && std::is_lvalue_reference_v<helper_access_t<Cont>>;

template <class Cont, class FromVal>
concept container_has_writable_helper = container_has_readable_helper<Cont> && std::assignable_from<helper_access_t<Cont>, FromVal>;

template <class Cont>
concept container_has_self_writable_helper = container_has_writable_helper<Cont, std::remove_cvref_t<helper_access_t<Cont>>>;

template <class Cont>
concept helper_operable_float_container = std::semiregular<Cont> && container_has_self_writable_helper<Cont>&& std::floating_point<std::remove_reference_t<helper_access_t<Cont>>>;
// TODO (old) 005 : add copy_constructible<Cont> etc. to helper_operable_float_container (or elsewhere appropriate)
// DONE : CopyConstructible, CopyAssignable, MoveConstructible, DefaultConstructible are all required as in std::semiregular (std::swappable is not required but is still satisfied due to std::ranges::swap)

// UNARY CASE

template <class Unary_Oper, basic_container Input_Cont, basic_container Output_Cont>
inline void unary_operate(Unary_Oper op, const Input_Cont& in_ct, Output_Cont& out_ct)
{
	// assertion if Input_Cont and Output_Cont are arrays whose sizes don't match
	//typename Input_Cont::const_iterator input_it = in_ct.begin();
	//typename Output_Cont::iterator output_it = out_ct.begin();
	// auto makes it possible to not refer to iterator types, so unary_operate can work with basic_container
	auto input_it = in_ct.begin();
	auto output_it = out_ct.begin();
	for (; input_it != in_ct.end(); ++input_it, ++output_it)
		//cont_helper<Output_Cont>::access(output_it) = op(cont_helper<Input_Cont>::access(input_it));
		cont_helper<Output_Cont>::project(*output_it) = op(cont_helper<Input_Cont>::project(*input_it));
}

template <class Unary_Oper, basic_container Input_Cont, basic_container Output_Cont = Input_Cont>
inline Output_Cont unary_operate(Unary_Oper op, const Input_Cont& ct)
{
	Output_Cont ret = cont_helper<Output_Cont>::value_init(cont_helper<Input_Cont>::extract(ct)); // requires MoveConstructible Output_Cont
	// TODO 008 (i) : requires cont_helper<Input_Cont>::structure_type to be implicitly convertble to cont_helper<Output_Cont>::structure_type

	unary_operate(std::forward<Unary_Oper>(op), ct, ret);

	return ret;
}

// BINARY CASE

template <class Binary_Oper, basic_container Input_Cont1, basic_container Input_Cont2, basic_container Output_Cont>
inline void binary_operate(Binary_Oper op, const Input_Cont1& in_ct1, const Input_Cont2& in_ct2, Output_Cont& out_ct)
{
	// assertion if Input_Cont1,2 and Output_Cont are arrays whose sizes don't match
	//typename Input_Cont1::const_iterator input_it1 = in_ct1.begin();
	//typename Input_Cont2::const_iterator input_it2 = in_ct2.begin();
	//typename Output_Cont::iterator output_it = out_ct.begin();
	// auto makes it possible to not refer to iterator types, so binary_operate can work with basic_container
	auto input_it1 = in_ct1.begin();
	auto input_it2 = in_ct2.begin();
	auto output_it = out_ct.begin();
	for (; input_it1 != in_ct1.end(); ++input_it1, ++input_it2, ++output_it)
		//cont_helper<Output_Cont>::access(output_it) = op(cont_helper<Input_Cont1>::access(input_it1), cont_helper<Input_Cont2>::access(input_it2));
		cont_helper<Output_Cont>::project(*output_it) = op(cont_helper<Input_Cont1>::project(*input_it1), cont_helper<Input_Cont2>::project(*input_it2));
}

template <class Binary_Oper, basic_container Input_Cont1, basic_container Input_Cont2, basic_container Output_Cont = Input_Cont1>
inline Output_Cont binary_operate(Binary_Oper op, const Input_Cont1& ct1, const Input_Cont2& ct2)
{
	typename cont_helper<Input_Cont1>::structure_type temp_str = cont_helper<Input_Cont1>::extract(ct1);
	if (temp_str != cont_helper<Input_Cont2>::extract(ct2)) // TODO 008 (ii) : requires cont_helper<Input_Cont1>::extract(ct1) and cont_helper<Input_Cont2>::extract(ct2) to be comparable
		return Output_Cont();

	Output_Cont ret = cont_helper<Output_Cont>::value_init(temp_str); // requires MoveConstructible Output_Cont
	// TODO 008 (iii) : further requires cont_helper<Input_Cont1>::structure_type to be implicitly convertible to cont_helper<Output_Cont>::structure_type

	binary_operate(std::forward<Binary_Oper>(op), ct1, ct2, ret);

	return ret;
}

// TODO for distant future : expression templates and variadic n-ary_operate

// Definitions of member functions of helper template

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
inline auto& cont_helper<Cont>::access(const cont_helper<Cont>::iterator& pos) { return *pos; }

template <sequencelike_container Cont>
inline const auto& cont_helper<Cont>::access(const cont_helper<Cont>::const_iterator& pos) { return *pos; }

template <sequencelike_container Cont>
inline auto& cont_helper<Cont>::project(const cont_helper<Cont>::reference& ref) { return ref; }

template <sequencelike_container Cont>
inline const auto& cont_helper<Cont>::project(const cont_helper<Cont>::const_reference& ref) { return ref; }

