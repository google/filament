// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LIBSPIRV_OPT_PASSES_H_
#define LIBSPIRV_OPT_PASSES_H_

// A single header to include all passes.

#include "aggressive_dead_code_elim_pass.h"
#include "block_merge_pass.h"
#include "ccp_pass.h"
#include "cfg_cleanup_pass.h"
#include "combine_access_chains.h"
#include "common_uniform_elim_pass.h"
#include "compact_ids_pass.h"
#include "copy_prop_arrays.h"
#include "dead_branch_elim_pass.h"
#include "dead_insert_elim_pass.h"
#include "dead_variable_elimination.h"
#include "eliminate_dead_constant_pass.h"
#include "eliminate_dead_functions_pass.h"
#include "flatten_decoration_pass.h"
#include "fold_spec_constant_op_and_composite_pass.h"
#include "freeze_spec_constant_value_pass.h"
#include "if_conversion.h"
#include "inline_exhaustive_pass.h"
#include "inline_opaque_pass.h"
#include "licm_pass.h"
#include "local_access_chain_convert_pass.h"
#include "local_redundancy_elimination.h"
#include "local_single_block_elim_pass.h"
#include "local_single_store_elim_pass.h"
#include "local_ssa_elim_pass.h"
#include "loop_fission.h"
#include "loop_fusion_pass.h"
#include "loop_peeling.h"
#include "loop_unroller.h"
#include "loop_unswitch_pass.h"
#include "merge_return_pass.h"
#include "null_pass.h"
#include "private_to_local_pass.h"
#include "reduce_load_size.h"
#include "redundancy_elimination.h"
#include "remove_duplicates_pass.h"
#include "replace_invalid_opc.h"
#include "scalar_replacement_pass.h"
#include "set_spec_constant_default_value_pass.h"
#include "ssa_rewrite_pass.h"
#include "strength_reduction_pass.h"
#include "strip_debug_info_pass.h"
#include "strip_reflect_info_pass.h"
#include "unify_const_pass.h"
#include "vector_dce.h"
#include "workaround1209.h"
#endif  // LIBSPIRV_OPT_PASSES_H_
