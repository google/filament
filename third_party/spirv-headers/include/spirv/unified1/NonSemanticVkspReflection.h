// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_NonSemanticVkspReflection_H_
#define SPIRV_UNIFIED1_NonSemanticVkspReflection_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NonSemanticVkspReflectionRevision = 4,
    NonSemanticVkspReflectionRevision_BitWidthPadding = 0x7fffffff
};

enum NonSemanticVkspReflectionInstructions {
    NonSemanticVkspReflectionConfiguration = 1,
    NonSemanticVkspReflectionStartCounter = 2,
    NonSemanticVkspReflectionStopCounter = 3,
    NonSemanticVkspReflectionPushConstants = 4,
    NonSemanticVkspReflectionSpecializationMapEntry = 5,
    NonSemanticVkspReflectionDescriptorSetBuffer = 6,
    NonSemanticVkspReflectionDescriptorSetImage = 7,
    NonSemanticVkspReflectionDescriptorSetSampler = 8,
    NonSemanticVkspReflectionInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_NonSemanticVkspReflection_H_
