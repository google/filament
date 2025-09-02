// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_AMD_shader_explicit_vertex_parameter_H_
#define SPIRV_UNIFIED1_AMD_shader_explicit_vertex_parameter_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AMD_shader_explicit_vertex_parameterRevision = 4,
    AMD_shader_explicit_vertex_parameterRevision_BitWidthPadding = 0x7fffffff
};

enum AMD_shader_explicit_vertex_parameterInstructions {
    AMD_shader_explicit_vertex_parameterInterpolateAtVertexAMD = 1,
    AMD_shader_explicit_vertex_parameterInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_AMD_shader_explicit_vertex_parameter_H_
