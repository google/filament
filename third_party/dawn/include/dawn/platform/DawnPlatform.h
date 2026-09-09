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
#include <span>

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

    // Returns zero if there does not exist a cached entry for |key|, otherwise returns a non-zero
    // size indicating the size of cached data
    virtual size_t FindKey(std::span<const std::byte> key) = 0;

    // Returns zero if unable to load cached entry for |key| into |dest|, otherwise returns number
    // of bytes written to |dest|.
    virtual size_t LoadData(std::span<const std::byte> key, std::span<std::byte> dest) = 0;

    // Stores the data in |src| at the entry specified by |key|.
    virtual void StoreData(std::span<const std::byte> key, std::span<const std::byte> src) = 0;

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

enum class JobStatus {
    Continue,   // For long-running tasks, returning |Continue| will schedule the task again.
    Cancelled,  // For long-running tasks that need to be terminated for shutdown, this may be
                // injected when Cancel() is called.
    Completed,  // For long-running tasks that actually finish.
};

class DAWN_PLATFORM_EXPORT JobHandle {
  public:
    // A job must be joined and canceled before the JobHandle is destroyed.
    JobHandle() = default;
    virtual ~JobHandle() = default;

    JobHandle(JobHandle&& other) = default;
    JobHandle& operator=(JobHandle&&) = default;

    // Cancels the job (and all potential workers) ASAP. As an implementation
    // note, one can imagine this forcing the callback to return |Cancelled|
    // on its next iteration, thereby causing the job to be considered done
    // and cancelled.
    virtual void Cancel() = 0;

    // Joins the job (and all potential workers), waiting for them to return.
    virtual void Join() = 0;

  private:
    JobHandle(const JobHandle&) = delete;
    JobHandle& operator=(const JobHandle&) = delete;
};

using PostWorkerTaskCallback = void (*)(void* userdata);
using PostWorkerJobCallback = JobStatus (*)(void* userdata);

class DAWN_PLATFORM_EXPORT WorkerTaskPool {
  public:
    WorkerTaskPool() = default;
    virtual ~WorkerTaskPool() = default;

    WorkerTaskPool(const WorkerTaskPool&) = delete;
    WorkerTaskPool& operator=(const WorkerTaskPool&) = delete;

    // Creates Dawn's default worker task pool with at most |maxThreadCount| task handling threads.
    // Platform implementations may use this when overriding Platform::CreateWorkerTaskPool().
    static std::unique_ptr<WorkerTaskPool> CreateDawnDefault(uint32_t maxThreadCount);

    virtual std::unique_ptr<WaitableEvent> PostWorkerTask(PostWorkerTaskCallback,
                                                          void* userdata) = 0;

    // This will start up to a worker which calls |cb| with |userdata| when scheduling permits while
    // |cb| returns |Continue|. In general, |cb| should periodically yield regardless of whether it
    // completed its work in order to allow for cancellation or reprioritization when appropriate.
    virtual std::unique_ptr<JobHandle> PostWorkerJob(PostWorkerJobCallback cb, void* userdata);
};

// These features map to similarly named ones in src/chromium/src/gpu/config/gpu_finch_features.h
// in `namespace features`.
enum class Features {
    kWebGPUUseDXC,
    kWebGPUEnableRangeAnalysisForRobustness,
    kWebGPUUseSpirv14,
    kWebGPUDecomposeUniformBuffers,
    kWebGPUUseHLSL2021,
    kWebGPUUseSpirvReconvergenceMode,
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

    // Report GPU process progress so that the watchdog thread won't think that a long function is
    // stuck.
    virtual void ReportProgress();

  private:
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
};

}  // namespace dawn::platform

#endif  // INCLUDE_DAWN_PLATFORM_DAWNPLATFORM_H_
