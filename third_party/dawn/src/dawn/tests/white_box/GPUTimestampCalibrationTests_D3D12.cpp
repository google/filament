// Copyright 2023 The Dawn & Tint Authors
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

#include <memory>

#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/tests/white_box/GPUTimestampCalibrationTests.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

class GPUTimestampCalibrationTestsD3D12 : public GPUTimestampCalibrationTestBackend {
  public:
    explicit GPUTimestampCalibrationTestsD3D12(const wgpu::Device& device) {
        mBackendDevice = native::d3d12::ToBackend(native::FromAPI(device.Get()));
        mBackendQueue = native::d3d12::ToBackend(mBackendDevice->GetQueue());
    }

    bool IsSupported() const override { return true; }

    void GetTimestampCalibration(uint64_t* gpuTimestamp, uint64_t* cpuTimestamp) override {
        mBackendQueue->GetCommandQueue()->GetClockCalibration(gpuTimestamp, cpuTimestamp);
    }

    float GetTimestampPeriod() const override { return mBackendDevice->GetTimestampPeriodInNS(); }

  private:
    raw_ptr<native::d3d12::Device> mBackendDevice;
    raw_ptr<native::d3d12::Queue> mBackendQueue;
};

}  // anonymous namespace

// static
std::unique_ptr<GPUTimestampCalibrationTestBackend> GPUTimestampCalibrationTestBackend::Create(
    const wgpu::Device& device) {
    return std::make_unique<GPUTimestampCalibrationTestsD3D12>(device);
}

}  // namespace dawn
