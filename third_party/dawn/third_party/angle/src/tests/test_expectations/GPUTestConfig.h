//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef TEST_EXPECTATIONS_GPU_TEST_CONFIG_H_
#define TEST_EXPECTATIONS_GPU_TEST_CONFIG_H_

#include <common/bitset_utils.h>

namespace angle
{

struct GPUTestConfig
{
  public:
    enum API
    {
        kAPIUnknown = 0,
        kAPID3D9,
        kAPID3D11,
        kAPIGLDesktop,
        kAPIGLES,
        kAPIVulkan,
        kAPISwiftShader,
        kAPIMetal,
        kAPIWgpu,
    };

    enum Condition
    {
        kConditionNone = 0,
        kConditionWinXP,
        kConditionWinVista,
        kConditionWin7,
        kConditionWin8,
        kConditionWin10,
        kConditionWin,
        kConditionMacLeopard,
        kConditionMacSnowLeopard,
        kConditionMacLion,
        kConditionMacMountainLion,
        kConditionMacMavericks,
        kConditionMacYosemite,
        kConditionMacElCapitan,
        kConditionMacSierra,
        kConditionMacHighSierra,
        kConditionMacMojave,
        kConditionMac,
        kConditionIOS,
        kConditionLinux,
        kConditionAndroid,
        kConditionNVIDIA,
        kConditionAMD,
        kConditionIntel,
        kConditionVMWare,
        kConditionApple,
        kConditionRelease,
        kConditionDebug,
        kConditionD3D9,
        kConditionD3D11,
        kConditionGLDesktop,
        kConditionGLES,
        kConditionVulkan,
        kConditionMetal,
        kConditionWgpu,
        kConditionNexus5X,
        kConditionPixel2OrXL,
        kConditionPixel4OrXL,
        kConditionPixel6,
        kConditionPixel7,
        kConditionFlipN2,
        kConditionMaliG710,
        kConditionGalaxyA23,
        kConditionGalaxyA34,
        kConditionGalaxyA54,
        kConditionGalaxyS22,
        kConditionGalaxyS23,
        kConditionGalaxyS24Exynos,
        kConditionGalaxyS24Qualcomm,
        kConditionFindX6,
        kConditionNVIDIAQuadroP400,
        kConditionNVIDIAGTX1660,
        kConditionPineapple,
        kConditionSwiftShader,
        kConditionPreRotation,
        kConditionPreRotation90,
        kConditionPreRotation180,
        kConditionPreRotation270,
        kConditionNoSan,
        kConditionASan,
        kConditionTSan,
        kConditionUBSan,

        kNumberOfConditions,
    };

    using ConditionArray = angle::BitSet<GPUTestConfig::kNumberOfConditions>;

    GPUTestConfig();
    GPUTestConfig(bool isSwiftShader);
    GPUTestConfig(const API &api, uint32_t preRotation);

    const GPUTestConfig::ConditionArray &getConditions() const;

  protected:
    GPUTestConfig::ConditionArray mConditions;
};

}  // namespace angle

#endif  // TEST_EXPECTATIONS_GPU_TEST_CONFIG_H_
