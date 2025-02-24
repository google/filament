/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include "pass.h"

namespace gpuav {
namespace spirv {

// Validating validating that gl_VertexID is an index within bound vertex buffers
class VertexAttributeFetchOob : public Pass {
  public:
    VertexAttributeFetchOob(Module& module);
    const char* Name() const final { return "VertexAttributeFetchOob"; }

    bool Instrument();
    void PrintDebugInfo() const final;

  private:
    uint32_t GetLinkFunctionId();
    void Reset() final {}

    uint32_t link_function_id = 0;
    bool instrumentation_performed = false;
};

}  // namespace spirv
}  // namespace gpuav
