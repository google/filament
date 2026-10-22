// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_NonSemanticDebugPrintf_H_
#define SPIRV_UNIFIED1_NonSemanticDebugPrintf_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NonSemanticDebugPrintfRevision = 1,
    NonSemanticDebugPrintfRevision_BitWidthPadding = 0x7fffffff
};

enum NonSemanticDebugPrintfInstructions {
    NonSemanticDebugPrintfDebugPrintf = 1,
    NonSemanticDebugPrintfInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_NonSemanticDebugPrintf_H_
