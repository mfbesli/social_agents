#pragma once

#include "numerical_sim.h"
#include "fdc.h"
#include "minimize.h"
#include "unique_id.h"
#include "change_func.h"

template <class Ind>
using individual_cont = std::list<Ind>;

template <size_t dim, class Ind>
class process_base_seq
{
public:
	unique_id::records rec;

	const polynomial::coefficients<dim> ontic_poly;

	numerical_sim<polynomial::float_type, dim> simulator;

	backward_fdc<dim> fdc_approximator;

	NM_minimize<polynomial::coefficients<dim>> minimizer;

	const polynomial::float_type time_step;

	const size_t state_record_len; // >= fdc length

	const size_t diff_record_len; // >= error length

protected:
	std::deque<aug::array<polynomial::float_type, dim>> state_record;
	// TODO : determine if using std::deque<> causes unnecessary allocations and excessive memory usage due to the moving window behaviour 
	std::deque<aug::array<polynomial::float_type, dim>> diff_approx_record;

	size_t iter;

	individual_cont<Ind> people;

public:
	process_base_seq(const polynomial::coefficients<dim>&, size_t, polynomial::float_type, size_t);

	inline const std::deque<aug::array<polynomial::float_type, dim>>& get_state_record() const;
	inline const std::deque<aug::array<polynomial::float_type, dim>>& get_diff_record() const;
	inline size_t current_iter() const;

	virtual void initialize() = 0;
	// virtual void advance() = 0;
	void advance();

protected:/*
	virtual void pre_step() {}
	virtual void post_step() {}
	
	template <class Ind>
	std::list<Ind*> extract_ind_list(const std::list<unique_id::type>& id_list) const;
	template <class Ind>
	std::list<Ind*> extract_ind_list() const;
	*/
	virtual const std::list<Ind*>& ind_priority_list() = 0; // may include updating the priority list, so not const

	inline polynomial::function<dim>& access_poly(Ind&);

	void compute_next_state();
	void compute_next_diff_approx();

	virtual void record_observations() = 0;
};

template <size_t dim, class Ind>
struct error_func_base;

template <size_t dim, class Ind>
struct error_gradient_func_base;

template <size_t dim, class Ind>
class individual_base : public unique_id::holder
{
public:
	process_base_seq<dim, Ind>* const parent_proc;

protected:
	polynomial::function<dim> poly;

public:
	change_func_base<polynomial::float_type>* change_func;

	NM_minimize<polynomial::coefficients<dim>>* minimizer;

	error_func_base<dim, Ind>* error_func;

	const size_t error_record_len;

	const size_t changes_len;

protected:
	std::deque<polynomial::float_type> error_record;
	// TODO : determine if using std::deque<> causes unnecessary allocations and excessive memory usage due to the moving window behaviour 
	polynomial::float_type error_threshold, high_error_ratio;

	std::deque<polynomial::coefficients<dim>> param_changes;

	polynomial::float_type fixing_threshold;

	polynomial::coefficients<dim> biases;

	size_t biases_iter;

public:
	individual_base(process_base_seq<dim, Ind>&, change_func_base<polynomial::float_type>&, NM_minimize<polynomial::coefficients<dim>>&,
		error_func_base<dim, Ind>&, size_t, size_t, polynomial::float_type, polynomial::float_type, polynomial::float_type);

	inline const polynomial::function<dim>& get_poly() const;

	virtual void pre_step();
	virtual void post_step() {}

	void minimize();
	void check_param_acts();

protected:
	inline void add_error_to_record(polynomial::float_type);
	inline void update();
	inline void change_params(const polynomial::coefficients<dim>&);

	virtual polynomial::compound_term<dim> free_param() = 0;
	void fix_params();

	//friend polynomial::function<dim>& process_base_seq<dim, Ind>::access_poly(Ind&); // is this still needed?
};

template <size_t dim, class Ind>
struct error_func_base
{
	virtual polynomial::float_type operator()(const Ind&, const polynomial::coefficients<dim>&) const = 0;

	virtual ~error_func_base() = default;
};

template <size_t dim, class Ind>
struct error_gradient_func_base
{
	virtual polynomial::coefficients<dim> operator()(const Ind&, const polynomial::coefficients<dim>&) const = 0;

	virtual ~error_gradient_func_base() = default;
};

template <class Func, class Grad>
struct error_func_gradient_cont_base // TODO : rename to error_func_gradient_pair_base
{
	Func func;
	Grad gradient_func;

	error_func_gradient_cont_base();

	virtual ~error_func_gradient_cont_base() = default;
};

#include <map>

template <class Val>
class observation
{
public:
	typedef std::map<size_t, Val> container_type;

protected:
	container_type record;

public:
	bool add(size_t, const Val&);
	bool add(size_t, Val&&); // is this beneficial?

	inline const container_type& get() const;
};


template <size_t dim, class Ind>
process_base_seq<dim, Ind>::process_base_seq(const polynomial::coefficients<dim>& poly, size_t r_len, polynomial::float_type t_step, size_t sim_fineness) :
	rec(), ontic_poly(poly), simulator(sim_fineness), time_step(t_step), state_record_len(r_len), iter(0) {}

template <size_t dim, class Ind>
inline const std::deque<aug::array<polynomial::float_type, dim>>& process_base_seq<dim, Ind>::get_state_record() const { return state_record; }

template <size_t dim, class Ind>
inline const std::deque<aug::array<polynomial::float_type, dim>>& process_base_seq<dim, Ind>::get_diff_record() const { return diff_approx_record; }

template <size_t dim, class Ind>
inline size_t process_base_seq<dim, Ind>::current_iter() const { return iter; }

template <size_t dim, class Ind>
void process_base_seq<dim, Ind>::advance()
{
	iter++;

	const std::list<Ind*>& ind_list = ind_priority_list();

	for (Ind* const& ind : ind_list)
		ind->pre_step();

	for (Ind* const& ind : ind_list)
		ind->minimize();

	for (Ind* const& ind : ind_list)
		ind->check_param_acts();

	for (Ind* const& ind : ind_list)
		ind->post_step();

	record_observations();

	compute_next_state();

	// TODO : compute_next_diff_approx();
}

template <size_t dim, class Ind>
inline polynomial::function<dim>& process_base_seq<dim, Ind>::access_poly(Ind& ind) { return ind.poly; }

template <size_t dim, class Ind>
void process_base_seq<dim, Ind>::compute_next_state()
{
	auto func = [this](const aug::array<dim, polynomial::float_type>& eval_pt) { return this->ontic_poly.evaluate(eval_pt); };

	state_record.push_back(simulator(func, state_record.back(), time_step));

	if (state_record.size() > state_record_len)
		state_record.pop_front();
}

template <size_t dim, class Ind>
void process_base_seq<dim, Ind>::compute_next_diff_approx()
{
	diff_approx_record.push_back(fdc_approximator(get_state_record(), time_step));

	if (diff_approx_record.size() > diff_record_len)
		diff_approx_record.pop_front();
}

template <size_t dim, class Ind>
individual_base<dim, Ind>::individual_base(process_base_seq<dim, Ind>& proc, change_func_base<polynomial::float_type>& chg_f,
	NM_minimize<polynomial::coefficients<dim>>& min, error_func_base<dim, Ind>& err, size_t err_len, size_t chg_len,
	polynomial::float_type err_thr, polynomial::float_type err_rat, polynomial::float_type fix_thr) : unique_id::holder(proc.rec),
	parent_proc(&proc), change_func(&chg_f), minimizer(&min), error_func(&err), error_record_len(err_len), changes_len(chg_len),
	error_threshold(err_thr), high_error_ratio(err_rat), fixing_threshold(fix_thr) {}

template <size_t dim, class Ind>
inline const polynomial::function<dim>& individual_base<dim, Ind>::get_poly() const { return poly; }

template <size_t dim, class Ind>
void individual_base<dim, Ind>::minimize() // TODO : change
{
	auto lam = [this](const polynomial::coefficients<dim>& act_coeffs) { return (*this->error_func)(*this, act_coeffs); };

	polynomial::coefficients<dim> temp = poly.sieve_acts();

	add_error_to_record(lam(temp));

	change_params((*minimizer)(lam, temp));
}

template <size_t dim, class Ind>
void individual_base<dim, Ind>::check_param_acts()
{
	bool s_free = false;
	if (error_record.size() >= error_record_len)
	{
		size_t high_err = 0;
		for (polynomial::float_type err : error_record)
			if (err > error_threshold)
				high_err++;

		if (polynomial::float_type(high_err) / polynomial::float_type(error_record.size()) > high_error_ratio)
			s_free = true;
	}

	if (s_free)
		free_param();
	else if (param_changes.size() >= changes_len)
		fix_params();
}

template <size_t dim, class Ind>
inline void individual_base<dim, Ind>::add_error_to_record(polynomial::float_type err)
{
	error_record.push_back(err);

	if (error_record.size() > error_record_len)
		error_record.pop_front();
}

template <size_t dim, class Ind>
inline void individual_base<dim, Ind>::change_params(const polynomial::coefficients<dim>& new_coeffs)
{
	polynomial::coefficients<dim> before = poly.coeffs;

	poly.coeffs.fuse(new_coeffs);
	
	param_changes.emplace_back(); // TODO 030 (Part 1) : param_changes.emplace_back(poly.coeffs) instead for a single allocation
	/*
	for (size_t i = 1; i <= dim; i++)
		for (const typename polynomial::coefficients<dim>::term_coeff_map::value_type& v : poly.coeffs.get_coeff_arr()[i])
		{
			polynomial::compound_term<dim> temp{ i, v.first };
			param_changes.back().set_coeff(temp, (*change_func)(before.get_coeff(temp), v.second));
		}*/

	for (typename polynomial::coefficients<dim>::const_reference tc : poly.coeffs) // TODO 030 (Part 2) : loop over param_changes.back() instead
	{
		// TODO 030 (Part 3) : cont_helper<polynomial::coefficients<dim>>::project(tc) = ...
		//					   instead of the two statements below (possibly through std::transform()) :
		polynomial::compound_term<dim> temp{ tc.comp_no(), tc.val().first };
		param_changes.back().set_coeff(temp, (*change_func)(before.get_coeff(temp), tc.val().second));
	}

	if (param_changes.size() > changes_len)
		param_changes.pop_front();
}

template <size_t dim, class Ind>
void individual_base<dim, Ind>::fix_params()
{
	polynomial::term_cont<dim> temp;
	// TODO : consider transforming below loops
	for (size_t i = 1; i <= dim; i++)
		for (const polynomial::term<dim>& t : poly.actives[i])
		{
			bool s_fix = true;
			for (const polynomial::coefficients<dim>& cs : param_changes)
				if (cs.get_coeff(polynomial::compound_term<dim>{ i, t }) > fixing_threshold)
				{
					s_fix = false;
					break;
				}

			if (s_fix)
				temp[i].insert(t);
		}

	for (size_t i = 1; i <= dim; i++)
		for (const polynomial::term<dim>& t : temp[i])
			poly.actives[i].erase(t);
}

template <class Func, class Grad>
error_func_gradient_cont_base<Func, Grad>::error_func_gradient_cont_base() : func(this), gradient_func(this) {}

template <typename Val>
bool observation<Val>::add(size_t iter, const Val& obsv)
{
	if (record.count(iter) == 0)
	{
		record.insert(std::make_pair(iter, obsv));
		return true;
	}
	else
		return false;
}

template <typename Val>
bool observation<Val>::add(size_t iter, Val&& obsv) // is this move semantics version beneficial?
{
	if (record.count(iter) == 0)
	{
		record.insert(std::make_pair(iter, std::move(obsv)));
		return true;
	}
	else
		return false;
}

template <typename Val>
inline const observation<Val>::container_type& observation<Val>::get() const { return record; }


