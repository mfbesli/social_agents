#pragma once

// unary/binary_operate should only work with sequence containers (or containers who behave like one)

#include "container_traits.h"

// declaration for an undefined function template to help deduce Default_Unary_Output_Cont<...>
template <class Unary_Oper, template <typename, typename...> class Input_Cont_Tmp, typename Input_Val, typename... Tmp_Params>
auto decl_dft_unary_output_cont(Unary_Oper op, Input_Cont_Tmp<Input_Val, Tmp_Params...> input_cont)
-> Input_Cont_Tmp<std::remove_reference_t<decltype(op(std::declval<typename decltype(input_cont)::value_type>()))>, Tmp_Params...>;

// default container type for the return type of unary_operate is the same container template with appropriate value type (consistent with the return type of a Unary_Oper function or object)
template <class Unary_Oper, class Input_Cont>
using Default_Unary_Output_Cont = decltype(decl_dft_unary_output_cont(std::declval<Unary_Oper>(), std::declval<Input_Cont>()));

template <class Unary_Oper, container Input_Cont, container Output_Cont = Default_Unary_Output_Cont<Unary_Oper, Input_Cont>>
inline Output_Cont unary_operate(const Input_Cont& ct, Unary_Oper op, const typename cont_structure_helper<Output_Cont>::structure_type& str)
{
	Output_Cont ret = cont_structure_helper<Output_Cont>::default_init(str);
	// assertion if Input_Cont and Output_Cont are arrays whose sizes don't match

	typename Output_Cont::iterator output_it = ret.begin();
	for (const typename Input_Cont::value_type& in_val : ct)
	{
		cont_structure_helper<Output_Cont>::assign(output_it, op(in_val));
		++output_it;
	}

	return ret;
}

template <class Unary_Oper, container Input_Cont, container Output_Cont = Default_Unary_Output_Cont<Unary_Oper, Input_Cont>>
inline Output_Cont unary_operate(const Input_Cont& ct, Unary_Oper op)
{
	return unary_operate<Unary_Oper, Input_Cont, Output_Cont>(ct, std::forward<Unary_Oper>(op), cont_structure_helper<Input_Cont>::extract(ct));
}


// declaration for an undefined function template to help deduce Default_Binary_Output_Cont<...>
template <class Binary_Oper, template <typename, typename...> class Input_Cont_Tmp1, typename Input_Val1, typename... Tmp_Params1, class Input_Cont2>
auto decl_dft_binary_output_cont(Binary_Oper op, Input_Cont_Tmp1<Input_Val1, Tmp_Params1...> input_cont1, Input_Cont2 input_cont2)
-> Input_Cont_Tmp1<std::remove_reference_t<decltype(op(std::declval<typename decltype(input_cont1)::value_type>(), std::declval<typename Input_Cont2::value_type>()))>, Tmp_Params1...>;

// default container type for the return type of binary_operate, similar to Default_Unary_Output_Cont<...>
template <class Binary_Oper, class Input_Cont1, class Input_Cont2>
using Default_Binary_Output_Cont = decltype(decl_dft_binary_output_cont(std::declval<Binary_Oper>(), std::declval<Input_Cont1>(), std::declval<Input_Cont2>()));

template <class Binary_Oper, container Input_Cont1, container Input_Cont2, container Output_Cont = Default_Binary_Output_Cont<Binary_Oper, Input_Cont1, Input_Cont2>>
inline Output_Cont binary_operate(const Input_Cont1& ct1, const Input_Cont2& ct2, Binary_Oper op, const typename cont_structure_helper<Output_Cont>::structure_type& str)
{
	Output_Cont ret = cont_structure_helper<Output_Cont>::default_init(str);
	// assertion if Input_Cont1,2 and Output_Cont are arrays whose sizes don't match

	typename Input_Cont1::const_iterator input_it1 = ct1.begin();
	typename Input_Cont2::const_iterator input_it2 = ct2.begin();
	typename Output_Cont::iterator output_it = ret.begin();
	for (; input_it1 != ct1.end(); ++input_it1, ++input_it2, ++output_it)
		str.assign(output_it, op(*input_it1, *input_it2));

	return ret;
}

template <class Binary_Oper, container Input_Cont1, container Input_Cont2, container Output_Cont = Default_Binary_Output_Cont<Binary_Oper, Input_Cont1, Input_Cont2>>
inline Output_Cont binary_operate(const Input_Cont1& ct1, const Input_Cont2& ct2, Binary_Oper op)
{
	typename cont_structure_helper<Input_Cont1>::structure_type temp_str = cont_structure_helper<Input_Cont1>::extract(ct1);
	if (temp_str != cont_structure_helper<Input_Cont2>::extract(ct2))
		return Output_Cont();

	return binary_operate<Binary_Oper, Input_Cont1, Input_Cont2, Output_Cont>(ct1, ct2, std::forward<Binary_Oper>(op), temp_str);
}

