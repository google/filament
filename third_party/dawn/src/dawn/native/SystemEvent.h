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

#ifndef SRC_DAWN_NATIVE_SYSTEMEVENT_H_
#define SRC_DAWN_NATIVE_SYSTEMEVENT_H_

#include <optional>
#include <utility>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Platform.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemHandle.h"

namespace dawn::native {

class SystemEventPipeSender;

// SystemEventReceiver holds an OS event primitive (Win32 Event Object or POSIX file descriptor (fd)
// that will be signalled by some other thing: either an OS integration like SetEventOnCompletion(),
// or our own code like SystemEventPipeSender.
//
// SystemEventReceiver is one-time-use (to make it easier to use correctly) - once it's been
// signalled, it won't ever get reset (become unsignalled). Instead, if we want to reuse underlying
// OS objects, it should be reset and recycled *after* the SystemEventReceiver and
// SystemEventPipeSender have been destroyed.
class SystemEventReceiver final : NonCopyable {
  public:
    static SystemEventReceiver CreateAlreadySignaled();

    SystemEventReceiver() = default;
    explicit SystemEventReceiver(SystemHandle primitive);
    SystemEventReceiver(SystemEventReceiver&&) = default;
    SystemEventReceiver& operator=(SystemEventReceiver&&) = default;

    const SystemHandle& GetPrimitive() const;

  private:
    template <typename It>
    friend bool WaitAnySystemEvent(It begin, It end, Nanoseconds timeout);
    friend std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();
    SystemHandle mPrimitive;
};

// See CreateSystemEventPipe.
class SystemEventPipeSender final : NonCopyable {
  public:
    SystemEventPipeSender() = default;
    SystemEventPipeSender(SystemEventPipeSender&&) = default;
    SystemEventPipeSender& operator=(SystemEventPipeSender&&) = default;
    ~SystemEventPipeSender();

    bool IsValid() const;
    void Signal() &&;

  private:
    friend std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();
    SystemHandle mPrimitive;
};

// CreateSystemEventPipe provides an SystemEventReceiver that can be signalled by Dawn code. This is
// useful for queue completions on Metal (where Metal signals us by calling a callback) and for
// async pipeline creations that happen in a worker-thread task.
//
// We use OS events even for these because, unlike C++/pthreads primitives (mutexes, atomics,
// condvars, etc.), it's possible to wait-any on them (wait for any of a list of events to fire).
// Other use-cases in Dawn that don't require wait-any should generally use C++ primitives, for
// example for signalling the completion of other types of worker-thread work that don't need to
// signal a WGPUFuture.
//
// SystemEventReceiver is one-time-use (see its docs), so there's no way to reset an
// SystemEventPipeSender.
//
// - On Windows, SystemEventReceiver is a Win32 Event Object, so we can create one with
//   CreateEvent() and signal it with SetEvent().
// - On POSIX, SystemEventReceiver is a file descriptor (fd), so we can create one with pipe(), and
//   signal it by write()ing into the pipe (to make it become readable, though we won't read() it).
std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();

class SystemEvent : public RefCounted {
  public:
    using RefCounted::RefCounted;

    static Ref<SystemEvent> CreateSignaled();
    static Ref<SystemEvent> CreateNonProgressingEvent();

    bool IsProgressing() const;
    bool IsSignaled() const;
    void Signal();

    // Lazily create a system event receiver. Immediately after this receiver
    // is signaled, IsSignaled should always return true.
    const SystemEventReceiver& GetOrCreateSystemEventReceiver();

  private:
    // Some SystemEvents may be non-progressing, i.e. DeviceLost. We tag these events so that we can
    // correctly return whether there is progressing work when users are polling.
    static constexpr uint64_t kNonProgressingPayload = 1;

    // mSignaled indicates whether the event has already been signaled.
    // It is stored outside the mPipe mutex so its status can quickly be checked without
    // acquiring a lock.
    std::atomic<bool> mSignaled{false};
    MutexProtected<std::optional<std::pair<SystemEventPipeSender, SystemEventReceiver>>> mPipe;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SYSTEMEVENT_H_
