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

#ifndef SOURCE_SPIRV_TARGET_ENV_H_
#define SOURCE_SPIRV_TARGET_ENV_H_

#include "spirv-tools/libspirv.h"

// Parses s into *env and returns true if successful.  If unparsable, returns
// false and sets *env to SPV_ENV_UNIVERSAL_1_0.
bool spvParseTargetEnv(const char* s, spv_target_env* env);

// Returns true if |env| is a VULKAN environment, false otherwise.
bool spvIsVulkanEnv(spv_target_env env);

// Returns the version number for the given SPIR-V target environment.
uint32_t spvVersionForTargetEnv(spv_target_env env);

#endif  // SOURCE_SPIRV_TARGET_ENV_H_
