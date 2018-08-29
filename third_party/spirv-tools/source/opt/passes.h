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

#ifndef SOURCE_OPT_PASSES_H_
#define SOURCE_OPT_PASSES_H_

// A single header to include all passes.

#include "source/opt/aggressive_dead_code_elim_pass.h"
#include "source/opt/block_merge_pass.h"
#include "source/opt/ccp_pass.h"
#include "source/opt/cfg_cleanup_pass.h"
#include "source/opt/combine_access_chains.h"
#include "source/opt/common_uniform_elim_pass.h"
#include "source/opt/compact_ids_pass.h"
#include "source/opt/copy_prop_arrays.h"
#include "source/opt/dead_branch_elim_pass.h"
#include "source/opt/dead_insert_elim_pass.h"
#include "source/opt/dead_variable_elimination.h"
#include "source/opt/eliminate_dead_constant_pass.h"
#include "source/opt/eliminate_dead_functions_pass.h"
#include "source/opt/flatten_decoration_pass.h"
#include "source/opt/fold_spec_constant_op_and_composite_pass.h"
#include "source/opt/freeze_spec_constant_value_pass.h"
#include "source/opt/if_conversion.h"
#include "source/opt/inline_exhaustive_pass.h"
#include "source/opt/inline_opaque_pass.h"
#include "source/opt/licm_pass.h"
#include "source/opt/local_access_chain_convert_pass.h"
#include "source/opt/local_redundancy_elimination.h"
#include "source/opt/local_single_block_elim_pass.h"
#include "source/opt/local_single_store_elim_pass.h"
#include "source/opt/local_ssa_elim_pass.h"
#include "source/opt/loop_fission.h"
#include "source/opt/loop_fusion_pass.h"
#include "source/opt/loop_peeling.h"
#include "source/opt/loop_unroller.h"
#include "source/opt/loop_unswitch_pass.h"
#include "source/opt/merge_return_pass.h"
#include "source/opt/null_pass.h"
#include "source/opt/private_to_local_pass.h"
#include "source/opt/reduce_load_size.h"
#include "source/opt/redundancy_elimination.h"
#include "source/opt/remove_duplicates_pass.h"
#include "source/opt/replace_invalid_opc.h"
#include "source/opt/scalar_replacement_pass.h"
#include "source/opt/set_spec_constant_default_value_pass.h"
#include "source/opt/ssa_rewrite_pass.h"
#include "source/opt/strength_reduction_pass.h"
#include "source/opt/strip_debug_info_pass.h"
#include "source/opt/strip_reflect_info_pass.h"
#include "source/opt/unify_const_pass.h"
#include "source/opt/vector_dce.h"
#include "source/opt/workaround1209.h"

#endif  // SOURCE_OPT_PASSES_H_
