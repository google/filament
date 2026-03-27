// SPDX-FileCopyrightText: 2022-2025 Arm Ltd.
// SPDX-License-Identifier: MIT

#ifndef SPIRV_UNIFIED1_ArmMotionEngine_100_H_
#define SPIRV_UNIFIED1_ArmMotionEngine_100_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ArmMotionEngineVersion = 100,
    ArmMotionEngineVersion_BitWidthPadding = 0x7fffffff
};
enum {
    ArmMotionEngineRevision = 1,
    ArmMotionEngineRevision_BitWidthPadding = 0x7fffffff
};

enum ArmMotionEngineInstructions {
    ArmMotionEngineMIN_SAD = 0,
    ArmMotionEngineMIN_SAD_COST = 1,
    ArmMotionEngineRAW_SAD = 2,
    ArmMotionEngineInstructionsMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_ArmMotionEngine_100_H_
