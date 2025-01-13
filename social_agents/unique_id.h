#pragma once

#include <unordered_map>
using std::size_t;

namespace unique_id
{
	typedef size_t type;

	class holder;

	class records
	{
	public:
		typedef std::unordered_map<type, holder*> reg_type;
		
	private:
		reg_type reg;

		type next;

	public:
		inline type assign(holder*);
		inline holder* find(type) const;
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

	struct compare
	{
		bool operator()(const holder&, const holder&) const;
	};
}


inline unique_id::type unique_id::records::assign(holder* h)
{
	if (h != nullptr)
	{
		reg[next] = h;
		return next++;
	}
	else
		return 0;
}

inline unique_id::holder* unique_id::records::find(type id) const
{
	reg_type::const_iterator i;
	if ((i = reg.find(id)) == reg.end())
		return nullptr;
	else
		return i->second;
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

