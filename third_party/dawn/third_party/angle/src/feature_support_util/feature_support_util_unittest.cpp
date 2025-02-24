//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util_unittest.cpp: Unit test for the feature-support utility.

#include <gtest/gtest.h>

#include "../gpu_info_util/SystemInfo.h"
#include "feature_support_util.h"

using namespace angle;

constexpr char kMfr[]   = "MfrFoo";
constexpr char kModel[] = "ModelX";

class FeatureSupportUtilTest : public testing::Test
{
  protected:
    FeatureSupportUtilTest()
    {
        mSystemInfo.machineManufacturer = kMfr;
        mSystemInfo.machineModelName    = kModel;
        mSystemInfo.gpus.resize(1);
        mSystemInfo.gpus[0].vendorId              = 123;
        mSystemInfo.gpus[0].deviceId              = 234;
        mSystemInfo.gpus[0].driverVendor          = "GPUVendorA";
        mSystemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 4};
    }

    SystemInfo mSystemInfo;
};

// Test the ANGLEGetFeatureSupportUtilAPIVersion function
TEST_F(FeatureSupportUtilTest, APIVersion)
{
    unsigned int versionToUse;
    unsigned int zero = 0;
    unsigned int lowestMinusOne =
        (kFeatureVersion_LowestSupported > 1) ? kFeatureVersion_LowestSupported - 1 : zero;

    versionToUse = kFeatureVersion_LowestSupported;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_LowestSupported, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);

    versionToUse = zero;
    EXPECT_FALSE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(zero, versionToUse);

    versionToUse = lowestMinusOne;
    EXPECT_FALSE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(lowestMinusOne, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported + 1;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);
}

// Test the ANGLEAddDeviceInfoToSystemInfo function
TEST_F(FeatureSupportUtilTest, SystemInfo)
{
    SystemInfo systemInfo          = mSystemInfo;
    systemInfo.machineManufacturer = "BAD";
    systemInfo.machineModelName    = "BAD";

    ANGLEAddDeviceInfoToSystemInfo(kMfr, kModel, &systemInfo);
    EXPECT_EQ(kMfr, systemInfo.machineManufacturer);
    EXPECT_EQ(kModel, systemInfo.machineModelName);
}

// Test the ANGLEAndroidParseRulesString function
TEST_F(FeatureSupportUtilTest, ParseRules)
{
    constexpr char kRulesFileContents[] = R"rulefile(
{
    "Rules" : [
        {
            "Rule" : "Default Rule (i.e. do not use ANGLE)",
            "UseANGLE" : false
        }
    ]
}
)rulefile";
    RulesHandle rulesHandle             = nullptr;
    int rulesVersion                    = 0;
    EXPECT_TRUE(ANGLEAndroidParseRulesString(kRulesFileContents, &rulesHandle, &rulesVersion));
    EXPECT_NE(nullptr, rulesHandle);
    ANGLEFreeRulesHandle(rulesHandle);
}

// Test the ANGLEAndroidParseRulesString and ANGLEShouldBeUsedForApplication functions
TEST_F(FeatureSupportUtilTest, TestRuleProcessing)
{
    SystemInfo systemInfo = mSystemInfo;

    constexpr char kRulesFileContents[] = R"rulefile(
{
    "Rules" : [
        {
            "Rule" : "Default Rule (i.e. do not use ANGLE)",
            "UseANGLE" : false
        },
        {
            "Rule" : "Supported application(s)",
            "UseANGLE" : true,
            "Applications" : [
                {
                    "AppName" : "com.isvA.app1"
                }
            ]
        },
        {
            "Rule" : "Exceptions for bad drivers(s)",
            "UseANGLE" : false,
            "Applications" : [
                {
                    "AppName" : "com.isvA.app1"
                }
            ],
            "Devices" : [
                {
                    "Manufacturer" : "MfrFoo",
                    "Model" : "ModelX",
                    "GPUs" : [
                        {
                            "Vendor" : "GPUVendorA",
                            "DeviceId" : 234,
                            "VerMajor" : 1, "VerMinor" : 2, "VerSubMinor" : 3, "VerPatch" : 4
                        }
                    ]
                }
            ]
        }
    ]
}
)rulefile";
    RulesHandle rulesHandle             = nullptr;
    int rulesVersion                    = 0;
    EXPECT_TRUE(ANGLEAndroidParseRulesString(kRulesFileContents, &rulesHandle, &rulesVersion));
    EXPECT_NE(nullptr, rulesHandle);

    // Test app1 with a SystemInfo that has an unsupported driver--should fail:
    constexpr char kApp1[] = "com.isvA.app1";
    EXPECT_FALSE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, kApp1));

    // Test app1 with a SystemInfo that has a supported driver--should pass:
    systemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 5};
    EXPECT_TRUE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, kApp1));

    // Test unsupported app2--should fail:
    constexpr char kApp2[] = "com.isvB.app2";
    EXPECT_FALSE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, kApp2));

    // Free the rules data structures:
    ANGLEFreeRulesHandle(rulesHandle);
}
