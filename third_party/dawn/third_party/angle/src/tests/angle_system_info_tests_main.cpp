//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Simple unit test suite that prints out system info as JSON.
//
// Example output for a Windows/NV machine:
//
// {
//     "activeGPUIndex": 0,
//     "isOptimus": false,
//     "isMacSwitchable": false,
//     "machineManufacturer": "",
//     "machineModelVersion": "",
//     "gpus": [
//         {
//             "vendorId": 4318,
//             "deviceId": 7040,
//             "driverVendor": "NVIDIA Corporation",
//             "driverVersion": "452.6.0.0",
//             "driverDate": "",
//             "detailedDriverVersion": {
//                 "major": 452,
//                 "minor": 6,
//                 "subMinor": 0,
//                 "patch": 0
//             }
//         }
//     ]
// }

#include "gpu_info_util/SystemInfo.h"

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <cstdlib>

#include "common/debug.h"
#if defined(ANGLE_ENABLE_VULKAN)
#    include "gpu_info_util/SystemInfo_vulkan.h"
#endif  // defined(ANGLE_ENABLE_VULKAN)

namespace js = rapidjson;

bool gFailedToFindGPU;

constexpr char kRenderTestOutputDir[] = "--render-test-output-dir=";

int main(int argc, char **argv)
{
    angle::SystemInfo info;

    bool useVulkan      = false;
    bool listTests      = false;
    bool useSwiftShader = false;
    std::string output_dir;

    for (int arg = 1; arg < argc; ++arg)
    {
        if (strcmp(argv[arg], "--vulkan") == 0)
        {
            useVulkan = true;
        }
        else if (strcmp(argv[arg], "--gtest_list_tests") == 0)
        {
            listTests = true;
        }
        else if (strcmp(argv[arg], "--swiftshader") == 0)
        {
            useSwiftShader = true;
        }
        else if (strstr(argv[arg], kRenderTestOutputDir))
        {
            output_dir = argv[arg] + strlen(kRenderTestOutputDir);
        }
    }

    if (listTests)
    {
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    if (useVulkan)
    {
#if defined(ANGLE_ENABLE_VULKAN)
        angle::vk::ICD preferredICD =
            useSwiftShader ? angle::vk::ICD::SwiftShader : angle::vk::ICD::Default;
        angle::GetSystemInfoVulkanWithICD(&info, preferredICD);
#else
        ANGLE_UNUSED_VARIABLE(useSwiftShader);
        printf("Vulkan not supported.\n");
        return EXIT_FAILURE;
#endif  // defined(ANGLE_ENABLE_VULKAN)
    }
    else
    {
        angle::GetSystemInfo(&info);
    }

    if (info.gpus.empty())
    {
        gFailedToFindGPU = true;
    }

    js::Document doc;
    doc.SetObject();

    js::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("activeGPUIndex", info.activeGPUIndex, allocator);
    doc.AddMember("isOptimus", info.isOptimus, allocator);
    doc.AddMember("isMacSwitchable", info.isMacSwitchable, allocator);

    js::Value machineManufacturer;
    machineManufacturer.SetString(info.machineManufacturer.c_str(), allocator);
    doc.AddMember("machineManufacturer", machineManufacturer, allocator);

    js::Value machineModelVersion;
    machineModelVersion.SetString(info.machineModelVersion.c_str(), allocator);
    doc.AddMember("machineModelVersion", machineModelVersion, allocator);

    js::Value androidSdkLevel;
    androidSdkLevel.SetInt(info.androidSdkLevel);
    doc.AddMember("androidSdkLevel", androidSdkLevel, allocator);

    js::Value gpus;
    gpus.SetArray();

    for (const angle::GPUDeviceInfo &gpu : info.gpus)
    {
        js::Value obj;
        obj.SetObject();

        obj.AddMember("vendorId", gpu.vendorId, allocator);
        obj.AddMember("deviceId", gpu.deviceId, allocator);

        js::Value driverVendor;
        driverVendor.SetString(gpu.driverVendor.c_str(), allocator);
        obj.AddMember("driverVendor", driverVendor, allocator);

        js::Value driverVersion;
        driverVersion.SetString(gpu.driverVersion.c_str(), allocator);
        obj.AddMember("driverVersion", driverVersion, allocator);

        js::Value driverDate;
        driverDate.SetString(gpu.driverDate.c_str(), allocator);
        obj.AddMember("driverDate", driverDate, allocator);

        js::Value versionInfo;
        versionInfo.SetObject();
        versionInfo.AddMember("major", gpu.detailedDriverVersion.major, allocator);
        versionInfo.AddMember("minor", gpu.detailedDriverVersion.minor, allocator);
        versionInfo.AddMember("subMinor", gpu.detailedDriverVersion.subMinor, allocator);
        versionInfo.AddMember("patch", gpu.detailedDriverVersion.patch, allocator);
        obj.AddMember("detailedDriverVersion", versionInfo, allocator);

        gpus.PushBack(obj, allocator);
    }

    doc.AddMember("gpus", gpus, allocator);

    js::StringBuffer buffer;
    js::PrettyWriter<js::StringBuffer> writer(buffer);
    doc.Accept(writer);

    const char *output = buffer.GetString();
    printf("%s\n", output);

    if (!output_dir.empty())
    {
        std::string outputFile = output_dir + "/angle_system_info.json";
        FILE *fp               = fopen(outputFile.c_str(), "w");
        if (fp)
        {
            fwrite(output, sizeof(char), strlen(output), fp);
            fclose(fp);
        }
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(ANGLE, SystemInfo)
{
    if (gFailedToFindGPU)
    {
        FAIL() << "Failed to find GPU info.";
    }
}