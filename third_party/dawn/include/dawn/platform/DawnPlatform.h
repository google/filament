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

#ifndef INCLUDE_DAWN_PLATFORM_DAWNPLATFORM_H_
#define INCLUDE_DAWN_PLATFORM_DAWNPLATFORM_H_

#include <webgpu/webgpu.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include "dawn/platform/dawn_platform_export.h"

namespace dawn::platform {

enum class TraceCategory {
    General,     // General trace events
    Validation,  // Dawn validation
    Recording,   // Native command recording
    GPUWork,     // Actual GPU work
};

class DAWN_PLATFORM_EXPORT CachingInterface {
  public:
    CachingInterface();
    virtual ~CachingInterface();

    // LoadData has two modes. The first mode is used to get a value which
    // corresponds to the |key|. The |valueOut| is a caller provided buffer
    // allocated to the size |valueSize| which is loaded with data of the
    // size returned. The second mode is used to query for the existence of
    // the |key| where |valueOut| is nullptr and |valueSize| must be 0.
    // The return size is non-zero if the |key| exists.
    virtual size_t LoadData(const void* key, size_t keySize, void* valueOut, size_t valueSize) = 0;

    // StoreData puts a |value| in the cache which corresponds to the |key|.
    virtual void StoreData(const void* key,
                           size_t keySize,
                           const void* value,
                           size_t valueSize) = 0;

  private:
    CachingInterface(const CachingInterface&) = delete;
    CachingInterface& operator=(const CachingInterface&) = delete;
};

class DAWN_PLATFORM_EXPORT WaitableEvent {
  public:
    WaitableEvent() = default;
    virtual ~WaitableEvent() = default;

    WaitableEvent(const WaitableEvent&) = delete;
    WaitableEvent& operator=(const WaitableEvent&) = delete;

    virtual void Wait() = 0;        // Wait for completion
    virtual bool IsComplete() = 0;  // Non-blocking check if the event is complete
};

using PostWorkerTaskCallback = void (*)(void* userdata);

class DAWN_PLATFORM_EXPORT WorkerTaskPool {
  public:
    WorkerTaskPool() = default;
    virtual ~WorkerTaskPool() = default;

    WorkerTaskPool(const WorkerTaskPool&) = delete;
    WorkerTaskPool& operator=(const WorkerTaskPool&) = delete;

    virtual std::unique_ptr<WaitableEvent> PostWorkerTask(PostWorkerTaskCallback,
                                                          void* userdata) = 0;
};

// These features map to similarly named ones in src/chromium/src/gpu/config/gpu_finch_features.h
// in `namespace features`.
enum class Features {
    kWebGPUUseDXC,
    kWebGPUUseTintIR,
    kWebGPUUseVulkanMemoryModel,
};

class DAWN_PLATFORM_EXPORT Platform {
  public:
    Platform();
    virtual ~Platform();

    virtual const unsigned char* GetTraceCategoryEnabledFlag(TraceCategory category);

    virtual double MonotonicallyIncreasingTime();

    virtual uint64_t AddTraceEvent(char phase,
                                   const unsigned char* categoryGroupEnabled,
                                   const char* name,
                                   uint64_t id,
                                   double timestamp,
                                   int numArgs,
                                   const char** argNames,
                                   const unsigned char* argTypes,
                                   const uint64_t* argValues,
                                   unsigned char flags);

    // Invoked to add a UMA histogram count-based sample
    virtual void HistogramCustomCounts(const char* name,
                                       int sample,
                                       int min,
                                       int max,
                                       int bucketCount);

    // Invoked to add a UMA histogram count-based sample that requires high-performance
    // counter (HPC) support.
    virtual void HistogramCustomCountsHPC(const char* name,
                                          int sample,
                                          int min,
                                          int max,
                                          int bucketCount);

    // Invoked to add a UMA histogram enumeration sample
    virtual void HistogramEnumeration(const char* name, int sample, int boundaryValue);

    // Invoked to add a UMA histogram sparse sample
    virtual void HistogramSparse(const char* name, int sample);

    // Invoked to add a UMA histogram boolean sample
    virtual void HistogramBoolean(const char* name, bool sample);

    // The returned CachingInterface is expected to outlive the device which uses it to persistently
    // cache objects.
    virtual CachingInterface* GetCachingInterface();

    virtual std::unique_ptr<WorkerTaskPool> CreateWorkerTaskPool();

    // Hook for querying if a Finch feature is enabled.
    virtual bool IsFeatureEnabled(Features feature);

  private:
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
};

}  // namespace dawn::platform

#endif  // INCLUDE_DAWN_PLATFORM_DAWNPLATFORM_H_
