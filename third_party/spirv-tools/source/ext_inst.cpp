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

#include "source/ext_inst.h"

#include <cstring>

// DebugInfo extended instruction set.
// #include "DebugInfo.h"

spv_ext_inst_type_t spvExtInstImportTypeGet(const char* name) {
  // The names are specified by the respective extension instruction
  // specifications.
  if (!strcmp("GLSL.std.450", name)) {
    return SPV_EXT_INST_TYPE_GLSL_STD_450;
  }
  if (!strcmp("OpenCL.std", name)) {
    return SPV_EXT_INST_TYPE_OPENCL_STD;
  }
  if (!strcmp("SPV_AMD_shader_explicit_vertex_parameter", name)) {
    return SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER;
  }
  if (!strcmp("SPV_AMD_shader_trinary_minmax", name)) {
    return SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX;
  }
  if (!strcmp("SPV_AMD_gcn_shader", name)) {
    return SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER;
  }
  if (!strcmp("SPV_AMD_shader_ballot", name)) {
    return SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT;
  }
  if (!strcmp("DebugInfo", name)) {
    return SPV_EXT_INST_TYPE_DEBUGINFO;
  }
  if (!strcmp("OpenCL.DebugInfo.100", name)) {
    return SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100;
  }
  if (!strcmp("NonSemantic.Shader.DebugInfo.100", name)) {
    return SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100;
  }
  if (!strncmp("NonSemantic.ClspvReflection.", name, 28)) {
    return SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION;
  }
  if (!strncmp("NonSemantic.VkspReflection.", name, 27)) {
    return SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION;
  }
  if (!strcmp("TOSA.001000.1", name)) {
    return SPV_EXT_INST_TYPE_TOSA_001000_1;
  }
  if (!strcmp("Arm.MotionEngine.100", name)) {
    return SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100;
  }
  // ensure to add any known non-semantic extended instruction sets
  // above this point, and update spvExtInstIsNonSemantic()
  if (!strncmp("NonSemantic.", name, 12)) {
    return SPV_EXT_INST_TYPE_NONSEMANTIC_UNKNOWN;
  }
  return SPV_EXT_INST_TYPE_NONE;
}

bool spvExtInstIsNonSemantic(const spv_ext_inst_type_t type) {
  if (type == SPV_EXT_INST_TYPE_NONSEMANTIC_UNKNOWN ||
      type == SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100 ||
      type == SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION ||
      type == SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION) {
    return true;
  }
  return false;
}

bool spvExtInstIsDebugInfo(const spv_ext_inst_type_t type) {
  if (type == SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100 ||
      type == SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100 ||
      type == SPV_EXT_INST_TYPE_DEBUGINFO) {
    return true;
  }
  return false;
}
