#pragma once

// TODO NEXT (2nd) : tl_wrapper that doesn't require default construction of tl_wrapper::init

#include <mutex>

namespace par_wrapper_impl
{
	template <class Res>
	constexpr size_t next_wrp()
	{
		static constexpr size_t next = 0;
		return ++next;
	}

	template <class Res, size_t n = next_wrp<Res>()> // requires DefaultConstructible and CopyConstructible Res
	class tl_wrapper_n // still shared by different objects of the same type
	{
		Res init;
		std::mutex m;

		// TODO : shouldn't be copy constructible

	public:
		Res& get()
		{
			thread_local Res res((std::lock_guard<std::mutex>(this->m), init)); // acquire lock if initializing and copy construct from init
			return res;
		}
	};
}

template <class Res>
using tl_wrapper = par_wrapper_impl::tl_wrapper_n<Res>;

template <class Res> // requires DefaultConstructible and CopyConstructible Res
class tl_wrapper_1 // in its present form this template can be used for at most one object of the same type (Res)
{
	Res init;
	std::mutex m;

public:
	Res& get();
};

template <class Res>
Res& tl_wrapper_1<Res>::get()
{
	thread_local Res res((std::lock_guard<std::mutex>(this->m), init)); // acquire lock if initializing and copy construct from init
	return res;
}

