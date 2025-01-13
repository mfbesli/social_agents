#pragma once

#include "single_ind_base.h"

template <size_t dim>
class process;

template <size_t dim>
class individual : public individual_base<dim>
{
public:
	individual();

	inline process<dim>& parent_process() const;

protected:
	virtual polynomial::term_refer<dim> free_param();
};

template <size_t dim>
class process : public process_base<dim>
{
public:
	process();

	void initialize();

	const std::list<individual_base<dim>*>& ind_priority_list();

	void record_observations();
};


