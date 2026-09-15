// SPDX-FileCopyrightText: 2025 Arm Ltd.
// SPDX-License-Identifier: MIT

#ifndef SPIRV_UNIFIED1_ArmExperimentalMLOperations_H_
#define SPIRV_UNIFIED1_ArmExperimentalMLOperations_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ArmExperimentalMLOperationsRevision = 1,
    ArmExperimentalMLOperationsRevision_BitWidthPadding = 0x7fffffff
};

enum ArmExperimentalMLOperationsInstructions {
    ArmExperimentalMLOperationsCALL = 0,
    ArmExperimentalMLOperationsInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_ArmExperimentalMLOperations_H_
