//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_libpci.cpp: implementation of the libPCI-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

#include <dlfcn.h>
#include <pci/pci.h>
#include <unistd.h>

#include "common/angleutils.h"
#include "common/debug.h"

#if !defined(GPU_INFO_USE_LIBPCI)
#    error SystemInfo_libpci.cpp compiled without GPU_INFO_USE_LIBPCI
#endif

namespace angle
{

namespace
{

struct LibPCI : private angle::NonCopyable
{
    LibPCI()
    {
        if (access("/sys/bus/pci/", F_OK) != 0 && access("/sys/bus/pci_express/", F_OK) != 0)
        {
            return;
        }

        mHandle = dlopen("libpci.so.3", RTLD_LAZY);

        if (mHandle == nullptr)
        {
            mHandle = dlopen("libpci.so", RTLD_LAZY);
        }

        if (mHandle == nullptr)
        {
            return;
        }

        mValid =
            (Alloc = reinterpret_cast<decltype(Alloc)>(dlsym(mHandle, "pci_alloc"))) != nullptr &&
            (Init = reinterpret_cast<decltype(Init)>(dlsym(mHandle, "pci_init"))) != nullptr &&
            (Cleanup = reinterpret_cast<decltype(Cleanup)>(dlsym(mHandle, "pci_cleanup"))) !=
                nullptr &&
            (ScanBus = reinterpret_cast<decltype(ScanBus)>(dlsym(mHandle, "pci_scan_bus"))) !=
                nullptr &&
            (FillInfo = reinterpret_cast<decltype(FillInfo)>(dlsym(mHandle, "pci_fill_info"))) !=
                nullptr &&
            (LookupName = reinterpret_cast<decltype(LookupName)>(
                 dlsym(mHandle, "pci_lookup_name"))) != nullptr &&
            (PCIReadByte = reinterpret_cast<decltype(PCIReadByte)>(
                 dlsym(mHandle, "pci_read_byte"))) != nullptr;
    }

    bool IsValid() const { return mValid; }

    ~LibPCI()
    {
        if (mHandle != nullptr)
        {
            dlclose(mHandle);
        }
    }

    decltype(&::pci_alloc) Alloc            = nullptr;
    decltype(&::pci_init) Init              = nullptr;
    decltype(&::pci_cleanup) Cleanup        = nullptr;
    decltype(&::pci_scan_bus) ScanBus       = nullptr;
    decltype(&::pci_fill_info) FillInfo     = nullptr;
    decltype(&::pci_lookup_name) LookupName = nullptr;
    decltype(&::pci_read_byte) PCIReadByte  = nullptr;

  private:
    void *mHandle = nullptr;
    bool mValid   = false;
};

}  // anonymous namespace

// Adds an entry per PCI GPU found and fills the device and vendor ID.
bool GetPCIDevicesWithLibPCI(std::vector<GPUDeviceInfo> *devices)
{
    LibPCI pci;
    if (!pci.IsValid())
    {
        return false;
    }

    pci_access *access = pci.Alloc();
    ASSERT(access != nullptr);
    pci.Init(access);
    pci.ScanBus(access);

    for (pci_dev *device = access->devices; device != nullptr; device = device->next)
    {
        pci.FillInfo(device, PCI_FILL_IDENT | PCI_FILL_CLASS);

        // Skip non-GPU devices
        if (device->device_class >> 8 != PCI_BASE_CLASS_DISPLAY)
        {
            continue;
        }

        // Skip unknown devices
        if (device->vendor_id == 0 || device->device_id == 0)
        {
            continue;
        }

        GPUDeviceInfo info;
        info.vendorId   = device->vendor_id;
        info.deviceId   = device->device_id;
        info.revisionId = pci.PCIReadByte(device, PCI_REVISION_ID);

        devices->push_back(info);
    }

    pci.Cleanup(access);

    return true;
}
}  // namespace angle
