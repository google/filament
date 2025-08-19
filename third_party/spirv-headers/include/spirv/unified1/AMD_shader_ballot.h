// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_AMD_shader_ballot_H_
#define SPIRV_UNIFIED1_AMD_shader_ballot_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AMD_shader_ballotRevision = 5,
    AMD_shader_ballotRevision_BitWidthPadding = 0x7fffffff
};

enum AMD_shader_ballotInstructions {
    AMD_shader_ballotSwizzleInvocationsAMD = 1,
    AMD_shader_ballotSwizzleInvocationsMaskedAMD = 2,
    AMD_shader_ballotWriteInvocationAMD = 3,
    AMD_shader_ballotMbcntAMD = 4,
    AMD_shader_ballotInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_AMD_shader_ballot_H_
