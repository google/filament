// Copyright (c) 2018 The Khronos Group Inc.
// Copyright (c) 2018 Valve Corporation
// Copyright (c) 2018 LunarG Inc.
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

#ifndef LIBSPIRV_UTIL_COMPOSITE_PASS_H_
#define LIBSPIRV_UTIL_COMPOSITE_PASS_H_

#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "basic_block.h"
#include "def_use_manager.h"
#include "ir_context.h"
#include "module.h"

namespace spvtools {
namespace opt {

// Return true if the extract indices in |extIndices| starting at |extOffset|
// match indices of insert |insInst|.
bool ExtInsMatch(const std::vector<uint32_t>& extIndices,
                 const Instruction* insInst, const uint32_t extOffset);

// Return true if indices in |extIndices| starting at |extOffset| and
// indices of insert |insInst| conflict, specifically, if the insert
// changes bits specified by the extract, but changes either more bits
// or less bits than the extract specifies, meaning the exact value being
// inserted cannot be used to replace the extract.
bool ExtInsConflict(const std::vector<uint32_t>& extIndices,
                    const Instruction* insInst, const uint32_t extOffset);

}  // namespace opt
}  // namespace spvtools

#endif  // LIBSPIRV_UTIL_COMPOSITE_PASS_H_
