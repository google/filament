//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "common/apple_platform_utils.h"

#include "common/debug.h"
#include "common/system_utils.h"

#include <Foundation/Foundation.h>
#include <Metal/Metal.h>

namespace angle
{

bool IsMetalRendererAvailable()
{
#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
    static bool queriedMachineModel    = false;
    static bool machineModelSufficient = true;

    if (!queriedMachineModel)
    {
        queriedMachineModel = true;

        std::string fullMachineModel;
        if (GetMacosMachineModel(&fullMachineModel))
        {
            using MachineModelVersion = std::pair<int32_t, int32_t>;

            std::string name;
            MachineModelVersion version;
            ParseMacMachineModel(fullMachineModel, &name, &version.first, &version.second);

            std::optional<MachineModelVersion> minVersion;
            if (name == "MacBookAir")
            {
                minVersion = {8, 1};  // MacBook Air (Retina, 13-inch, 2018)
            }
            else if (name == "MacBookPro")
            {
                minVersion = {13, 1};  // MacBook Pro (13-inch, 2016)
            }
            else if (name == "MacBook")
            {
                minVersion = {9, 1};  // MacBook (Retina, 12-inch, Early 2016)
            }
            else if (name == "Macmini")
            {
                minVersion = {8, 1};  // Mac mini (2018)
            }
            else if (name == "iMac")
            {
                minVersion = {17, 1};  // iMac (Retina 5K, 27-inch, Late 2015)
            }
            else if (name == "iMacPro")
            {
                minVersion = {1, 1};  // iMac Pro
            }

            if (minVersion.has_value() && version < minVersion.value())
            {
                WARN() << "Disabling Metal because machine model \"" << fullMachineModel
                       << "\" is below the minium supported version.";
                machineModelSufficient = false;
            }
        }
    }

    if (!machineModelSufficient)
    {
        ASSERT(queriedMachineModel);
        return false;
    }
#endif

#if defined(ANGLE_PLATFORM_MACOS) && defined(__aarch64__)
    NSOperatingSystemVersion systemVersion = [[NSProcessInfo processInfo] operatingSystemVersion];
    if (systemVersion.majorVersion == 15 && systemVersion.minorVersion == 0)
    {
        // On ARM64 MacOS 15.0.x, Metal Shader with newLibraryWithSource didn't work,
        // if the executable path contains non-ASCII characters.
        // Bug: https://issues.chromium.org/issues/389559087
        std::string executableName = GetExecutablePath();
        for (char c : executableName)
        {
            if (static_cast<unsigned char>(c) > 127)
            {
                return false;
            }
        }
    }
#endif

    static bool gpuFamilySufficient = []() -> bool {
        ANGLE_APPLE_OBJC_SCOPE
        {
            auto device = [MTLCreateSystemDefaultDevice() ANGLE_APPLE_AUTORELEASE];
            if (!device)
            {
                return false;
            }
#if TARGET_OS_MACCATALYST && __IPHONE_OS_VERSION_MIN_REQUIRED < 160000
            // Devices in family 1, such as MacBookPro11,4, cannot use ANGLE's Metal backend.
            return [device supportsFamily:MTLGPUFamilyMacCatalyst2];
#elif TARGET_OS_MACCATALYST || TARGET_OS_OSX
            // Devices in family 1, such as MacBookPro11,4, cannot use ANGLE's Metal backend.
            return [device supportsFamily:MTLGPUFamilyMac2];
#else
            // Devices starting with A9 onwards are supported. Simulator is supported as per
            // definition that running simulator on Mac Family 1 devices is not supported.
            return true;
#endif
        }
    }();
    return gpuFamilySufficient;
}

#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
bool GetMacosMachineModel(std::string *outMachineModel)
{
#    if TARGET_OS_OSX && __MAC_OS_X_VERSION_MIN_REQUIRED < 120000
    const mach_port_t mainPort = kIOMasterPortDefault;
#    else
    const mach_port_t mainPort = kIOMainPortDefault;
#    endif
    io_service_t platformExpert =
        IOServiceGetMatchingService(mainPort, IOServiceMatching("IOPlatformExpertDevice"));

    if (platformExpert == IO_OBJECT_NULL)
    {
        return false;
    }

    CFDataRef modelData = static_cast<CFDataRef>(
        IORegistryEntryCreateCFProperty(platformExpert, CFSTR("model"), kCFAllocatorDefault, 0));
    if (modelData == nullptr)
    {
        IOObjectRelease(platformExpert);
        return false;
    }

    *outMachineModel = reinterpret_cast<const char *>(CFDataGetBytePtr(modelData));

    IOObjectRelease(platformExpert);
    CFRelease(modelData);

    return true;
}

bool ParseMacMachineModel(const std::string &identifier,
                          std::string *type,
                          int32_t *major,
                          int32_t *minor)
{
    size_t numberLoc = identifier.find_first_of("0123456789");
    if (numberLoc == std::string::npos)
    {
        return false;
    }

    size_t commaLoc = identifier.find(',', numberLoc);
    if (commaLoc == std::string::npos || commaLoc >= identifier.size())
    {
        return false;
    }

    const char *numberPtr = &identifier[numberLoc];
    const char *commaPtr  = &identifier[commaLoc + 1];
    char *endPtr          = nullptr;

    int32_t majorTmp = static_cast<int32_t>(std::strtol(numberPtr, &endPtr, 10));
    if (endPtr == numberPtr)
    {
        return false;
    }

    int32_t minorTmp = static_cast<int32_t>(std::strtol(commaPtr, &endPtr, 10));
    if (endPtr == commaPtr)
    {
        return false;
    }

    *major = majorTmp;
    *minor = minorTmp;
    *type  = identifier.substr(0, numberLoc);

    return true;
}
#endif

}  // namespace angle
