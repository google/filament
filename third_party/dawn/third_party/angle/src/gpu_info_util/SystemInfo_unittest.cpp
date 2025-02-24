//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SystemInfo_unittest.cpp: Unit tests for SystemInfo* helper functions.
//

#include "common/platform.h"
#include "gpu_info_util/SystemInfo_internal.h"

#if defined(ANGLE_PLATFORM_APPLE)
#    include "common/apple_platform_utils.h"
#endif  // defined(ANGLE_PLATFORM_APPLE)

#include <gtest/gtest.h>

using namespace angle;

namespace
{

// Test AMD Brahma driver version parsing
TEST(SystemInfoTest, AMDBrahmaVersionParsing)
{
    std::string version;

    // Check parsing fails when no version string is present.
    ASSERT_FALSE(ParseAMDBrahmaDriverVersion("I am a lumberjack.", &version));
    ASSERT_EQ("", version);

    // Check parsing when the string is just the version string, with and without dots
    ASSERT_TRUE(ParseAMDBrahmaDriverVersion("42", &version));
    ASSERT_EQ("42", version);
    ASSERT_TRUE(ParseAMDBrahmaDriverVersion("42.0.56", &version));
    ASSERT_EQ("42.0.56", version);

    // Check parsing with prefix / suffix
    ASSERT_TRUE(ParseAMDBrahmaDriverVersion("Version=42.0.56", &version));
    ASSERT_EQ("42.0.56", version);
    ASSERT_TRUE(ParseAMDBrahmaDriverVersion("42.0.56 is the version", &version));
    ASSERT_EQ("42.0.56", version);
    ASSERT_TRUE(ParseAMDBrahmaDriverVersion("42.0.56 is the version, 111", &version));
    ASSERT_EQ("42.0.56", version);
}

// Test AMD Catalyst version parsing
TEST(SystemInfoTest, AMDCatalystVersionParsing)
{
    std::string version;

    // Check parsing fails when no version string is present.
    ASSERT_FALSE(ParseAMDCatalystDriverVersion("I am a lumberjack.\nReleaseVersion=", &version));
    ASSERT_EQ("", version);

    // Check parsing fails when ReleaseVersion= is present but no number appears in the line
    ASSERT_FALSE(ParseAMDCatalystDriverVersion("11\nReleaseVersion=\n12", &version));
    ASSERT_EQ("", version);

    // Check parsing works on the simple case
    ASSERT_TRUE(ParseAMDCatalystDriverVersion("ReleaseVersion=42.0.56", &version));
    ASSERT_EQ("42.0.56", version);

    // Check parsing works if there are other lines
    ASSERT_TRUE(ParseAMDCatalystDriverVersion("11\nReleaseVersion=42.0.56\n12", &version));
    ASSERT_EQ("42.0.56", version);

    // Check parsing get the first version string
    ASSERT_TRUE(
        ParseAMDCatalystDriverVersion("ReleaseVersion=42.0.56\nReleaseVersion=0", &version));
    ASSERT_EQ("42.0.56", version);

    // Check parsing with prefix / suffix
    ASSERT_TRUE(ParseAMDCatalystDriverVersion("ReleaseVersion=version is 42.0.56", &version));
    ASSERT_EQ("42.0.56", version);
    ASSERT_TRUE(ParseAMDCatalystDriverVersion("ReleaseVersion=42.0.56 is the version", &version));
    ASSERT_EQ("42.0.56", version);
}

#if defined(ANGLE_PLATFORM_MACOS)

// Test Mac machine model parsing
TEST(SystemInfoTest, MacMachineModelParsing)
{
    std::string model;
    int32_t major = 1, minor = 2;

    // Test on the empty string, that is returned by GetMachineModel on an error
    EXPECT_FALSE(ParseMacMachineModel("", &model, &major, &minor));
    EXPECT_EQ(0U, model.length());
    EXPECT_EQ(1, major);
    EXPECT_EQ(2, minor);

    // Test on an invalid string
    EXPECT_FALSE(ParseMacMachineModel("FooBar", &model, &major, &minor));

    // Test on a MacPro model
    EXPECT_TRUE(ParseMacMachineModel("MacPro4,1", &model, &major, &minor));
    EXPECT_EQ("MacPro", model);
    EXPECT_EQ(4, major);
    EXPECT_EQ(1, minor);

    // Test on a MacBookPro model
    EXPECT_TRUE(ParseMacMachineModel("MacBookPro6,2", &model, &major, &minor));
    EXPECT_EQ("MacBookPro", model);
    EXPECT_EQ(6, major);
    EXPECT_EQ(2, minor);
}

#endif  // defined(ANGLE_PLATFORM_MACOS)

// Test Windows CM Device ID parsing
TEST(SystemInfoTest, CMDeviceIDToDeviceAndVendorID)
{
    uint32_t vendor = 0;
    uint32_t device = 0;

    // Test on a real-life CM Device ID
    EXPECT_TRUE(CMDeviceIDToDeviceAndVendorID(
        "PCI\\VEN_10DE&DEV_0FFA&SUBSYS_094B10DE&REV_A1\\4&95673C&0&0018", &vendor, &device));
    EXPECT_EQ(0x10deu, vendor);
    EXPECT_EQ(0x0ffau, device);

    // Test on a stripped-down but valid CM Device ID string
    EXPECT_TRUE(CMDeviceIDToDeviceAndVendorID("PCI\\VEN_10DE&DEV_0FFA", &vendor, &device));
    EXPECT_EQ(0x10deu, vendor);
    EXPECT_EQ(0x0ffau, device);

    // Test on a string that is too small
    EXPECT_FALSE(CMDeviceIDToDeviceAndVendorID("\\VEN_10DE&DEV_0FFA", &vendor, &device));
    EXPECT_EQ(0u, vendor);
    EXPECT_EQ(0u, device);

    // Test with invalid number
    EXPECT_FALSE(CMDeviceIDToDeviceAndVendorID("PCI\\VEN_XXXX&DEV_XXXX", &vendor, &device));
    EXPECT_EQ(0u, vendor);
    EXPECT_EQ(0u, device);
}

}  // anonymous namespace
