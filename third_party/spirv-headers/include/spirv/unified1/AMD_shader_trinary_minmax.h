// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_AMD_shader_trinary_minmax_H_
#define SPIRV_UNIFIED1_AMD_shader_trinary_minmax_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AMD_shader_trinary_minmaxRevision = 4,
    AMD_shader_trinary_minmaxRevision_BitWidthPadding = 0x7fffffff
};

enum AMD_shader_trinary_minmaxInstructions {
    AMD_shader_trinary_minmaxFMin3AMD = 1,
    AMD_shader_trinary_minmaxUMin3AMD = 2,
    AMD_shader_trinary_minmaxSMin3AMD = 3,
    AMD_shader_trinary_minmaxFMax3AMD = 4,
    AMD_shader_trinary_minmaxUMax3AMD = 5,
    AMD_shader_trinary_minmaxSMax3AMD = 6,
    AMD_shader_trinary_minmaxFMid3AMD = 7,
    AMD_shader_trinary_minmaxUMid3AMD = 8,
    AMD_shader_trinary_minmaxSMid3AMD = 9,
    AMD_shader_trinary_minmaxInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_AMD_shader_trinary_minmax_H_
