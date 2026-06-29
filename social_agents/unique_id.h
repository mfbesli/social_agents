#pragma once

// Unique ID specs:
// - A registry to keep track of which assigned ID user objects are currently in operation 
// - Each holder causes itself to be removed from the relevant registry during destruction as it will no longer be in operation 
// - No need for the registry to be able to access a specific ID holder


#include <unordered_set>
using std::size_t;

namespace unique_id
{
	typedef size_t type;

	class holder;

	class records
	{
	public:
		typedef std::unordered_set<type> reg_type;
		
	private:
		reg_type reg;

		type next;

	public:
		inline type assign(holder&);
		inline bool erase(type);

		inline type size() const;
		inline type total() const;

		records();
	};

	class holder
	{
	public:
		records* const rec;

		const type id;

		holder(records&);
		virtual ~holder();
	};

	constexpr type Unassigned = 0;

	// holder comparison object for ease of use with containers
	struct compare
	{
		bool operator()(const holder&, const holder&) const;
	};
}


inline unique_id::type unique_id::records::assign(holder& h)
{
	reg.insert(next);
	
	return next++;
}

inline bool unique_id::records::erase(type id)
{
	reg_type::const_iterator i;
	if ((i = reg.find(id)) == reg.end())
		return false;
	else
	{
		reg.erase(i);
		return true;
	}
}

inline unique_id::type unique_id::records::size() const { return reg.size(); }

inline unique_id::type unique_id::records::total() const { return next - 1; }

