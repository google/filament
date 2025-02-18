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

#include <vector>

#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumIterations = 50;

enum class UploadMethod {
    WriteBuffer,
    MappedAtCreation,
    MapWithExtendedUsages,
    StagingBuffer,
};

// Perf delta exists between ranges [0, 1MB] vs [1MB, MAX_SIZE).
// These are sample buffer sizes within each range.
enum class UploadSize {
    BufferSize_1KB = 1 * 1024,
    BufferSize_64KB = 64 * 1024,
    BufferSize_1MB = 1 * 1024 * 1024,

    BufferSize_4MB = 4 * 1024 * 1024,
    BufferSize_16MB = 16 * 1024 * 1024,
};

struct BufferUploadParams : AdapterTestParam {
    BufferUploadParams(const AdapterTestParam& param,
                       UploadMethod uploadMethod,
                       UploadSize uploadSize)
        : AdapterTestParam(param), uploadMethod(uploadMethod), uploadSize(uploadSize) {}

    UploadMethod uploadMethod;
    UploadSize uploadSize;
};

std::ostream& operator<<(std::ostream& ostream, const BufferUploadParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);

    switch (param.uploadMethod) {
        case UploadMethod::WriteBuffer:
            ostream << "_WriteBuffer";
            break;
        case UploadMethod::MappedAtCreation:
            ostream << "_MappedAtCreation";
            break;
        case UploadMethod::MapWithExtendedUsages:
            ostream << "_MapWithExtendedUsages";
            break;
        case UploadMethod::StagingBuffer:
            ostream << "_StagingBuffer";
            break;
    }

    switch (param.uploadSize) {
        case UploadSize::BufferSize_1KB:
            ostream << "_BufferSize_1KB";
            break;
        case UploadSize::BufferSize_64KB:
            ostream << "_BufferSize_64KB";
            break;
        case UploadSize::BufferSize_1MB:
            ostream << "_BufferSize_1MB";
            break;
        case UploadSize::BufferSize_4MB:
            ostream << "_BufferSize_4MB";
            break;
        case UploadSize::BufferSize_16MB:
            ostream << "_BufferSize_16MB";
            break;
    }

    return ostream;
}

// Test uploading |kBufferSize| bytes of data |kNumIterations| times.
class BufferUploadPerf : public DawnPerfTestWithParams<BufferUploadParams> {
  public:
    BufferUploadPerf()
        : DawnPerfTestWithParams(kNumIterations, 1),
          data(static_cast<size_t>(GetParam().uploadSize)) {}
    ~BufferUploadPerf() override = default;

    void SetUp() override;

  private:
    void Step() override;

    wgpu::Buffer dst;
    std::vector<uint8_t> data;
};

void BufferUploadPerf::SetUp() {
    DawnPerfTestWithParams<BufferUploadParams>::SetUp();

    wgpu::BufferDescriptor desc = {};
    desc.size = data.size();
    desc.usage = wgpu::BufferUsage::CopyDst;

    dst = device.CreateBuffer(&desc);
}

void BufferUploadPerf::Step() {
    switch (GetParam().uploadMethod) {
        case UploadMethod::WriteBuffer: {
            for (unsigned int i = 0; i < kNumIterations; ++i) {
                queue.WriteBuffer(dst, 0, data.data(), data.size());
            }
            // Make sure all WriteBuffer's are flushed.
            queue.Submit(0, nullptr);
            break;
        }

        case UploadMethod::MappedAtCreation: {
            wgpu::BufferDescriptor desc = {};
            desc.size = data.size();
            desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
            desc.mappedAtCreation = true;

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            for (unsigned int i = 0; i < kNumIterations; ++i) {
                wgpu::Buffer buffer = device.CreateBuffer(&desc);
                memcpy(buffer.GetMappedRange(0, data.size()), data.data(), data.size());
                buffer.Unmap();
                encoder.CopyBufferToBuffer(buffer, 0, dst, 0, data.size());
            }

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
            break;
        }

        default:
            DAWN_UNREACHABLE();
    }
}

TEST_P(BufferUploadPerf, Run) {
    RunTest();
}

class BufferMapExtendedUsagesPerf : public DawnPerfTestWithParams<BufferUploadParams> {
  public:
    BufferMapExtendedUsagesPerf()
        : DawnPerfTestWithParams(kNumIterations, 1),
          data(static_cast<size_t>(GetParam().uploadSize)) {}
    ~BufferMapExtendedUsagesPerf() override = default;

    void SetUp() override;

  private:
    void Step() override;

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;

    void MapAsyncAndWait(const wgpu::Buffer& buffer,
                         wgpu::MapMode mode,
                         size_t offset,
                         size_t size);

    wgpu::Buffer buffers[kNumIterations];
    wgpu::Buffer stagingBuffers[kNumIterations];
    std::vector<uint8_t> data;
};

std::vector<wgpu::FeatureName> BufferMapExtendedUsagesPerf::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = DawnPerfTestWithParams::GetRequiredFeatures();
    if (!UsesWire() && GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages &&
        SupportsFeatures({wgpu::FeatureName::BufferMapExtendedUsages})) {
        requiredFeatures.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
    }
    return requiredFeatures;
}

void BufferMapExtendedUsagesPerf::SetUp() {
    DawnPerfTestWithParams<BufferUploadParams>::SetUp();

    // Skip all tests if the BufferMapExtendedUsages feature is not supported.
    DAWN_TEST_UNSUPPORTED_IF(GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages &&
                             !device.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages));

    for (auto& buffer : buffers) {
        wgpu::BufferDescriptor desc = {};
        desc.size = data.size();

        if (GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages) {
            desc.usage = wgpu::BufferUsage::MapWrite;
        } else {
            desc.usage = wgpu::BufferUsage::CopyDst;
        }

        desc.usage |= wgpu::BufferUsage::Storage;

        buffer = device.CreateBuffer(&desc);
    }

    if (GetParam().uploadMethod == UploadMethod::StagingBuffer) {
        for (auto& buffer : stagingBuffers) {
            wgpu::BufferDescriptor desc = {};
            desc.size = data.size();

            desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

            buffer = device.CreateBuffer(&desc);
        }
    }
}

void BufferMapExtendedUsagesPerf::Step() {
    switch (GetParam().uploadMethod) {
        case UploadMethod::WriteBuffer: {
            for (unsigned int i = 0; i < kNumIterations; ++i) {
                queue.WriteBuffer(buffers[i], 0, data.data(), data.size());
            }
            // Make sure all WriteBuffer's are flushed.
            queue.Submit(0, nullptr);
            break;
        }

        case UploadMethod::MappedAtCreation: {
            wgpu::BufferDescriptor desc = {};
            desc.size = data.size();
            desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
            desc.mappedAtCreation = true;

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            for (unsigned int i = 0; i < kNumIterations; ++i) {
                stagingBuffers[i] = device.CreateBuffer(&desc);
                memcpy(stagingBuffers[i].GetMappedRange(0, data.size()), data.data(), data.size());
                stagingBuffers[i].Unmap();
                encoder.CopyBufferToBuffer(stagingBuffers[i], 0, buffers[i], 0, data.size());
            }

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
            break;
        }

        case UploadMethod::StagingBuffer: {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            for (unsigned int i = 0; i < kNumIterations; ++i) {
                MapAsyncAndWait(stagingBuffers[i], wgpu::MapMode::Write, 0, data.size());
                memcpy(stagingBuffers[i].GetMappedRange(0, data.size()), data.data(), data.size());
                stagingBuffers[i].Unmap();
                encoder.CopyBufferToBuffer(stagingBuffers[i], 0, buffers[i], 0, data.size());
            }

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
            break;
        }

        case UploadMethod::MapWithExtendedUsages: {
            for (unsigned int i = 0; i < kNumIterations; ++i) {
                MapAsyncAndWait(buffers[i], wgpu::MapMode::Write, 0, data.size());
                memcpy(buffers[i].GetMappedRange(0, data.size()), data.data(), data.size());
                buffers[i].Unmap();
            }
            break;
        }
    }
}

void BufferMapExtendedUsagesPerf::MapAsyncAndWait(const wgpu::Buffer& buffer,
                                                  wgpu::MapMode mode,
                                                  size_t offset,
                                                  size_t size) {
    wgpu::Future future = buffer.MapAsync(mode, offset, size, wgpu::CallbackMode::WaitAnyOnly,
                                          [](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                              ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                                          });
    wgpu::FutureWaitInfo waitInfo = {future};
    GetInstance().WaitAny(1, &waitInfo, UINT64_MAX);
    ASSERT_TRUE(waitInfo.completed);
}

TEST_P(BufferMapExtendedUsagesPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(BufferUploadPerf,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend()},
                        {UploadMethod::WriteBuffer, UploadMethod::MappedAtCreation},
                        {UploadSize::BufferSize_1KB, UploadSize::BufferSize_64KB,
                         UploadSize::BufferSize_1MB, UploadSize::BufferSize_4MB,
                         UploadSize::BufferSize_16MB});

DAWN_INSTANTIATE_TEST_P(BufferMapExtendedUsagesPerf,
                        {D3D12Backend(), D3D11Backend(), MetalBackend(), OpenGLBackend(),
                         VulkanBackend()},
                        {UploadMethod::WriteBuffer, UploadMethod::MappedAtCreation,
                         UploadMethod::MapWithExtendedUsages, UploadMethod::StagingBuffer},
                        {UploadSize::BufferSize_1KB, UploadSize::BufferSize_64KB,
                         UploadSize::BufferSize_1MB, UploadSize::BufferSize_4MB,
                         UploadSize::BufferSize_16MB});

}  // anonymous namespace
}  // namespace dawn
