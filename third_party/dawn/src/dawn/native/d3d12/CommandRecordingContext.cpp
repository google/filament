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

#include "dawn/native/d3d12/CommandRecordingContext.h"

#include <profileapi.h>
#include <sysinfoapi.h>

#include <string>
#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {

void CommandRecordingContext::AddToSharedTextureList(Texture* texture) {
    mSharedTextures.insert(texture);
}

void CommandRecordingContext::Open(ComPtr<ID3D12GraphicsCommandList> commandList) {
    mD3d12CommandList = std::move(commandList);
    mD3d12CommandList.As(&mD3d12CommandList4);
    mNeedsSubmit = false;
}

MaybeError CommandRecordingContext::ExecuteCommandList(Device* device,
                                                       ID3D12CommandQueue* commandQueue) {
    DAWN_ASSERT(mD3d12CommandList != nullptr);

    for (Texture* texture : mSharedTextures) {
        DAWN_TRY(texture->SynchronizeTextureBeforeUse(this));
    }

    MaybeError error =
        CheckHRESULT(mD3d12CommandList->Close(), "D3D12 closing pending command list");
    if (error.IsError()) {
        Release();
        DAWN_TRY(std::move(error));
    }
    DAWN_TRY(device->GetResidencyManager()->EnsureHeapsAreResident(mHeapsPendingUsage.data(),
                                                                   mHeapsPendingUsage.size()));

    if (device->IsToggleEnabled(Toggle::RecordDetailedTimingInTraceEvents)) {
        uint64_t gpuTimestamp;
        uint64_t cpuTimestamp;
        FILETIME fileTimeNonPrecise;
        SYSTEMTIME systemTimeNonPrecise;

        // Both supported since Windows 2000, have a accuracy of 1ms
        GetSystemTimeAsFileTime(&fileTimeNonPrecise);
        GetSystemTime(&systemTimeNonPrecise);
        // Query CPU and GPU timestamps at almost the same time
        commandQueue->GetClockCalibration(&gpuTimestamp, &cpuTimestamp);

        uint64_t gpuFrequency;
        uint64_t cpuFrequency;
        LARGE_INTEGER cpuFrequencyLargeInteger;
        commandQueue->GetTimestampFrequency(&gpuFrequency);
        QueryPerformanceFrequency(&cpuFrequencyLargeInteger);  // Supported since Windows 2000
        cpuFrequency = cpuFrequencyLargeInteger.QuadPart;

        std::string timingInfo = absl::StrFormat(
            "UTC Time: %u/%u/%u %02u:%02u:%02u.%03u, File Time: %u, CPU "
            "Timestamp: %u, GPU Timestamp: %u, CPU Tick Frequency: %u, GPU Tick Frequency: "
            "%u",
            systemTimeNonPrecise.wYear, systemTimeNonPrecise.wMonth, systemTimeNonPrecise.wDay,
            systemTimeNonPrecise.wHour, systemTimeNonPrecise.wMinute, systemTimeNonPrecise.wSecond,
            systemTimeNonPrecise.wMilliseconds,
            (static_cast<uint64_t>(fileTimeNonPrecise.dwHighDateTime) << 32) +
                fileTimeNonPrecise.dwLowDateTime,
            cpuTimestamp, gpuTimestamp, cpuFrequency, gpuFrequency);

        TRACE_EVENT_INSTANT1(device->GetPlatform(), General,
                             "d3d12::CommandRecordingContext::ExecuteCommandList Detailed Timing",
                             "Timing", timingInfo.c_str());
    }

    ID3D12CommandList* d3d12CommandList = GetCommandList();
    commandQueue->ExecuteCommandLists(1, &d3d12CommandList);

    Release();
    return {};
}

void CommandRecordingContext::TrackHeapUsage(Heap* heap, ExecutionSerial serial) {
    // Before tracking the heap, check the last serial it was recorded on to ensure we aren't
    // tracking it more than once.
    if (heap->GetLastUsage() < serial) {
        heap->SetLastUsage(serial);
        mHeapsPendingUsage.push_back(heap);
    }
}

ID3D12GraphicsCommandList* CommandRecordingContext::GetCommandList() const {
    DAWN_ASSERT(mD3d12CommandList != nullptr);
    return mD3d12CommandList.Get();
}

// This function will fail on Windows versions prior to 1809. Support must be queried through
// the device before calling.
ID3D12GraphicsCommandList4* CommandRecordingContext::GetCommandList4() const {
    DAWN_ASSERT(mD3d12CommandList != nullptr);
    return mD3d12CommandList4.Get();
}

void CommandRecordingContext::Release() {
    mD3d12CommandList.Reset();
    mD3d12CommandList4.Reset();

    mNeedsSubmit = false;

    mSharedTextures.clear();
    mHeapsPendingUsage.clear();
    mTempBuffers.clear();

    ReleaseKeyedMutexes();
}

bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}

void CommandRecordingContext::AddToTempBuffers(Ref<Buffer> tempBuffer) {
    mTempBuffers.emplace_back(tempBuffer);
}

MaybeError CommandRecordingContext::AcquireKeyedMutex(Ref<d3d::KeyedMutex> keyedMutex) {
    if (!mAcquiredKeyedMutexes.contains(keyedMutex)) {
        DAWN_TRY(keyedMutex->AcquireKeyedMutex());
        mAcquiredKeyedMutexes.emplace(std::move(keyedMutex));
    }
    return {};
}

void CommandRecordingContext::ReleaseKeyedMutexes() {
    for (auto& keyedMutex : mAcquiredKeyedMutexes) {
        keyedMutex->ReleaseKeyedMutex();
    }
    mAcquiredKeyedMutexes.clear();
}

}  // namespace dawn::native::d3d12
