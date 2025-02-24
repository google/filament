//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// driver_utils.h : provides more information about current driver.

#ifndef LIBANGLE_RENDERER_DRIVER_UTILS_H_
#define LIBANGLE_RENDERER_DRIVER_UTILS_H_

#include "common/platform.h"
#include "common/platform_helpers.h"
#include "libANGLE/angletypes.h"

namespace angle
{
struct VersionInfo;
}

namespace rx
{

enum VendorID : uint32_t
{
    VENDOR_ID_UNKNOWN = 0x0,
    VENDOR_ID_AMD     = 0x1002,
    VENDOR_ID_APPLE   = 0x106B,
    VENDOR_ID_ARM     = 0x13B5,
    // Broadcom devices won't use PCI, but this is their Vulkan vendor id.
    VENDOR_ID_BROADCOM      = 0x14E4,
    VENDOR_ID_GOOGLE        = 0x1AE0,
    VENDOR_ID_INTEL         = 0x8086,
    VENDOR_ID_MESA          = 0x10005,
    VENDOR_ID_MICROSOFT     = 0x1414,
    VENDOR_ID_NVIDIA        = 0x10DE,
    VENDOR_ID_POWERVR       = 0x1010,
    VENDOR_ID_QUALCOMM_DXGI = 0x4D4F4351,
    VENDOR_ID_QUALCOMM      = 0x5143,
    VENDOR_ID_SAMSUNG       = 0x144D,
    VENDOR_ID_VIVANTE       = 0x9999,
    VENDOR_ID_VMWARE        = 0x15AD,
    VENDOR_ID_VIRTIO        = 0x1AF4,
};

enum AndroidDeviceID : uint32_t
{
    ANDROID_DEVICE_ID_UNKNOWN     = 0x0,
    ANDROID_DEVICE_ID_NEXUS5X     = 0x4010800,
    ANDROID_DEVICE_ID_PIXEL2      = 0x5040001,
    ANDROID_DEVICE_ID_PIXEL1XL    = 0x5030004,
    ANDROID_DEVICE_ID_PIXEL4      = 0x6040001,
    ANDROID_DEVICE_ID_GALAXYA23   = 0x6010901,
    ANDROID_DEVICE_ID_GALAXYS23   = 0x43050A01,
    ANDROID_DEVICE_ID_SWIFTSHADER = 0xC0DE,
};

inline bool IsAMD(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_AMD;
}

inline bool IsAppleGPU(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_APPLE;
}

inline bool IsARM(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_ARM;
}

inline bool IsBroadcom(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_BROADCOM;
}

inline bool IsIntel(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_INTEL;
}

inline bool IsGoogle(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_GOOGLE;
}

inline bool IsMicrosoft(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_MICROSOFT;
}

inline bool IsNvidia(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_NVIDIA;
}

inline bool IsPowerVR(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_POWERVR;
}

inline bool IsQualcomm(uint32_t vendorId)
{
    // Qualcomm is an unusual one. It has two different vendor IDs depending on
    // where you look. On Windows, DXGI will report the VENDOR_ID_QUALCOMM_DXGI
    // value (due to it being an ACPI device rather than PCI device), but their
    // native Vulkan driver will actually report their PCI vendor ID (the
    // VENDOR_ID_QUALCOMM value). So we have to check both, to ensure we arrive
    // at the right conclusion regardless of what source we are querying for
    // vendor information.
    return vendorId == VENDOR_ID_QUALCOMM || vendorId == VENDOR_ID_QUALCOMM_DXGI;
}

inline bool IsSamsung(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_SAMSUNG;
}

inline bool IsVivante(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_VIVANTE;
}

inline bool IsVMWare(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_VMWARE;
}

inline bool IsVirtIO(uint32_t vendorId)
{
    return vendorId == VENDOR_ID_VIRTIO;
}

inline bool IsNexus5X(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_NEXUS5X;
}

inline bool IsPixel1XL(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_PIXEL1XL;
}

inline bool IsPixel2(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_PIXEL2;
}

inline bool IsPixel4(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_PIXEL4;
}

inline bool IsGalaxyA23(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_GALAXYA23;
}

inline bool IsGalaxyS23(uint32_t vendorId, uint32_t deviceId)
{
    return IsQualcomm(vendorId) && deviceId == ANDROID_DEVICE_ID_GALAXYS23;
}

inline bool IsSwiftshader(uint32_t vendorId, uint32_t deviceId)
{
    return IsGoogle(vendorId) && deviceId == ANDROID_DEVICE_ID_SWIFTSHADER;
}

std::string GetVendorString(uint32_t vendorId);

bool IsSandyBridge(uint32_t DeviceId);
bool IsIvyBridge(uint32_t DeviceId);
bool IsHaswell(uint32_t DeviceId);
bool IsBroadwell(uint32_t DeviceId);
bool IsCherryView(uint32_t DeviceId);
bool IsSkylake(uint32_t DeviceId);
bool IsBroxton(uint32_t DeviceId);
bool IsKabyLake(uint32_t DeviceId);
bool IsGeminiLake(uint32_t DeviceId);
bool IsCoffeeLake(uint32_t DeviceId);
bool IsMeteorLake(uint32_t DeviceId);
bool Is9thGenIntel(uint32_t DeviceId);
bool Is11thGenIntel(uint32_t DeviceId);
bool Is12thGenIntel(uint32_t DeviceId);

// For ease of comparison of SystemInfo's external-facing VersionInfo with ANGLE's more commonly
// used VersionTriple.
bool operator==(const angle::VersionInfo &a, const angle::VersionTriple &b);
bool operator!=(const angle::VersionInfo &a, const angle::VersionTriple &b);
bool operator<(const angle::VersionInfo &a, const angle::VersionTriple &b);
bool operator>=(const angle::VersionInfo &a, const angle::VersionTriple &b);

// Platform helpers
using angle::IsAndroid;
using angle::IsApple;
using angle::IsChromeOS;
using angle::IsFuchsia;
using angle::IsIOS;
using angle::IsLinux;
using angle::IsMac;
using angle::IsWindows;
using angle::IsWindows10OrLater;
using angle::IsWindows8OrLater;
using angle::IsWindowsVistaOrLater;

bool IsWayland();

using OSVersion = angle::VersionTriple;

OSVersion GetMacOSVersion();

OSVersion GetiOSVersion();

OSVersion GetLinuxOSVersion();

int GetAndroidSDKVersion();

}  // namespace rx
#endif  // LIBANGLE_RENDERER_DRIVER_UTILS_H_
