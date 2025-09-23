// SPDX-FileCopyrightText: 2020-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
// 
// 

#ifndef SPIRV_UNIFIED1_NonSemanticClspvReflection_H_
#define SPIRV_UNIFIED1_NonSemanticClspvReflection_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NonSemanticClspvReflectionRevision = 7,
    NonSemanticClspvReflectionRevision_BitWidthPadding = 0x7fffffff
};

enum NonSemanticClspvReflectionInstructions {
    NonSemanticClspvReflectionKernel = 1,
    NonSemanticClspvReflectionArgumentInfo = 2,
    NonSemanticClspvReflectionArgumentStorageBuffer = 3,
    NonSemanticClspvReflectionArgumentUniform = 4,
    NonSemanticClspvReflectionArgumentPodStorageBuffer = 5,
    NonSemanticClspvReflectionArgumentPodUniform = 6,
    NonSemanticClspvReflectionArgumentPodPushConstant = 7,
    NonSemanticClspvReflectionArgumentSampledImage = 8,
    NonSemanticClspvReflectionArgumentStorageImage = 9,
    NonSemanticClspvReflectionArgumentSampler = 10,
    NonSemanticClspvReflectionArgumentWorkgroup = 11,
    NonSemanticClspvReflectionSpecConstantWorkgroupSize = 12,
    NonSemanticClspvReflectionSpecConstantGlobalOffset = 13,
    NonSemanticClspvReflectionSpecConstantWorkDim = 14,
    NonSemanticClspvReflectionPushConstantGlobalOffset = 15,
    NonSemanticClspvReflectionPushConstantEnqueuedLocalSize = 16,
    NonSemanticClspvReflectionPushConstantGlobalSize = 17,
    NonSemanticClspvReflectionPushConstantRegionOffset = 18,
    NonSemanticClspvReflectionPushConstantNumWorkgroups = 19,
    NonSemanticClspvReflectionPushConstantRegionGroupOffset = 20,
    NonSemanticClspvReflectionConstantDataStorageBuffer = 21,
    NonSemanticClspvReflectionConstantDataUniform = 22,
    NonSemanticClspvReflectionLiteralSampler = 23,
    NonSemanticClspvReflectionPropertyRequiredWorkgroupSize = 24,
    NonSemanticClspvReflectionSpecConstantSubgroupMaxSize = 25,
    NonSemanticClspvReflectionArgumentPointerPushConstant = 26,
    NonSemanticClspvReflectionArgumentPointerUniform = 27,
    NonSemanticClspvReflectionProgramScopeVariablesStorageBuffer = 28,
    NonSemanticClspvReflectionProgramScopeVariablePointerRelocation = 29,
    NonSemanticClspvReflectionImageArgumentInfoChannelOrderPushConstant = 30,
    NonSemanticClspvReflectionImageArgumentInfoChannelDataTypePushConstant = 31,
    NonSemanticClspvReflectionImageArgumentInfoChannelOrderUniform = 32,
    NonSemanticClspvReflectionImageArgumentInfoChannelDataTypeUniform = 33,
    NonSemanticClspvReflectionArgumentStorageTexelBuffer = 34,
    NonSemanticClspvReflectionArgumentUniformTexelBuffer = 35,
    NonSemanticClspvReflectionConstantDataPointerPushConstant = 36,
    NonSemanticClspvReflectionProgramScopeVariablePointerPushConstant = 37,
    NonSemanticClspvReflectionPrintfInfo = 38,
    NonSemanticClspvReflectionPrintfBufferStorageBuffer = 39,
    NonSemanticClspvReflectionPrintfBufferPointerPushConstant = 40,
    NonSemanticClspvReflectionNormalizedSamplerMaskPushConstant = 41,
    NonSemanticClspvReflectionWorkgroupVariableSize = 42,
    NonSemanticClspvReflectionInstructionsMax = 0x7fffffff
};


enum NonSemanticClspvReflectionKernelPropertyFlags {
    NonSemanticClspvReflectionNone = 0x0,
    NonSemanticClspvReflectionMayUsePrintf = 0x1,
    NonSemanticClspvReflectionKernelPropertyFlagsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_NonSemanticClspvReflection_H_
