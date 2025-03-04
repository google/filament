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

#ifndef DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP
#define DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <condition_variable>
#include <mutex>

namespace wgpu {
struct DeviceDescriptor;
}  // namespace wgpu

namespace dawn {

class NullDeviceBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp(const benchmark::State& state) override;
    void TearDown(const benchmark::State& state) override;

  protected:
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;

  private:
    virtual wgpu::DeviceDescriptor GetDeviceDescriptor() const = 0;

    // Lock and conditional variable used to synchronize the benchmark global adapter/device.
    std::mutex mMutex;
    std::condition_variable mCv;
    int mNumDoneThreads = 0;
};

}  // namespace dawn

#endif  // DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP
