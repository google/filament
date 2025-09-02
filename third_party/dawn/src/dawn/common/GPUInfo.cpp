// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/common/GPUInfo.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <sstream>

#include "dawn/common/Assert.h"

namespace dawn::gpu_info {
namespace {
// Intel
// Referenced from the following Mesa source code:
// https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/include/pci_ids/iris_pci_ids.h
// gen9
const std::array<PCIDeviceID, 2> IrisPlus655 = {{0x3EA5, 0x3EA8}};

}  // anonymous namespace

DriverVersion::DriverVersion() = default;

DriverVersion::DriverVersion(const std::initializer_list<uint16_t>& version) {
    DAWN_ASSERT(version.size() <= kMaxVersionFields);
    mDriverVersion.assign(version.begin(), version.end());
}

uint16_t& DriverVersion::operator[](size_t i) {
    return mDriverVersion.operator[](i);
}

const uint16_t& DriverVersion::operator[](size_t i) const {
    return mDriverVersion.operator[](i);
}

uint32_t DriverVersion::size() const {
    return mDriverVersion.size();
}

std::string DriverVersion::ToString() const {
    std::ostringstream oss;
    if (!mDriverVersion.empty()) {
        // Convert all but the last element to avoid a trailing "."
        std::copy(mDriverVersion.begin(), mDriverVersion.end() - 1,
                  std::ostream_iterator<uint16_t>(oss, "."));
        // Add the last element
        oss << mDriverVersion.back();
    }

    return oss.str();
}

// According to Intel graphics driver version schema, build number is generated from the
// last two fields.
// See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics.html for
// more details.
IntelWindowsDriverVersion::IntelWindowsDriverVersion(const DriverVersion& driverVersion) {
    size_t size = driverVersion.size();
    DAWN_ASSERT(size >= 2);
    mBuildNumber = driverVersion[size - 2] * 10000 + driverVersion[size - 1];
}

IntelWindowsDriverVersion::IntelWindowsDriverVersion(const std::initializer_list<uint16_t>& version)
    : IntelWindowsDriverVersion(DriverVersion(version)) {}

int CompareIntelMesaDriverVersion(const DriverVersion& version1, const DriverVersion& version2) {
    for (uint32_t i = 0; i < 3; ++i) {
        int diff = static_cast<int32_t>(version1[i]) - static_cast<int32_t>(version2[i]);
        if (diff != 0) {
            return diff;
        }
    }
    return 0;
}

// Intel GPUs
bool IsSkylake(PCIDeviceID deviceId) {
    return (deviceId & 0xFF00) == 0x1900;
}

bool IsIrisPlus655(PCIDeviceID deviceId) {
    return std::find(IrisPlus655.cbegin(), IrisPlus655.cend(), deviceId) != IrisPlus655.cend();
}

bool IsIntelGen11OrOlder(PCIVendorID venderId, PCIDeviceID deviceId) {
    return IsIntelGen7(venderId, deviceId) || IsIntelGen8(venderId, deviceId) ||
           IsIntelGen9(venderId, deviceId) || IsIntelGen11(venderId, deviceId);
}

}  // namespace dawn::gpu_info
