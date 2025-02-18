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

#ifndef SRC_DAWN_COMMON_GPUINFO_H_
#define SRC_DAWN_COMMON_GPUINFO_H_

#include <string>

#include "absl/container/inlined_vector.h"
#include "dawn/common/GPUInfo_autogen.h"

namespace dawn::gpu_info {

// Four uint16 fields could cover almost all driver version schemas:
// D3D12: AA.BB.CCC.DDDD
// Vulkan: AAA.BBB.CCC.DDD on Nvidia, CCC.DDDD for Intel Windows, and AA.BB.CCC for others,
// See https://vulkan.gpuinfo.org/
static constexpr uint32_t kMaxVersionFields = 4;

class DriverVersion {
  public:
    DriverVersion();
    DriverVersion(const std::initializer_list<uint16_t>& version);

    uint16_t& operator[](size_t i);
    const uint16_t& operator[](size_t i) const;

    uint32_t size() const;
    std::string ToString() const;

  private:
    absl::InlinedVector<uint16_t, kMaxVersionFields> mDriverVersion;
};

// Do comparison between two driver versions. Currently we only support the comparison between
// Intel Windows driver versions.
// - Return -1 if build number of version1 is smaller
// - Return 1 if build number of version1 is bigger
// - Return 0 if version1 and version2 represent same driver version
int CompareWindowsDriverVersion(PCIVendorID vendorId,
                                const DriverVersion& version1,
                                const DriverVersion& version2);

// Do comparison between two Intel Mesa driver versions.
// - Return a negative number if build number of version1 is smaller
// - Return a positive number if build number of version1 is bigger
// - Return 0 if version1 and version2 represent same driver version
int CompareIntelMesaDriverVersion(const DriverVersion& version1, const DriverVersion& version2);

// Intel architectures
bool IsSkylake(PCIDeviceID deviceId);
bool IsIrisPlus655(PCIDeviceID deviceId);

bool IsIntelGen11OrOlder(PCIVendorID venderId, PCIDeviceID deviceId);

}  // namespace dawn::gpu_info
#endif  // SRC_DAWN_COMMON_GPUINFO_H_
