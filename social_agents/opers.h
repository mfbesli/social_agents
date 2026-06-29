#pragma once

// unary/binary_operate : similar to std::transform, but uses methods specified in the cont_helper<> struct (see below) to initialize and access values within a container


#include "container_traits.h"

// HELPER TEMPLATE vvv

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

	static inline auto& project(const cont_helper<Cont>::reference&);
	static inline const auto& project(const cont_helper<Cont>::const_reference&);
};

// HELPER TEMPLATE RELATED CONCEPTS vvv

namespace helper_impl
{
	template <class Cont>
	concept container_has_helper = basic_container<Cont> && requires(Cont ct)
	{
		cont_helper<Cont>::value_init(cont_helper<Cont>::extract(ct));
		cont_helper<Cont>::project(*ct.begin());
	};

	template <basic_container Cont>
	struct access_type {};

	template <container_has_helper Cont>
	struct access_type<Cont>
	{
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

// UNARY CASE vvv

template <class Unary_Oper, basic_container Input_Cont, basic_container Output_Cont>
inline void unary_operate(Unary_Oper op, const Input_Cont& in_ct, Output_Cont& out_ct)
{
	// assertion if Input_Cont and Output_Cont are arrays whose sizes don't match
	auto input_it = in_ct.begin();
	auto output_it = out_ct.begin();
	for (; input_it != in_ct.end(); ++input_it, ++output_it)
		cont_helper<Output_Cont>::project(*output_it) = op(cont_helper<Input_Cont>::project(*input_it));
}

template <class Unary_Oper, basic_container Input_Cont, basic_container Output_Cont = Input_Cont>
inline Output_Cont unary_operate(Unary_Oper op, const Input_Cont& ct)
{
	Output_Cont ret = cont_helper<Output_Cont>::value_init(cont_helper<Input_Cont>::extract(ct));

	unary_operate(std::forward<Unary_Oper>(op), ct, ret);

	return ret;
}

// BINARY CASE vvv

template <class Binary_Oper, basic_container Input_Cont1, basic_container Input_Cont2, basic_container Output_Cont>
inline void binary_operate(Binary_Oper op, const Input_Cont1& in_ct1, const Input_Cont2& in_ct2, Output_Cont& out_ct)
{
	// assertion if Input_Cont1,2 and Output_Cont are arrays whose sizes don't match
	auto input_it1 = in_ct1.begin();
	auto input_it2 = in_ct2.begin();
	auto output_it = out_ct.begin();
	for (; input_it1 != in_ct1.end(); ++input_it1, ++input_it2, ++output_it)
		cont_helper<Output_Cont>::project(*output_it) = op(cont_helper<Input_Cont1>::project(*input_it1), cont_helper<Input_Cont2>::project(*input_it2));
}

template <class Binary_Oper, basic_container Input_Cont1, basic_container Input_Cont2, basic_container Output_Cont = Input_Cont1>
inline Output_Cont binary_operate(Binary_Oper op, const Input_Cont1& ct1, const Input_Cont2& ct2)
{
	typename cont_helper<Input_Cont1>::structure_type temp_str = cont_helper<Input_Cont1>::extract(ct1);
	if (temp_str != cont_helper<Input_Cont2>::extract(ct2))
		return Output_Cont();

	Output_Cont ret = cont_helper<Output_Cont>::value_init(temp_str);

	binary_operate(std::forward<Binary_Oper>(op), ct1, ct2, ret);

	return ret;
}

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
inline auto& cont_helper<Cont>::project(const cont_helper<Cont>::reference& ref) { return ref; }

template <sequencelike_container Cont>
inline const auto& cont_helper<Cont>::project(const cont_helper<Cont>::const_reference& ref) { return ref; }

