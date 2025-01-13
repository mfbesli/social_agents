#pragma once

#include "unique_id.h"

struct unique_id_pair_hasher
{
	static constexpr size_t factor = (sizeof(size_t) >= 8) ? 0x9e3779b97f4a7c17 : 0x9e3779b9; // from Fibonacci hashing

	inline size_t operator()(const std::pair<unique_id::type, unique_id::type>& pr) const
	{
		return factor * std::hash<unique_id::type>()(pr.first) + std::hash<unique_id::type>()(pr.second);
	}
};

/*
struct unique_id_pair_unordered : public std::pair<unique_id::type, unique_id::type>
{
	unique_id_pair_unordered();
	unique_id_pair_unordered(const unique_id::type&, const unique_id::type&);
	unique_id_pair_unordered(const std::pair<unique_id::type, unique_id::type>&);
	unique_id_pair_unordered(const unique_id_pair_unordered&);
	unique_id_pair_unordered(unique_id_pair_unordered&&);
};
*/

