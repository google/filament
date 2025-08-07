// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_AMD_gcn_shader_H_
#define SPIRV_UNIFIED1_AMD_gcn_shader_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AMD_gcn_shaderRevision = 2,
    AMD_gcn_shaderRevision_BitWidthPadding = 0x7fffffff
};

enum AMD_gcn_shaderInstructions {
    AMD_gcn_shaderCubeFaceIndexAMD = 1,
    AMD_gcn_shaderCubeFaceCoordAMD = 2,
    AMD_gcn_shaderTimeAMD = 3,
    AMD_gcn_shaderInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_AMD_gcn_shader_H_
