#pragma once

#include "single_ind_base.h"

template <size_t dim>
class process;

template <size_t dim>
class individual : public individual_base<dim, individual>
{
public:
	individual();

	inline process<dim>& parent_process() const;

protected:
	virtual polynomial::compound_term<dim> free_param();
};

template <size_t dim>
class process : public process_base_seq<dim, individual>
{
public:
	process();

	void initialize();

	const std::list<individual_base<dim, individual>*>& ind_priority_list();

	void record_observations();
};


