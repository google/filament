//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_ios.cpp: implementation of the iOS-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

namespace angle
{

bool GetSystemInfo_ios(SystemInfo *info)
{
    {
        // TODO(anglebug.com/42262902): Get the actual system version and GPU info.
        info->machineModelVersion = "0.0";
        GPUDeviceInfo deviceInfo;
        deviceInfo.vendorId = kVendorID_Apple;
        info->gpus.push_back(deviceInfo);
    }

    return true;
}

}  // namespace angle
