//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo.h: gathers information available without starting a GPU driver.

#ifndef GPU_INFO_UTIL_SYSTEM_INFO_H_
#define GPU_INFO_UTIL_SYSTEM_INFO_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace angle
{

using VendorID       = uint32_t;
using DeviceID       = uint32_t;
using RevisionID     = uint32_t;
using SystemDeviceID = uint64_t;
using DriverID       = uint32_t;

struct VersionInfo
{
    uint32_t major    = 0;
    uint32_t minor    = 0;
    uint32_t subMinor = 0;
    uint32_t patch    = 0;
};

struct GPUDeviceInfo
{
    GPUDeviceInfo();
    ~GPUDeviceInfo();

    GPUDeviceInfo(const GPUDeviceInfo &other);

    VendorID vendorId             = 0;
    DeviceID deviceId             = 0;
    RevisionID revisionId         = 0;
    SystemDeviceID systemDeviceId = 0;

    std::string driverVendor;
    std::string driverVersion;
    std::string driverDate;

    // Fields only available via GetSystemInfoVulkan:
    VersionInfo detailedDriverVersion;
    uint8_t deviceUUID[16]    = {};
    uint8_t driverUUID[16]    = {};
    DriverID driverId         = 0;
    uint32_t driverApiVersion = 0;
};

struct SystemInfo
{
    SystemInfo();
    ~SystemInfo();

    SystemInfo(const SystemInfo &other);

    bool hasNVIDIAGPU() const;
    bool hasIntelGPU() const;
    bool hasAMDGPU() const;

    // Returns the index to `gpus` if the entry matches the preferred device string.
    std::optional<size_t> getPreferredGPUIndex() const;

    std::vector<GPUDeviceInfo> gpus;

    // Index of the GPU expected to be used for 3D graphics. Based on a best-guess heuristic on
    // some platforms. On Windows, this is accurate. Note `gpus` must be checked for empty before
    // indexing.
    int activeGPUIndex = 0;

    bool isOptimus       = false;
    bool isAMDSwitchable = false;
    // Only true on dual-GPU Mac laptops.
    bool isMacSwitchable = false;

    // Only available on Android
    std::string machineManufacturer;
    int androidSdkLevel = 0;

    // Only available on macOS and Android
    std::string machineModelName;

    // Only available on macOS
    std::string machineModelVersion;
};

// Gathers information about the system without starting a GPU driver and returns them in `info`.
// Returns true if all info was gathered, false otherwise. Even when false is returned, `info` will
// be filled with partial information.
bool GetSystemInfo(SystemInfo *info);

// Vulkan-specific info collection.
bool GetSystemInfoVulkan(SystemInfo *info);

// Known PCI vendor IDs
constexpr VendorID kVendorID_AMD           = 0x1002;
constexpr VendorID kVendorID_ARM           = 0x13B5;
constexpr VendorID kVendorID_Broadcom      = 0x14E4;
constexpr VendorID kVendorID_GOOGLE        = 0x1AE0;
constexpr VendorID kVendorID_ImgTec        = 0x1010;
constexpr VendorID kVendorID_Intel         = 0x8086;
constexpr VendorID kVendorID_NVIDIA        = 0x10DE;
constexpr VendorID kVendorID_Qualcomm      = 0x5143;
constexpr VendorID kVendorID_Qualcomm_DXGI = 0x4D4F4351;
constexpr VendorID kVendorID_Samsung       = 0x144D;
constexpr VendorID kVendorID_VMWare        = 0x15ad;
constexpr VendorID kVendorID_Apple         = 0x106B;
constexpr VendorID kVendorID_Microsoft     = 0x1414;
constexpr VendorID kVendorID_VirtIO        = 0x1AF4;

// Known non-PCI (i.e. Khronos-registered) vendor IDs
constexpr VendorID kVendorID_Vivante     = 0x10001;
constexpr VendorID kVendorID_VeriSilicon = 0x10002;
constexpr VendorID kVendorID_Kazan       = 0x10003;
constexpr VendorID kVendorID_CodePlay    = 0x10004;
constexpr VendorID kVendorID_Mesa        = 0x10005;
constexpr VendorID kVendorID_PoCL        = 0x10006;

// Known device IDs
constexpr DeviceID kDeviceID_Swiftshader  = 0xC0DE;
constexpr DeviceID kDeviceID_Adreno540    = 0x5040001;
constexpr DeviceID kDeviceID_Adreno750    = 0x43051401;
constexpr DeviceID kDeviceID_UHD630Mobile = 0x3E9B;

// Predicates on vendor IDs
bool IsAMD(VendorID vendorId);
bool IsARM(VendorID vendorId);
bool IsBroadcom(VendorID vendorId);
bool IsImgTec(VendorID vendorId);
bool IsIntel(VendorID vendorId);
bool IsKazan(VendorID vendorId);
bool IsNVIDIA(VendorID vendorId);
bool IsQualcomm(VendorID vendorId);
bool IsSamsung(VendorID vendorId);
bool IsGoogle(VendorID vendorId);
bool IsSwiftshader(VendorID vendorId);
bool IsVeriSilicon(VendorID vendorId);
bool IsVMWare(VendorID vendorId);
bool IsVirtIO(VendorID vendorId);
bool IsVivante(VendorID vendorId);
bool IsAppleGPU(VendorID vendorId);
bool IsMicrosoft(VendorID vendorId);

// Returns a readable vendor name given the VendorID
std::string VendorName(VendorID vendor);

// Use a heuristic to attempt to find the GPU used for 3D graphics. Sets activeGPUIndex,
// isOptimus, and isAMDSwitchable.
// Always assumes the non-Intel GPU is active on dual-GPU machines.
void GetDualGPUInfo(SystemInfo *info);

// Dumps the system info to stdout.
void PrintSystemInfo(const SystemInfo &info);

uint64_t GetSystemDeviceIdFromParts(uint32_t highPart, uint32_t lowPart);
uint32_t GetSystemDeviceIdHighPart(uint64_t systemDeviceId);
uint32_t GetSystemDeviceIdLowPart(uint64_t systemDeviceId);

// Returns lower-case of ANGLE_PREFERRED_DEVICE environment variable contents.
std::string GetPreferredDeviceString();

}  // namespace angle

#endif  // GPU_INFO_UTIL_SYSTEM_INFO_H_
