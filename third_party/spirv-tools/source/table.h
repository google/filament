// Copyright (c) 2015-2016 The Khronos Group Inc.
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

#ifndef SOURCE_TABLE_H_
#define SOURCE_TABLE_H_

#include "source/extensions.h"
#include "source/latest_version_spirv_header.h"
#include "source/util/index_range.h"
#include "spirv-tools/libspirv.hpp"

// NOTE: Instruction and operand tables have moved to table2.{h|cpp},
// where they are represented more compactly.

struct spv_context_t {
  const spv_target_env target_env;
  spvtools::MessageConsumer consumer;
};

namespace spvtools {

// Sets the message consumer to |consumer| in the given |context|. The original
// message consumer will be overwritten.
void SetContextMessageConsumer(spv_context context, MessageConsumer consumer);
}  // namespace spvtools

#endif  // SOURCE_TABLE_H_
