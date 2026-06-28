#pragma once

// TODO NEXT (1st) : a simple process_base_par to test tl_wrapper and tl_wrapper_1

#include "numerical_sim.h"
#include "fdc.h"
#include "minimize.h"
#include "unique_id.h"
#include "change_func.h"
#include "single_ind_par_helper.h"

template <class Ind>
using individual_cont = std::list<Ind>;

template <size_t dim, class Ind>
class process_base_par
{

};

