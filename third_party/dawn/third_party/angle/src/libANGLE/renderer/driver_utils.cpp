//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// driver_utils.h : provides more information about current driver.

#include <algorithm>

#include "libANGLE/renderer/driver_utils.h"

#include "common/android_util.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "gpu_info_util/SystemInfo.h"

#if defined(ANGLE_PLATFORM_LINUX)
#    include <sys/utsname.h>
#endif

namespace rx
{
// Intel
// Referenced from
// https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/include/pci_ids/crocus_pci_ids.h
// https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/include/pci_ids/iris_pci_ids.h
namespace
{
// gen6
const uint16_t SandyBridge[] = {
    0x0102, 0x0106, 0x010A,         // snb_gt1
    0x0112, 0x0122, 0x0116, 0x0126  // snb_gt2
};

// gen7
const uint16_t IvyBridge[] = {
    0x0152, 0x0156, 0x015A,  // ivb_gt1
    0x0162, 0x0166, 0x016A   // ivb_gt2
};

// gen 7.5
const uint16_t Haswell[] = {
    0x0402, 0x0406, 0x040A, 0x040B, 0x040E, 0x0C02, 0x0C06, 0x0C0A, 0x0C0B, 0x0C0E,
    0x0A02, 0x0A06, 0x0A0A, 0x0A0B, 0x0A0E, 0x0D02, 0x0D06, 0x0D0A, 0x0D0B, 0x0D0E,  // hsw_gt1
    0x0412, 0x0416, 0x041A, 0x041B, 0x041E, 0x0C12, 0x0C16, 0x0C1A, 0x0C1B, 0x0C1E,
    0x0A12, 0x0A16, 0x0A1A, 0x0A1B, 0x0A1E, 0x0D12, 0x0D16, 0x0D1A, 0x0D1B, 0x0D1E,  // hsw_gt2
    0x0422, 0x0426, 0x042A, 0x042B, 0x042E, 0x0C22, 0x0C26, 0x0C2A, 0x0C2B, 0x0C2E,
    0x0A22, 0x0A26, 0x0A2A, 0x0A2B, 0x0A2E, 0x0D22, 0x0D26, 0x0D2A, 0x0D2B, 0x0D2E  // hsw_gt3
};

// gen8
const uint16_t Broadwell[] = {
    0x1602, 0x1606, 0x160A, 0x160B, 0x160D, 0x160E,  // bdw_gt1
    0x1612, 0x1616, 0x161A, 0x161B, 0x161D, 0x161E,  // bdw_gt2
    0x1622, 0x1626, 0x162A, 0x162B, 0x162D, 0x162E   // bdw_gt3
};

const uint16_t CherryView[] = {0x22B0, 0x22B1, 0x22B2, 0x22B3};

// gen9
const uint16_t Skylake[] = {
    0x1902, 0x1906, 0x190A, 0x190B, 0x190E,                                          // skl_gt1
    0x1912, 0x1913, 0x1915, 0x1916, 0x1917, 0x191A, 0x191B, 0x191D, 0x191E, 0x1921,  // skl_gt2
    0x1923, 0x1926, 0x1927, 0x192B, 0x192D,                                          // skl_gt3
    0x192A, 0x1932, 0x193A, 0x193B, 0x193D                                           // skl_gt4
};

// gen9lp
const uint16_t Broxton[] = {0x0A84, 0x1A84, 0x1A85, 0x5A84, 0x5A85};

const uint16_t GeminiLake[] = {0x3184, 0x3185};

// gen9p5
const uint16_t KabyLake[] = {
    // Kaby Lake
    0x5902, 0x5906, 0x5908, 0x590A, 0x590B, 0x590E,                  // kbl_gt1
    0x5913, 0x5915,                                                  // kbl_gt1_5
    0x5912, 0x5916, 0x5917, 0x591A, 0x591B, 0x591D, 0x591E, 0x5921,  // kbl_gt2
    0x5923, 0x5926, 0x5927,                                          // kbl_gt3
    0x593B,                                                          // kbl_gt4
    // Amber Lake
    0x591C, 0x87C0  // kbl_gt2
};

const uint16_t CoffeeLake[] = {
    // Amber Lake
    0x87CA,  // cfl_gt2

    // Coffee Lake
    0x3E90, 0x3E93, 0x3E99, 0x3E9C,                                  // cfl_gt1
    0x3E91, 0x3E92, 0x3E94, 0x3E96, 0x3E98, 0x3E9A, 0x3E9B, 0x3EA9,  // cfl_gt2
    0x3EA5, 0x3EA6, 0x3EA7, 0x3EA8,                                  // cfl_gt3

    // Whisky Lake
    0x3EA1, 0x3EA4,  // cfl_gt1
    0x3EA0, 0x3EA3,  // cfl_gt2
    0x3EA2,          // cfl_gt3

    // Comet Lake
    0x9B21, 0x9BA0, 0x9BA2, 0x9BA4, 0x9BA5, 0x9BA8, 0x9BAA, 0x9BAB, 0x9BAC,          // cfl_gt1
    0x9B41, 0x9BC0, 0x9BC2, 0x9BC4, 0x9BC5, 0x9BC6, 0x9BC8, 0x9BCA, 0x9BCB, 0x9BCC,  // cfl_gt2
    0x9BE6, 0x9BF6                                                                   // cfl_gt2
};

const uint16_t MeteorLake[] = {0x7d40, 0x7d45, 0x7d55, 0x7d60, 0x7dd5};

const uint16_t IntelGen11[] = {
    // Ice Lake
    0x8A71,                                  // icl_gt0_5
    0x8A56, 0x8A58, 0x8A5B, 0x8A5D,          // icl_gt1
    0x8A54, 0x8A57, 0x8A59, 0x8A5A, 0x8A5C,  // icl_gt1_5
    0x8A50, 0x8A51, 0x8A52, 0x8A53,          // icl_gt2

    // Elkhart Lake
    0x4541, 0x4551, 0x4555, 0x4557, 0x4570, 0x4571,

    // Jasper Lake
    0x4E51, 0x4E55, 0x4E57, 0x4E61, 0x4E71};

const uint16_t IntelGen12[] = {
    // Rocket Lake
    0x4C8C,                          // rkl_gt05
    0x4C8A, 0x4C8B, 0x4C90, 0x4C9A,  // rkl_gt1

    // Alder Lake
    0x468B,                                                                  // adl_gt05
    0x4680, 0x4682, 0x4688, 0x468A, 0x4690, 0x4692, 0x4693,                  // adl_gt1
    0x4626, 0x4628, 0x462A, 0x46A0, 0x46A1, 0x46A2, 0x46A3, 0x46A6, 0x46A8,  // adl_gt2
    0x46AA, 0x46B0, 0x46B1, 0x46B2, 0x46B3, 0x46C0, 0x46C1, 0x46C2, 0x46C3,  // adl_gt2
    0x46D0, 0x46D1, 0x46D2, 0x46D3, 0x46D4,                                  // adl_n

    // Tiger Lake
    0x9A60, 0x9A68, 0x9A70,                                          // tgl_gt1
    0x9A40, 0x9A49, 0x9A59, 0x9A78, 0x9AC0, 0x9AC9, 0x9AD9, 0x9AF8,  // tgl_gt2

    // Raptor Lake
    0xA780, 0xA781, 0xA782, 0xA783, 0xA788, 0xA789, 0xA78A, 0xA78B,                  // rpl
    0xA720, 0xA721, 0xA7A0, 0xA7A1, 0xA7A8, 0xA7A9, 0xA7AA, 0xA7AB, 0xA7AC, 0xA7AD,  // rpl_p

    // DG1
    0x4905, 0x4906, 0x4907, 0x4908, 0x4909};
}  // anonymous namespace

bool IsSandyBridge(uint32_t DeviceId)
{
    return std::find(std::begin(SandyBridge), std::end(SandyBridge), DeviceId) !=
           std::end(SandyBridge);
}

bool IsIvyBridge(uint32_t DeviceId)
{
    return std::find(std::begin(IvyBridge), std::end(IvyBridge), DeviceId) != std::end(IvyBridge);
}

bool IsHaswell(uint32_t DeviceId)
{
    return std::find(std::begin(Haswell), std::end(Haswell), DeviceId) != std::end(Haswell);
}

bool IsBroadwell(uint32_t DeviceId)
{
    return std::find(std::begin(Broadwell), std::end(Broadwell), DeviceId) != std::end(Broadwell);
}

bool IsCherryView(uint32_t DeviceId)
{
    return std::find(std::begin(CherryView), std::end(CherryView), DeviceId) !=
           std::end(CherryView);
}

bool IsSkylake(uint32_t DeviceId)
{
    return std::find(std::begin(Skylake), std::end(Skylake), DeviceId) != std::end(Skylake);
}

bool IsBroxton(uint32_t DeviceId)
{
    return std::find(std::begin(Broxton), std::end(Broxton), DeviceId) != std::end(Broxton);
}

bool IsKabyLake(uint32_t DeviceId)
{
    return std::find(std::begin(KabyLake), std::end(KabyLake), DeviceId) != std::end(KabyLake);
}

bool IsGeminiLake(uint32_t DeviceId)
{
    return std::find(std::begin(GeminiLake), std::end(GeminiLake), DeviceId) !=
           std::end(GeminiLake);
}

bool IsCoffeeLake(uint32_t DeviceId)
{
    return std::find(std::begin(CoffeeLake), std::end(CoffeeLake), DeviceId) !=
           std::end(CoffeeLake);
}

bool IsMeteorLake(uint32_t DeviceId)
{
    return std::find(std::begin(MeteorLake), std::end(MeteorLake), DeviceId) !=
           std::end(MeteorLake);
}

bool Is9thGenIntel(uint32_t DeviceId)
{
    return IsSkylake(DeviceId) || IsBroxton(DeviceId) || IsKabyLake(DeviceId);
}

bool Is11thGenIntel(uint32_t DeviceId)
{
    return std::find(std::begin(IntelGen11), std::end(IntelGen11), DeviceId) !=
           std::end(IntelGen11);
}

bool Is12thGenIntel(uint32_t DeviceId)
{
    return std::find(std::begin(IntelGen12), std::end(IntelGen12), DeviceId) !=
           std::end(IntelGen12);
}

std::string GetVendorString(uint32_t vendorId)
{
    switch (vendorId)
    {
        case VENDOR_ID_AMD:
            return "AMD";
        case VENDOR_ID_ARM:
            return "ARM";
        case VENDOR_ID_APPLE:
            return "Apple";
        case VENDOR_ID_BROADCOM:
            return "Broadcom";
        case VENDOR_ID_GOOGLE:
            return "Google";
        case VENDOR_ID_INTEL:
            return "Intel";
        case VENDOR_ID_MESA:
            return "Mesa";
        case VENDOR_ID_MICROSOFT:
            return "Microsoft";
        case VENDOR_ID_NVIDIA:
            return "NVIDIA";
        case VENDOR_ID_POWERVR:
            return "Imagination Technologies";
        case VENDOR_ID_QUALCOMM_DXGI:
        case VENDOR_ID_QUALCOMM:
            return "Qualcomm";
        case VENDOR_ID_SAMSUNG:
            return "Samsung Electronics Co., Ltd.";
        case VENDOR_ID_VIVANTE:
            return "Vivante";
        case VENDOR_ID_VMWARE:
            return "VMware";
        case VENDOR_ID_VIRTIO:
            return "VirtIO";
        case 0xba5eba11:  // Mock vendor ID used for tests.
            return "Test";
        case 0:
            return "NULL";
    }

    std::stringstream s;
    s << gl::FmtHex(vendorId);
    return s.str();
}

bool operator==(const angle::VersionInfo &a, const angle::VersionTriple &b)
{
    return angle::VersionTriple(a.major, a.minor, a.subMinor) == b;
}

bool operator!=(const angle::VersionInfo &a, const angle::VersionTriple &b)
{
    return angle::VersionTriple(a.major, a.minor, a.subMinor) != b;
}

bool operator<(const angle::VersionInfo &a, const angle::VersionTriple &b)
{
    return angle::VersionTriple(a.major, a.minor, a.subMinor) < b;
}

bool operator>=(const angle::VersionInfo &a, const angle::VersionTriple &b)
{
    return angle::VersionTriple(a.major, a.minor, a.subMinor) >= b;
}

int GetAndroidSDKVersion()
{
    std::string androidSdkLevel;
    if (!angle::android::GetSystemProperty(angle::android::kSDKSystemPropertyName,
                                           &androidSdkLevel))
    {
        return 0;
    }

    return std::atoi(androidSdkLevel.c_str());
}
#if !defined(ANGLE_PLATFORM_MACOS)
OSVersion GetMacOSVersion()
{
    // Return a default version
    return OSVersion(0, 0, 0);
}
#endif

#if !ANGLE_PLATFORM_IOS_FAMILY
OSVersion GetiOSVersion()
{
    // Return a default version
    return OSVersion(0, 0, 0);
}
#endif

#if defined(ANGLE_PLATFORM_LINUX)
bool ParseLinuxOSVersion(const char *version, int *major, int *minor, int *patch)
{
    errno = 0;  // reset global error flag.
    char *next;
    *major = static_cast<int>(strtol(version, &next, 10));
    if (next == nullptr || *next != '.' || errno != 0)
    {
        return false;
    }

    *minor = static_cast<int>(strtol(next + 1, &next, 10));
    if (next == nullptr || *next != '.' || errno != 0)
    {
        return false;
    }

    *patch = static_cast<int>(strtol(next + 1, &next, 10));
    if (errno != 0)
    {
        return false;
    }

    return true;
}
#endif

OSVersion GetLinuxOSVersion()
{
#if defined(ANGLE_PLATFORM_LINUX)
    struct utsname uname_info;
    if (uname(&uname_info) != 0)
    {
        return OSVersion(0, 0, 0);
    }

    int majorVersion = 0, minorVersion = 0, patchVersion = 0;
    if (ParseLinuxOSVersion(uname_info.release, &majorVersion, &minorVersion, &patchVersion))
    {
        return OSVersion(majorVersion, minorVersion, patchVersion);
    }
#endif

    return OSVersion(0, 0, 0);
}

// There are multiple environment variables that may or may not be set during Wayland
// sessions, including WAYLAND_DISPLAY, XDG_SESSION_TYPE, and DESKTOP_SESSION
bool IsWayland()
{
    static bool checked   = false;
    static bool isWayland = false;
    if (!checked)
    {
        if (IsLinux())
        {
            if (!angle::GetEnvironmentVar("WAYLAND_DISPLAY").empty())
            {
                isWayland = true;
            }
            else if (angle::GetEnvironmentVar("XDG_SESSION_TYPE") == "wayland")
            {
                isWayland = true;
            }
            else if (angle::GetEnvironmentVar("DESKTOP_SESSION").find("wayland") !=
                     std::string::npos)
            {
                isWayland = true;
            }
        }
        checked = true;
    }
    return isWayland;
}

}  // namespace rx
