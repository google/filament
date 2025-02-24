// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_QUEUE_H_
#define SRC_DAWN_NATIVE_QUEUE_H_

#include <memory>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialMap.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/Error.h"
#include "dawn/native/ExecutionQueue.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SystemEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/DawnNative.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

// For the commands with async callback like 'MapAsync' and 'OnSubmittedWorkDone', we track the
// execution serials of completion in the queue for them. This implements 'CallbackTask' so that the
// aysnc callback can be fired by 'CallbackTaskManager' in a unified way. This also caches the
// finished serial, as the callback needs to use it in the trace event.
struct TrackTaskCallback : CallbackTask {
    explicit TrackTaskCallback(dawn::platform::Platform* platform) : mPlatform(platform) {}
    void SetFinishedSerial(ExecutionSerial serial);
    ~TrackTaskCallback() override = default;

  protected:
    raw_ptr<dawn::platform::Platform> mPlatform = nullptr;
    // The serial by which time the callback can be fired.
    ExecutionSerial mSerial = kMaxExecutionSerial;
};

class QueueBase : public ApiObjectBase, public ExecutionQueueBase {
  public:
    ~QueueBase() override;

    static Ref<QueueBase> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    // Dawn API
    void APISubmit(uint32_t commandCount, CommandBufferBase* const* commands);
    Future APIOnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo);
    void APIWriteBuffer(BufferBase* buffer, uint64_t bufferOffset, const void* data, size_t size);
    void APIWriteTexture(const TexelCopyTextureInfo* destination,
                         const void* data,
                         size_t dataSize,
                         const TexelCopyBufferLayout* dataLayout,
                         const Extent3D* writeSize);
    void APICopyTextureForBrowser(const TexelCopyTextureInfo* source,
                                  const TexelCopyTextureInfo* destination,
                                  const Extent3D* copySize,
                                  const CopyTextureForBrowserOptions* options);
    void APICopyExternalTextureForBrowser(const ImageCopyExternalTexture* source,
                                          const TexelCopyTextureInfo* destination,
                                          const Extent3D* copySize,
                                          const CopyTextureForBrowserOptions* options);

    MaybeError WriteBuffer(BufferBase* buffer,
                           uint64_t bufferOffset,
                           const void* data,
                           size_t size);
    // Ensure a flush occurs if needed, and track this task as complete after the
    // scheduled work is complete.
    void TrackTaskAfterEventualFlush(std::unique_ptr<TrackTaskCallback> task);
    // Track a task as complete after the serial has passed. If the serial is in the future,
    // a flush is forced.
    void TrackTask(std::unique_ptr<TrackTaskCallback> task, ExecutionSerial serial);
    // Track a task as eventually complete after the pending command serial passes.
    // This is only needed because other parts of Dawn use pending command serial extensively.
    // TODO(crbug.com/dawn/1413): It should be removed after ExecutionQueue for better tracking
    // of completion.
    void TrackPendingTask(std::unique_ptr<TrackTaskCallback> task);

    void Tick(ExecutionSerial finishedSerial);
    void HandleDeviceLoss();

  protected:
    QueueBase(DeviceBase* device, const QueueDescriptor* descriptor);
    QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    void DestroyImpl() override;

    virtual MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) = 0;
    virtual MaybeError WriteBufferImpl(BufferBase* buffer,
                                       uint64_t bufferOffset,
                                       const void* data,
                                       size_t size);
    virtual MaybeError WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                        const void* data,
                                        size_t dataSize,
                                        const TexelCopyBufferLayout& dataLayout,
                                        const Extent3D& writeSize);

  private:
    MaybeError WriteTextureInternal(const TexelCopyTextureInfo* destination,
                                    const void* data,
                                    size_t dataSize,
                                    const TexelCopyBufferLayout& dataLayout,
                                    const Extent3D* writeSize);
    MaybeError CopyTextureForBrowserInternal(const TexelCopyTextureInfo* source,
                                             const TexelCopyTextureInfo* destination,
                                             const Extent3D* copySize,
                                             const CopyTextureForBrowserOptions* options);
    MaybeError CopyExternalTextureForBrowserInternal(const ImageCopyExternalTexture* source,
                                                     const TexelCopyTextureInfo* destination,
                                                     const Extent3D* copySize,
                                                     const CopyTextureForBrowserOptions* options);

    MaybeError ValidateSubmit(uint32_t commandCount, CommandBufferBase* const* commands) const;
    MaybeError ValidateOnSubmittedWorkDone() const;
    MaybeError ValidateWriteTexture(const TexelCopyTextureInfo* destination,
                                    size_t dataSize,
                                    const TexelCopyBufferLayout& dataLayout,
                                    const Extent3D* writeSize) const;

    MaybeError SubmitInternal(uint32_t commandCount, CommandBufferBase* const* commands);

    MutexProtected<SerialMap<ExecutionSerial, std::unique_ptr<TrackTaskCallback>>> mTasksInFlight;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUEUE_H_
