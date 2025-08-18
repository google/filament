// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file and library_webgpu.js together implement <webgpu/webgpu.h>.
//

#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// Used for "implementation-defined logging" per webgpu.h spec.
// This is a no-op in NDEBUG builds, to reduce code-size.
#ifndef NDEBUG
#include <cstdio>
#define DEBUG_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define DCHECK_PRINTF(condition, ...) \
  do {                                \
    if (!(condition)) {               \
      fprintf(stderr, __VA_ARGS__);   \
      assert(false);                  \
    }                                 \
  } while (0)
#else
#define DEBUG_PRINTF(...)
#define DCHECK_PRINTF(...)
#endif

using FutureID = uint64_t;
static constexpr FutureID kNullFutureId = 0;
using InstanceID = uint64_t;
static constexpr InstanceID kNullInstanceId = 0;

static constexpr size_t kTimedWaitAnyMaxCountMax = INT32_MAX;
static constexpr size_t kTimedWaitAnyMaxCountDefault = 64;

// ----------------------------------------------------------------------------
// Declarations for JS emwgpu functions (defined in library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {
void emwgpuDelete(void* ptr);
void emwgpuSetLabel(void* ptr, const char* data, size_t length);

// Note that for the JS entry points, we pass the FutureIDs as uint64_t pointer
// and deref it in JS on the other side. The timeout, however, is converted to
// a int32_t because it will be used with JS setTimeout which actually takes a
// int32_t.
double emwgpuWaitAny(FutureID const* futurePtr,
                     size_t futureCount,
                     int32_t const* timeoutMSPtr);
WGPUTextureFormat emwgpuGetPreferredFormat();

// Device functions, i.e. creation functions to create JS backing objects given
// a pre-allocated handle, and destruction implementations.
[[nodiscard]] bool emwgpuDeviceCreateBuffer(
    WGPUDevice device,
    const WGPUBufferDescriptor* descriptor,
    WGPUBuffer buffer);
void emwgpuDeviceCreateShaderModule(
    WGPUDevice device,
    const WGPUShaderModuleDescriptor* descriptor,
    WGPUShaderModule shader);
void emwgpuDeviceDestroy(WGPUDevice device);

// Buffer mapping operations that has work that needs to be done on the JS side.
void emwgpuBufferDestroy(WGPUBuffer buffer);
const void* emwgpuBufferGetConstMappedRange(WGPUBuffer buffer,
                                            size_t offset,
                                            size_t size);
void* emwgpuBufferGetMappedRange(WGPUBuffer buffer, size_t offset, size_t size);
WGPUStatus emwgpuBufferWriteMappedRange(WGPUBuffer buffer,
                                        size_t offset,
                                        void const* data,
                                        size_t size);
WGPUStatus emwgpuBufferReadMappedRange(WGPUBuffer buffer,
                                       size_t offset,
                                       void* data,
                                       size_t size);
void emwgpuBufferUnmap(WGPUBuffer buffer);

// Future/async operation that need to be forwarded to JS.
void emwgpuAdapterRequestDevice(WGPUAdapter adapter,
                                FutureID futureId,
                                FutureID deviceLostFutureId,
                                WGPUDevice device,
                                WGPUQueue queue,
                                const WGPUDeviceDescriptor* descriptor);
void emwgpuBufferMapAsync(WGPUBuffer buffer,
                          FutureID futureID,
                          WGPUMapMode mode,
                          size_t offset,
                          size_t size);
void emwgpuDeviceCreateComputePipelineAsync(
    WGPUDevice device,
    FutureID futureId,
    const WGPUComputePipelineDescriptor* descriptor,
    WGPUComputePipeline pipeline);
void emwgpuDeviceCreateRenderPipelineAsync(
    WGPUDevice device,
    FutureID futureId,
    const WGPURenderPipelineDescriptor* descriptor,
    WGPURenderPipeline pipeline);
void emwgpuDevicePopErrorScope(WGPUDevice device, FutureID futureId);
void emwgpuInstanceRequestAdapter(WGPUInstance instance,
                                  FutureID futureId,
                                  const WGPURequestAdapterOptions* options,
                                  WGPUAdapter adapter);
void emwgpuQueueOnSubmittedWorkDone(WGPUQueue queue, FutureID futureId);
void emwgpuShaderModuleGetCompilationInfo(WGPUShaderModule shader,
                                          FutureID futureId,
                                          WGPUCompilationInfo* compilationInfo);
}  // extern "C"

namespace {

// ----------------------------------------------------------------------------
// Implementation details that are not exposed upwards in the API.
// ----------------------------------------------------------------------------

class NonCopyable {
 protected:
  constexpr NonCopyable() = default;
  ~NonCopyable() = default;

  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&&) = default;

 private:
  NonCopyable(const NonCopyable&) = delete;
  void operator=(const NonCopyable&) = delete;
};

class NonMovable : NonCopyable {
 protected:
  constexpr NonMovable() = default;
  ~NonMovable() = default;

 private:
  NonMovable(NonMovable&&) = delete;
  void operator=(NonMovable&&) = delete;
};

// For some objects we may do additional cleanup routines, i.e. Destroy if the
// object was natively created via the API. However, if the object was imported
// from JS, we don't do the additional cleanup because they may still be used
// outside of the Wasm API.
struct ImportedFromJSTag {};
static constexpr ImportedFromJSTag kImportedFromJS;

class RefCounted : NonMovable {
 public:
  static constexpr bool HasExternalRefCount = false;

  explicit RefCounted(ImportedFromJSTag) : mIsImportedFromJS(true) {}
  RefCounted() = default;

  void AddRef() {
    [[maybe_unused]] uint64_t oldRefCount =
        mRefCount.fetch_add(1u, std::memory_order_relaxed);
    assert(oldRefCount >= 1);
  }

  bool Release() {
    if (mRefCount.fetch_sub(1u, std::memory_order_release) == 1u) {
      std::atomic_thread_fence(std::memory_order_acquire);
      return true;
    }
    return false;
  }

  bool IsImported() const { return mIsImportedFromJS; }

 private:
  std::atomic<uint64_t> mRefCount = 1;
  bool mIsImportedFromJS = false;
};

class RefCountedWithExternalCount : public RefCounted {
 public:
  static constexpr bool HasExternalRefCount = true;

  explicit RefCountedWithExternalCount(ImportedFromJSTag tag)
      : RefCounted(tag) {}
  RefCountedWithExternalCount() = default;
  virtual ~RefCountedWithExternalCount() = default;

  void AddRef() {
    AddExternalRef();
    RefCounted::AddRef();
  }

  bool Release() {
    if (mExternalRefCount.fetch_sub(1u, std::memory_order_release) == 1u) {
      std::atomic_thread_fence(std::memory_order_acquire);
      WillDropLastExternalRef();
    }
    return RefCounted::Release();
  }

  void AddExternalRef() {
    mExternalRefCount.fetch_add(1u, std::memory_order_relaxed);
  }

 private:
  virtual void WillDropLastExternalRef() = 0;

  std::atomic<uint64_t> mExternalRefCount = 0;
};

template <typename T>
class Ref {
 public:
  Ref() : mValue(nullptr) {
    static_assert(std::is_convertible_v<T*, RefCounted*>,
                  "Cannot make a Ref<T> when T is not a Refcounted type.");
  }
  ~Ref() { Release(mValue); }

  // Constructors from nullptr.
  // NOLINTNEXTLINE(runtime/explicit)
  constexpr Ref(std::nullptr_t) : Ref() {}

  // Constructors from T*.
  // NOLINTNEXTLINE(runtime/explicit)
  Ref(T* value) : mValue(value) { AddRef(value); }
  Ref<T>& operator=(T* value) {
    Set(value);
    return *this;
  }

  // Constructors from a Ref<T>.
  Ref(const Ref<T>& other) : mValue(other.mValue) { AddRef(other.mValue); }
  Ref<T>& operator=(const Ref<T>& other) {
    Set(other.mValue);
    return *this;
  }
  Ref(Ref<T>&& other) { mValue = other.Detach(); }
  Ref<T>& operator=(Ref<T>&& other) {
    if (&other != this) {
      Release(mValue);
      mValue = other.Detach();
    }
    return *this;
  }

  explicit operator bool() const { return !!mValue; }

  // Smart pointer methods.
  const T* Get() const { return mValue; }
  T* Get() { return mValue; }
  const T* operator->() const { return mValue; }
  T* operator->() { return mValue; }

  [[nodiscard]] T* Detach() {
    T* value = mValue;
    mValue = nullptr;
    return value;
  }

  void Acquire(T* value) {
    Release(mValue);
    mValue = value;
  }

 private:
  static void AddRef(T* value) {
    if (value != nullptr) {
      value->RefCounted::AddRef();
    }
  }
  static void Release(T* value) {
    if (value != nullptr && value->RefCounted::Release()) {
      delete value;
      // emwgpuDelete() removes the pointer from the jsObjects mapping.
      // Considering some class implementation may need to use it in
      // destructor, we call emwgpuDelete() after the pointer delete.
      // This also applies to implementation of `wgpu{Type}Release`.
      emwgpuDelete(value);
    }
  }

  void Set(T* value) {
    if (mValue != value) {
      // Ensure that the new value is referenced before the old is released to
      // prevent any transitive frees that may affect the new value.
      AddRef(value);
      Release(mValue);
      mValue = value;
    }
  }

  T* mValue;
};

template <typename T>
Ref<T> AcquireRef(T* pointee) {
  Ref<T> ref;
  ref.Acquire(pointee);
  return ref;
}

template <typename T>
auto ReturnToAPI(Ref<T>&& object) {
  if constexpr (T::HasExternalRefCount) {
    // For an object which has external ref count, just need to increase the
    // external ref count, and keep the total ref count unchanged.
    object->AddExternalRef();
  }
  return object.Detach();
}

// StringView utilities.
WGPUStringView ToOutputStringView(const std::string& s) {
  return {s.data(), s.size()};
}

// clang-format off
// X Macro to help generate boilerplate code for all refcounted object types.
#define WGPU_OBJECTS(X) \
  X(Adapter)             \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(Buffer)              \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(Device)              \
  X(Instance)            \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(Queue)               \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(ShaderModule)        \
  X(Surface)             \
  X(Texture)             \
  X(TextureView)

// X Macro to help generate boilerplate code for all passthrough object types.
// Passthrough objects refer to objects that are implemented via JS objects.
#define WGPU_PASSTHROUGH_OBJECTS(X) \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(Surface)             \
  X(Texture)             \
  X(TextureView)
// clang-format on

// ----------------------------------------------------------------------------
// Future related structures and helpers.
// ----------------------------------------------------------------------------

enum class EventCompletionType {
  Ready,
  Shutdown,
};
enum class EventType {
  CompilationInfo,
  CreateComputePipeline,
  CreateRenderPipeline,
  DeviceLost,
  MapAsync,
  PopErrorScope,
  RequestAdapter,
  RequestDevice,
  WorkDone,
};

bool ValidateCallbackMode(WGPUCallbackMode mode, bool hasCallback) {
  if (mode == WGPUCallbackMode_WaitAnyOnly ||
      mode == WGPUCallbackMode_AllowProcessEvents ||
      mode == WGPUCallbackMode_AllowSpontaneous ||
      (int(mode) == 0 && !hasCallback)) {
    return true;
  } else {
    DEBUG_PRINTF("Invalid WGPUCallbackMode %d\n", mode);
    return false;
  }
}

template <typename CallbackInfo>
bool ValidateCallbackMode(const CallbackInfo& info) {
  // This small templated function delegates to the larger non-templated
  // function to avoid unnecessary monomorphization.
  return ValidateCallbackMode(info.mode, info.callback);
}

class EventManager;

class TrackedEvent : NonMovable {
 public:
  virtual ~TrackedEvent() = default;
  virtual EventType GetType() = 0;
  virtual void Complete(FutureID futureId, EventCompletionType type) = 0;

 protected:
  TrackedEvent(InstanceID instance, WGPUCallbackMode mode)
      : mInstanceId(instance),
        // Mode can only be 0 if there's no callback, so just pick any default.
        mMode(int(mode) == 0 ? WGPUCallbackMode_AllowSpontaneous : mode) {
    assert(mMode == WGPUCallbackMode_WaitAnyOnly ||
           mMode == WGPUCallbackMode_AllowProcessEvents ||
           mMode == WGPUCallbackMode_AllowSpontaneous);
  }

 private:
  friend EventManager;

  // Events need to keep track of the instance they came from for validation.
  const InstanceID mInstanceId;
  const WGPUCallbackMode mMode;
  bool mIsReady = false;
};

// Compositable class for objects that provide entry point(s) that produce
// Events, i.e. returns a Future.
//
// Note that if passing pointers between JS and C++, to ensure that EventSource
// inheritance is handled properly, EventSource must be the first class
// inherited by subclasses. Otherwise the pointer is not cast properly and
// results in corrupted data. As an example, given:
//   (1) WGPUAdapter emwgpuCreateAdapter(const EventSource* source);
//   (2) WGPUAdapter emwgpuCreateAdapter(WGPUInstance instance);
// WGPUInstance **must** list EventSource as it's first inherited class for (1)
// to work.
class EventSource {
 public:
  explicit EventSource(InstanceID instanceId) : mInstanceId(instanceId) {}
  explicit EventSource(const EventSource* source)
      : mInstanceId(source ? source->GetInstanceId() : kNullInstanceId) {}
  InstanceID GetInstanceId() const { return mInstanceId; }

 private:
  const InstanceID mInstanceId = 0;
};

// Thread-safe EventManager class that tracks all events.
//
// Note that there is a single global EventManager that should be accessed via
// GetEventManager(). The EventManager needs to outlive all WGPUInstances in
// order to handle Spontaneous events.
class EventManager : NonMovable {
 public:
  EventManager() {
    // We set up a tracker for events that are registered against a null
    // Instance because devices may have been created and injected before the
    // Instance was created.
    // TODO(crbug.com/388914937): Remove this once users are updated.
    std::unique_lock<std::mutex> lock(mMutex);
    mPerInstanceEvents.try_emplace(kNullInstanceId);
  }

  void RegisterInstance(InstanceID instance) {
    assert(instance);
    std::unique_lock<std::mutex> lock(mMutex);
    mPerInstanceEvents.try_emplace(instance);
  }

  void UnregisterInstance(InstanceID instance) {
    assert(instance);
    std::unique_lock<std::mutex> lock(mMutex);
    auto instanceIt = mPerInstanceEvents.find(instance);
    assert(instanceIt != mPerInstanceEvents.end());

    // When unregistering the Instance, resolve all non-spontaneous callbacks
    // with Shutdown.
    for (const FutureID futureId : instanceIt->second) {
      if (auto futureIdsIt = mEvents.find(futureId);
          futureIdsIt != mEvents.end()) {
        futureIdsIt->second->Complete(futureId, EventCompletionType::Shutdown);
        mEvents.erase(futureIdsIt);
      }
    }
    mPerInstanceEvents.erase(instance);
  }

  void ProcessEvents(InstanceID instance) {
    assert(instance);
    std::vector<std::pair<FutureID, std::unique_ptr<TrackedEvent>>> completable;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto instanceIt = mPerInstanceEvents.find(instance);
      assert(instanceIt != mPerInstanceEvents.end());
      auto& instanceFutureIds = instanceIt->second;

      // Note that we are only currently handling AllowProcessEvents events,
      // i.e. we are not handling AllowSpontaneous events in this loop.
      for (auto futureIdsIt = instanceFutureIds.begin();
           futureIdsIt != instanceFutureIds.end();) {
        FutureID futureId = *futureIdsIt;
        auto eventIt = mEvents.find(futureId);
        if (eventIt == mEvents.end()) {
          ++futureIdsIt;
          continue;
        }
        auto& event = eventIt->second;

        if (event->mMode == WGPUCallbackMode_AllowProcessEvents &&
            event->mIsReady) {
          completable.emplace_back(futureId, std::move(event));
          mEvents.erase(eventIt);
          futureIdsIt = instanceFutureIds.erase(futureIdsIt);
        } else {
          ++futureIdsIt;
        }
      }
    }

    // Since the sets are ordered, the events must already be ordered by
    // FutureID.
    for (auto& [futureId, event] : completable) {
      event->Complete(futureId, EventCompletionType::Ready);
    }
  }

  WGPUWaitStatus WaitAny(InstanceID instance,
                         size_t count,
                         WGPUFutureWaitInfo* infos,
                         uint64_t timeoutNS) {
    assert(instance);

    if (count == 0) {
      return WGPUWaitStatus_Success;
    }

    // To handle timeouts, use Asyncify and proxy back into JS.
    if (timeoutNS > 0) {
      // Should have already been validated in WGPUInstanceImpl::WaitAny.
      assert(emscripten_has_asyncify());

      std::vector<FutureID> futures;
      std::unordered_map<FutureID, WGPUFutureWaitInfo*> futureIdToInfo;
      for (size_t i = 0; i < count; ++i) {
        FutureID id = infos[i].future.id;
        DCHECK_PRINTF(id != kNullFutureId && id < mNextFutureId,
                      "Invalid future id %" PRIu64 "\n", id);
        futures.push_back(id);
        futureIdToInfo.emplace(id, &infos[i]);
      }

      // We need to clamp and convert the timeout to verify that the timeout in
      // ms is less than the maximum value that JS setTimeout can take. If it
      // would exceed the timeout, just assume that we are waiting until
      // completion.
      static constexpr uint64_t kNsInMs = 1000000;
      uint64_t timeoutMS64 = timeoutNS / kNsInMs;
      bool hasTimeout = timeoutMS64 <= INT32_MAX;
      int32_t timeoutMS = static_cast<int32_t>(timeoutMS64);

      FutureID completedId = static_cast<FutureID>(emwgpuWaitAny(
          futures.data(), count, hasTimeout ? &timeoutMS : nullptr));
      if (completedId == kNullFutureId) {
        return WGPUWaitStatus_TimedOut;
      }
      futureIdToInfo[completedId]->completed = true;

      std::unique_ptr<TrackedEvent> completed;
      {
        std::unique_lock<std::mutex> lock(mMutex);
        auto eventIt = mEvents.find(completedId);
        if (eventIt == mEvents.end()) {
          return WGPUWaitStatus_Success;
        }

        completed = std::move(eventIt->second);
        mEvents.erase(eventIt);
        if (auto instanceIt = mPerInstanceEvents.find(instance);
            instanceIt != mPerInstanceEvents.end()) {
          instanceIt->second.erase(completedId);
        }
      }

      if (completed) {
        completed->Complete(completedId, EventCompletionType::Ready);
      }
      return WGPUWaitStatus_Success;
    }

    std::map<FutureID, std::unique_ptr<TrackedEvent>> completable;
    bool anyCompleted = false;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto instanceIt = mPerInstanceEvents.find(instance);
      assert(instanceIt != mPerInstanceEvents.end());
      auto& instanceFutureIds = instanceIt->second;

      for (size_t i = 0; i < count; ++i) {
        FutureID futureId = infos[i].future.id;
        auto eventIt = mEvents.find(futureId);
        if (eventIt == mEvents.end()) {
          infos[i].completed = true;
          anyCompleted = true;
          continue;
        }

        auto& event = eventIt->second;
        assert(event->mInstanceId == instance);
        infos[i].completed = event->mIsReady;
        if (event->mIsReady) {
          anyCompleted = true;
          completable.emplace(futureId, std::move(event));
          mEvents.erase(eventIt);
          instanceFutureIds.erase(futureId);
        }
      }
    }

    // We used an ordered map to collect the events, so they must be ordered.
    for (auto& [futureId, event] : completable) {
      event->Complete(futureId, EventCompletionType::Ready);
    }
    return anyCompleted ? WGPUWaitStatus_Success : WGPUWaitStatus_TimedOut;
  }

  FutureID TrackEvent(std::unique_ptr<TrackedEvent> event) {
    FutureID futureId = mNextFutureId++;
    InstanceID instance = event->mInstanceId;
    std::unique_lock<std::mutex> lock(mMutex);
    switch (event->mMode) {
      case WGPUCallbackMode_WaitAnyOnly:
      case WGPUCallbackMode_AllowProcessEvents: {
        auto it = mPerInstanceEvents.find(instance);
        if (it == mPerInstanceEvents.end()) {
          // The instance has already been unregistered so just complete this
          // event as shutdown now.
          event->Complete(futureId, EventCompletionType::Shutdown);
          return futureId;
        }
        it->second.insert(futureId);
        break;
      }
      // Any invalid callback mode should already have been validated by
      // ValidateCallbackMode, but it still may be 0 if there's no callback.
      default:
      case WGPUCallbackMode_AllowSpontaneous:
        break;
    }
    mEvents.try_emplace(futureId, std::move(event));
    return futureId;
  }

  template <typename Event, typename... ReadyArgs>
  void SetFutureReady(FutureID futureId, ReadyArgs&&... readyArgs) {
    assert(futureId != kNullFutureId);
    std::unique_ptr<TrackedEvent> spontaneousEvent;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto eventIt = mEvents.find(futureId);
      if (eventIt == mEvents.end()) {
        return;
      }

      auto& event = eventIt->second;
      assert(event->GetType() == Event::kType);
      static_cast<Event*>(event.get())
          ->ReadyHook(std::forward<ReadyArgs>(readyArgs)...);
      event->mIsReady = true;

      // If the event can be spontaneously completed, prepare to do so now.
      if (event->mMode == WGPUCallbackMode_AllowSpontaneous) {
        spontaneousEvent = std::move(event);
        mEvents.erase(futureId);
      }
    }

    if (spontaneousEvent) {
      spontaneousEvent->Complete(futureId, EventCompletionType::Ready);
    }
  }
  template <typename Event, typename... ReadyArgs>
  void SetFutureReady(double futureId, ReadyArgs&&... readyArgs) {
    SetFutureReady<Event>(static_cast<uint64_t>(futureId),
                          std::forward<ReadyArgs>(readyArgs)...);
  }

 private:
  std::mutex mMutex;
  std::atomic<FutureID> mNextFutureId = 1;

  // The EventManager separates events based on the WGPUInstance that the event
  // stems from.
  std::unordered_map<InstanceID, std::set<FutureID>> mPerInstanceEvents;
  std::unordered_map<FutureID, std::unique_ptr<TrackedEvent>> mEvents;
};

static EventManager& GetEventManager() {
  static EventManager kEventManager;
  return kEventManager;
}

}  // namespace

// ----------------------------------------------------------------------------
// WGPU struct declarations.
// ----------------------------------------------------------------------------

// Default struct declarations.
#define DEFINE_WGPU_DEFAULT_STRUCT(Name)              \
  struct WGPU##Name##Impl final : public RefCounted { \
    WGPU##Name##Impl(const EventSource* = nullptr) {} \
  };
WGPU_PASSTHROUGH_OBJECTS(DEFINE_WGPU_DEFAULT_STRUCT)

struct WGPUAdapterImpl final : public EventSource, public RefCounted {
 public:
  WGPUAdapterImpl(const EventSource* source);
};

namespace {
// Forward declare so this can be used as a friend below.
class MapAsyncEvent;
}  // namespace

struct WGPUBufferImpl final : public EventSource,
                              public RefCountedWithExternalCount {
 public:
  WGPUBufferImpl(const EventSource* source, bool mappedAtCreation);
  // Injection constructor used when we already have a backing Buffer.
  WGPUBufferImpl(const EventSource* source, WGPUBufferMapState mapState);

  void Destroy();
  const void* GetConstMappedRange(size_t offset, size_t size);
  WGPUBufferMapState GetMapState() const;
  void* GetMappedRange(size_t offset, size_t size);
  WGPUStatus WriteMappedRange(size_t offset, void const* data, size_t size);
  WGPUStatus ReadMappedRange(size_t offset, void* data, size_t size);
  WGPUFuture MapAsync(WGPUMapMode mode,
                      size_t offset,
                      size_t size,
                      WGPUBufferMapCallbackInfo callbackInfo);
  void Unmap();

 private:
  friend MapAsyncEvent;

  void WillDropLastExternalRef() override;

  bool IsPendingMapRequest(FutureID futureID) const;
  void AbortPendingMap(const char* message);

  // Encapsulates information about a map request. Note that when
  // futureID == kNullFutureId, there are no pending map requests, however, it
  // is still possible that we are still "mapped" because of mappedAtCreation
  // which is not associated with a particular async map / future.
  struct MapRequest {
    FutureID futureID = kNullFutureId;
    WGPUMapMode mode = WGPUMapMode_None;
  };
  MapRequest mPendingMapRequest;
  WGPUBufferMapState mMapState;
};

struct WGPUQueueImpl final : public EventSource, public RefCounted {
 public:
  WGPUQueueImpl(const EventSource* source);
};

enum class DropDeviceFromDeviceLostEvent : bool { No, Yes };

// Device is specially implemented in order to handle refcounting the Queue.
struct WGPUDeviceImpl final : public EventSource,
                              public RefCountedWithExternalCount {
 public:
  // Reservation constructor used when calling RequestDevice.
  WGPUDeviceImpl(const EventSource* source,
                 const WGPUDeviceDescriptor& descriptor,
                 WGPUQueue queue);
  // Injection constructor used when we already have a backing Device.
  WGPUDeviceImpl(const EventSource* source, WGPUQueue queue);

  void Destroy(DropDeviceFromDeviceLostEvent dropDevice);
  WGPUQueue GetQueue() const;
  WGPUFuture GetLostFuture() const;

  void OnUncapturedError(WGPUErrorType type, char const* message);

 private:
  void WillDropLastExternalRef() override;

  Ref<WGPUQueueImpl> mQueue;
  WGPUUncapturedErrorCallbackInfo mUncapturedErrorCallbackInfo =
      WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT;
  FutureID mDeviceLostFutureId = kNullFutureId;
};

// Instance is specially implemented in order to handle Futures implementation.
struct WGPUInstanceImpl final : public EventSource, public RefCounted {
 public:
  static Ref<WGPUInstanceImpl> Create(const WGPUInstanceDescriptor* desc);
  ~WGPUInstanceImpl();

  void ProcessEvents();
  WGPUWaitStatus WaitAny(size_t count,
                         WGPUFutureWaitInfo* infos,
                         uint64_t timeoutNS);

 private:
  WGPUInstanceImpl();

  static InstanceID GetNextInstanceId();

  // Zero means TimedWaitAny is not supported at all.
  size_t mTimedWaitAnyMaxCount = 0;
};

namespace {
// Forward declare so this can be used as a friend below.
class CompilationInfoEvent;
}  // namespace

struct WGPUShaderModuleImpl final : public EventSource, public RefCounted {
 public:
  WGPUShaderModuleImpl(const EventSource* source);

  WGPUFuture GetCompilationInfo(WGPUCompilationInfoCallbackInfo callbackInfo);

 private:
  friend CompilationInfoEvent;

  struct WGPUCompilationInfoDeleter {
    void operator()(WGPUCompilationInfo* compilationInfo) {
      if (!compilationInfo) {
        return;
      }

      // Free allocations iff there were any.
      if (compilationInfo->messageCount) {
        // Since we allocate all the messages in a single block, we only need to
        // free the first pointer.
        free(const_cast<char*>(compilationInfo->messages[0].message.data));
        // We also allocate all the Utf16 structs in a single array.
        free(reinterpret_cast<WGPUDawnCompilationMessageUtf16*>(
            compilationInfo->messages[0].nextInChain));
        // Finally, free the array of messages.
        free(const_cast<WGPUCompilationMessage*>(compilationInfo->messages));
      }
      delete compilationInfo;
    }
  };
  using CompilationInfo =
      std::unique_ptr<WGPUCompilationInfo, WGPUCompilationInfoDeleter>;
  CompilationInfo mCompilationInfo = nullptr;
};

// ----------------------------------------------------------------------------
// Future events.
// ----------------------------------------------------------------------------

namespace {

class CompilationInfoEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::CompilationInfo;

  CompilationInfoEvent(InstanceID instance,
                       WGPUShaderModule shader,
                       const WGPUCompilationInfoCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2),
        mShader(shader) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUCompilationInfoRequestStatus status,
                 WGPUCompilationInfo* compilationInfo) {
    WGPUShaderModuleImpl::CompilationInfo info(compilationInfo);
    mStatus = status;
    if (mStatus != WGPUCompilationInfoRequestStatus_Success) {
      return;
    }

    if (!mShader->mCompilationInfo.get()) {
      // If there wasn't already a cached version of the info, set it now.
      mShader->mCompilationInfo = std::move(info);
    }
    assert(mShader->mCompilationInfo.get());
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUCompilationInfoRequestStatus_CallbackCancelled;
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPUCompilationInfoRequestStatus_Success
                    ? mShader->mCompilationInfo.get()
                    : nullptr,
                mUserdata1, mUserdata2);
    }
  }

 private:
  WGPUCompilationInfoCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  Ref<WGPUShaderModuleImpl> mShader;
  WGPUCompilationInfoRequestStatus mStatus =
      WGPUCompilationInfoRequestStatus_Success;
};

template <typename PipelineImpl, EventType Type, typename CallbackInfo>
class CreatePipelineEventBase final : public TrackedEvent {
 public:
  static constexpr EventType kType = Type;

  CreatePipelineEventBase(InstanceID instance, const CallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUCreatePipelineAsyncStatus status,
                 PipelineImpl* pipeline,
                 const char* message) {
    mStatus = status;
    mPipeline.Acquire(pipeline);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUCreatePipelineAsyncStatus_CallbackCancelled;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPUCreatePipelineAsyncStatus_Success
                    ? ReturnToAPI(std::move(mPipeline))
                    : nullptr,
                ToOutputStringView(mMessage), mUserdata1, mUserdata2);
    }
  }

 private:
  using Callback = decltype(std::declval<CallbackInfo>().callback);
  Callback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
  Ref<PipelineImpl> mPipeline;
  std::string mMessage;
};
using CreateComputePipelineEvent =
    CreatePipelineEventBase<WGPUComputePipelineImpl,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<WGPURenderPipelineImpl,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo>;

class DeviceLostEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::DeviceLost;

  DeviceLostEvent(InstanceID instance,
                  WGPUDevice device,
                  const WGPUDeviceLostCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2),
        mDevice(device) {
    assert(mDevice);
  }

  EventType GetType() override { return kType; }

  // If dropDevice=Yes, drops the DeviceLostEvent->WGPUDeviceImpl ref so the
  // device won't be passed to the callback, in preparation to free the device.
  void ReadyHook(DropDeviceFromDeviceLostEvent dropDevice,
                 WGPUDeviceLostReason reason,
                 const char* message) {
    if (dropDevice == DropDeviceFromDeviceLostEvent::Yes) {
      mDevice = nullptr;
    }

    mReason = reason;
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mReason = WGPUDeviceLostReason_CallbackCancelled;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      WGPUDevice device = mDevice.Get();
      mCallback(&device, mReason, ToOutputStringView(mMessage), mUserdata1,
                mUserdata2);
    }
  }

 private:
  WGPUDeviceLostCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  Ref<WGPUDeviceImpl> mDevice;

  WGPUDeviceLostReason mReason;
  std::string mMessage;
};

class PopErrorScopeEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::PopErrorScope;

  PopErrorScopeEvent(InstanceID instance,
                     const WGPUPopErrorScopeCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUPopErrorScopeStatus status,
                 WGPUErrorType errorType,
                 const char* message) {
    mStatus = status;
    mErrorType = errorType;
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUPopErrorScopeStatus_CallbackCancelled;
      mErrorType = WGPUErrorType_NoError;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus, mErrorType, ToOutputStringView(mMessage), mUserdata1,
                mUserdata2);
    }
  }

 private:
  WGPUPopErrorScopeCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPUPopErrorScopeStatus mStatus = WGPUPopErrorScopeStatus_Success;
  WGPUErrorType mErrorType = WGPUErrorType_Unknown;
  std::string mMessage;
};

class MapAsyncEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::MapAsync;

  MapAsyncEvent(InstanceID instance,
                WGPUBuffer buffer,
                const WGPUBufferMapCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2),
        mBuffer(buffer) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUMapAsyncStatus status, const char* message) {
    // For mapping, this hook may be called more than once if we are not in
    // Spontaneous mode. The precedence of which status should follow
    // Success < Error < Aborted. Luckily, the enum is defined such that the
    // precedence holds true already, so we can exploit that here.
    static_assert(WGPUMapAsyncStatus_Success < WGPUMapAsyncStatus_Error);
    static_assert(WGPUMapAsyncStatus_Error < WGPUMapAsyncStatus_Aborted);
    if (status > mStatus) {
      mStatus = status;
      if (message) {
        mMessage = message;
      }
    }
  }

  void Complete(FutureID futureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUMapAsyncStatus_CallbackCancelled;
      mMessage = "A valid external Instance reference no longer exists.";
    }

    if (mBuffer->IsPendingMapRequest(futureID)) {
      if (mStatus == WGPUMapAsyncStatus_Success) {
        mBuffer->mMapState = WGPUBufferMapState_Mapped;
      } else {
        mBuffer->mMapState = WGPUBufferMapState_Unmapped;
        mBuffer->mPendingMapRequest = {};
      }
    } else {
      assert(mStatus != WGPUMapAsyncStatus_Success);
    }

    if (mCallback) {
      mCallback(mStatus, ToOutputStringView(mMessage), mUserdata1, mUserdata2);
    }
  }

 private:
  WGPUBufferMapCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  Ref<WGPUBufferImpl> mBuffer;
  WGPUMapAsyncStatus mStatus = WGPUMapAsyncStatus_Success;
  std::string mMessage;
};

class RequestAdapterEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::RequestAdapter;

  RequestAdapterEvent(InstanceID instance,
                      const WGPURequestAdapterCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPURequestAdapterStatus status,
                 WGPUAdapter adapter,
                 const char* message) {
    mStatus = status;
    mAdapter.Acquire(adapter);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPURequestAdapterStatus_CallbackCancelled;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPURequestAdapterStatus_Success
                    ? ReturnToAPI(std::move(mAdapter))
                    : nullptr,
                ToOutputStringView(mMessage), mUserdata1, mUserdata2);
    }
  }

 private:
  WGPURequestAdapterCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPURequestAdapterStatus mStatus;
  Ref<WGPUAdapterImpl> mAdapter;
  std::string mMessage;
};

class RequestDeviceEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::RequestDevice;

  RequestDeviceEvent(InstanceID instance,
                     const WGPURequestDeviceCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPURequestDeviceStatus status,
                 WGPUDevice device,
                 const char* message) {
    mStatus = status;
    mDevice.Acquire(device);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPURequestDeviceStatus_CallbackCancelled;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPURequestDeviceStatus_Success
                    ? ReturnToAPI(std::move(mDevice))
                    : nullptr,
                ToOutputStringView(mMessage), mUserdata1, mUserdata2);
    }
  }

 private:
  WGPURequestDeviceCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPURequestDeviceStatus mStatus;
  Ref<WGPUDeviceImpl> mDevice;
  std::string mMessage;
};

class WorkDoneEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::WorkDone;

  WorkDoneEvent(InstanceID instance,
                const WGPUQueueWorkDoneCallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUQueueWorkDoneStatus status) { mStatus = status; }

  void Complete(FutureID, EventCompletionType type) override {
    std::string message;
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUQueueWorkDoneStatus_CallbackCancelled;
      message = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus, ToOutputStringView(message), mUserdata1, mUserdata2);
    }
  }

 private:
  WGPUQueueWorkDoneCallback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPUQueueWorkDoneStatus mStatus;
};

}  // namespace

// ----------------------------------------------------------------------------
// Definitions for C++ emwgpu functions (callable from library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {

// Object creation helpers that all return a pointer which is used as a key
// in the JS object table in library_webgpu.js.
#define DEFINE_EMWGPU_DEFAULT_CREATE(Name)                             \
  WGPU##Name emwgpuCreate##Name(const EventSource* source = nullptr) { \
    return ReturnToAPI(AcquireRef(new WGPU##Name##Impl(source)));      \
  }
WGPU_PASSTHROUGH_OBJECTS(DEFINE_EMWGPU_DEFAULT_CREATE)

WGPUAdapter emwgpuCreateAdapter(const EventSource* source) {
  return ReturnToAPI(AcquireRef(new WGPUAdapterImpl(source)));
}

WGPUBuffer emwgpuCreateBuffer(const EventSource* source,
                              WGPUBufferMapState mapState) {
  return ReturnToAPI(AcquireRef(new WGPUBufferImpl(source, mapState)));
}

WGPUDevice emwgpuCreateDevice(const EventSource* source, WGPUQueue queue) {
  return ReturnToAPI(AcquireRef(new WGPUDeviceImpl(source, queue)));
}

WGPUQueue emwgpuCreateQueue(const EventSource* source) {
  return ReturnToAPI(AcquireRef(new WGPUQueueImpl(source)));
}

WGPUShaderModule emwgpuCreateShaderModule(const EventSource* source) {
  return ReturnToAPI(AcquireRef(new WGPUShaderModuleImpl(source)));
}

// Future event callbacks.
void emwgpuOnCompilationInfoCompleted(double futureId,
                                      WGPUCompilationInfoRequestStatus status,
                                      WGPUCompilationInfo* compilationInfo) {
  GetEventManager().SetFutureReady<CompilationInfoEvent>(futureId, status,
                                                         compilationInfo);
}
void emwgpuOnCreateComputePipelineCompleted(
    double futureId,
    WGPUCreatePipelineAsyncStatus status,
    WGPUComputePipeline pipeline,
    const char* message) {
  assert(pipeline);
  if (status != WGPUCreatePipelineAsyncStatus_Success) {
    delete pipeline;
    pipeline = nullptr;
  }
  GetEventManager().SetFutureReady<CreateComputePipelineEvent>(
      futureId, status, pipeline, message);
}
void emwgpuOnCreateRenderPipelineCompleted(double futureId,
                                           WGPUCreatePipelineAsyncStatus status,
                                           WGPURenderPipeline pipeline,
                                           const char* message) {
  assert(pipeline);
  if (status != WGPUCreatePipelineAsyncStatus_Success) {
    delete pipeline;
    pipeline = nullptr;
  }
  GetEventManager().SetFutureReady<CreateRenderPipelineEvent>(
      futureId, status, pipeline, message);
}
void emwgpuOnDeviceLostCompleted(double futureId,
                                 WGPUDeviceLostReason reason,
                                 const char* message) {
  GetEventManager().SetFutureReady<DeviceLostEvent>(
      futureId, DropDeviceFromDeviceLostEvent::No, reason, message);
}
void emwgpuOnMapAsyncCompleted(double futureId,
                               WGPUMapAsyncStatus status,
                               const char* message) {
  GetEventManager().SetFutureReady<MapAsyncEvent>(futureId, status, message);
}
void emwgpuOnPopErrorScopeCompleted(double futureId,
                                    WGPUPopErrorScopeStatus status,
                                    WGPUErrorType errorType,
                                    const char* message) {
  GetEventManager().SetFutureReady<PopErrorScopeEvent>(futureId, status,
                                                       errorType, message);
}
void emwgpuOnRequestAdapterCompleted(double futureId,
                                     WGPURequestAdapterStatus status,
                                     WGPUAdapter adapter,
                                     const char* message) {
  assert(adapter);
  if (status != WGPURequestAdapterStatus_Success) {
    delete adapter;
    adapter = nullptr;
  }
  GetEventManager().SetFutureReady<RequestAdapterEvent>(futureId, status,
                                                        adapter, message);
}
void emwgpuOnRequestDeviceCompleted(double futureId,
                                    WGPURequestDeviceStatus status,
                                    WGPUDevice device,
                                    const char* message) {
  // This handler should always have a device since we pre-allocate it before
  // calling out to JS.
  assert(device);
  if (status == WGPURequestDeviceStatus_Success) {
    GetEventManager().SetFutureReady<RequestDeviceEvent>(futureId, status,
                                                         device, message);
  } else {
    // If the request failed, we need to resolve the DeviceLostEvent.
    GetEventManager().SetFutureReady<RequestDeviceEvent>(futureId, status,
                                                         nullptr, message);
    GetEventManager().SetFutureReady<DeviceLostEvent>(
        device->GetLostFuture().id, DropDeviceFromDeviceLostEvent::Yes,
        WGPUDeviceLostReason_FailedCreation, "Device creation failed.");

    // Free the device now that there should be no pointers to it.
    [[maybe_unused]] bool deviceFreed = device->Release();
    assert(deviceFreed);
  }
}
void emwgpuOnWorkDoneCompleted(double futureId,
                               WGPUQueueWorkDoneStatus status) {
  GetEventManager().SetFutureReady<WorkDoneEvent>(futureId, status);
}

// Uncaptured error handler is similar to the Future event callbacks, but it
// doesn't go through the EventManager and just calls the callback on the Device
// immediately.
void emwgpuOnUncapturedError(WGPUDevice device,
                             WGPUErrorType type,
                             char const* message) {
  device->OnUncapturedError(type, message);
}

}  // extern "C"

// ----------------------------------------------------------------------------
// WGPU struct implementations.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// WGPUAdapterImpl implementations.
// ----------------------------------------------------------------------------

WGPUAdapterImpl::WGPUAdapterImpl(const EventSource* source)
    : EventSource(source) {}

// ----------------------------------------------------------------------------
// WGPUBuffer implementations.
// ----------------------------------------------------------------------------

WGPUBufferImpl::WGPUBufferImpl(const EventSource* source, bool mappedAtCreation)
    : EventSource(source),
      mMapState(mappedAtCreation ? WGPUBufferMapState_Mapped
                                 : WGPUBufferMapState_Unmapped) {
  if (mappedAtCreation) {
    mPendingMapRequest = {kNullFutureId, WGPUMapMode_Write};
  }
}

WGPUBufferImpl::WGPUBufferImpl(const EventSource* source,
                               WGPUBufferMapState mapState)
    : EventSource(source),
      RefCountedWithExternalCount(kImportedFromJS),
      mMapState(mapState) {}

void WGPUBufferImpl::Destroy() {
  emwgpuBufferDestroy(this);
  AbortPendingMap("Buffer was destroyed before mapping was resolved.");
}

const void* WGPUBufferImpl::GetConstMappedRange(size_t offset, size_t size) {
  if (mMapState != WGPUBufferMapState_Mapped) {
    return nullptr;
  }
  return emwgpuBufferGetConstMappedRange(this, offset, size);
}

WGPUBufferMapState WGPUBufferImpl::GetMapState() const {
  return mMapState;
}

void* WGPUBufferImpl::GetMappedRange(size_t offset, size_t size) {
  if (mMapState != WGPUBufferMapState_Mapped) {
    return nullptr;
  }
  if (mPendingMapRequest.mode != WGPUMapMode_Write) {
    assert(false);
    return nullptr;
  }

  return emwgpuBufferGetMappedRange(this, offset, size);
}

WGPUStatus WGPUBufferImpl::WriteMappedRange(size_t offset,
                                            void const* data,
                                            size_t size) {
  if (mMapState != WGPUBufferMapState_Mapped) {
    return WGPUStatus_Error;
  }
  if (mPendingMapRequest.mode != WGPUMapMode_Write) {
    assert(false);
    return WGPUStatus_Error;
  }
  return emwgpuBufferWriteMappedRange(this, offset, data, size);
}

WGPUStatus WGPUBufferImpl::ReadMappedRange(size_t offset,
                                           void* data,
                                           size_t size) {
  if (mMapState != WGPUBufferMapState_Mapped) {
    return WGPUStatus_Error;
  }
  return emwgpuBufferReadMappedRange(this, offset, data, size);
}

WGPUFuture WGPUBufferImpl::MapAsync(WGPUMapMode mode,
                                    size_t offset,
                                    size_t size,
                                    WGPUBufferMapCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId = GetEventManager().TrackEvent(
      std::make_unique<MapAsyncEvent>(GetInstanceId(), this, callbackInfo));

  if (mMapState == WGPUBufferMapState_Pending) {
    GetEventManager().SetFutureReady<MapAsyncEvent>(
        futureId, WGPUMapAsyncStatus_Error,
        "Buffer already has an outstanding map pending.");
    return WGPUFuture{futureId};
  }

  assert(mPendingMapRequest.mode == WGPUMapMode_None);
  mMapState = WGPUBufferMapState_Pending;
  mPendingMapRequest = {futureId, mode};

  emwgpuBufferMapAsync(this, futureId, mode, offset, size);
  return WGPUFuture{futureId};
}

void WGPUBufferImpl::Unmap() {
  emwgpuBufferUnmap(this);
  AbortPendingMap("Buffer was unmapped before mapping was resolved.");
}

bool WGPUBufferImpl::IsPendingMapRequest(FutureID futureID) const {
  assert(futureID != kNullFutureId);
  return mPendingMapRequest.futureID == futureID;
}

void WGPUBufferImpl::AbortPendingMap(const char* message) {
  if (mMapState == WGPUBufferMapState_Unmapped) {
    return;
  }

  mMapState = WGPUBufferMapState_Unmapped;

  FutureID futureId = mPendingMapRequest.futureID;
  mPendingMapRequest = {};
  if (futureId == kNullFutureId) {
    // If we were mappedAtCreation, then there is no pending map request so we
    // don't need to resolve any futures.
    return;
  }
  GetEventManager().SetFutureReady<MapAsyncEvent>(
      futureId, WGPUMapAsyncStatus_Aborted, message);
}

void WGPUBufferImpl::WillDropLastExternalRef() {
  AbortPendingMap("Buffer was destroyed before mapping was resolved.");
}

// ----------------------------------------------------------------------------
// WGPUDeviceImpl implementations.
// ----------------------------------------------------------------------------

WGPUDeviceImpl::WGPUDeviceImpl(const EventSource* source,
                               const WGPUDeviceDescriptor& descriptor,
                               WGPUQueue queue)
    : EventSource(source),
      mUncapturedErrorCallbackInfo(descriptor.uncapturedErrorCallbackInfo) {
  // Create the DeviceLostEvent now.
  mDeviceLostFutureId =
      GetEventManager().TrackEvent(std::make_unique<DeviceLostEvent>(
          source->GetInstanceId(), this, descriptor.deviceLostCallbackInfo));
  mQueue.Acquire(queue);
}

WGPUDeviceImpl::WGPUDeviceImpl(const EventSource* source, WGPUQueue queue)
    : EventSource(source), RefCountedWithExternalCount(kImportedFromJS) {
  mQueue.Acquire(queue);
}

void WGPUDeviceImpl::Destroy(DropDeviceFromDeviceLostEvent dropDevice) {
  // Ready the DeviceLostEvent now. dropDevice=Yes ensures we can't later
  // try to pass an invalid device pointer to the callback.
  GetEventManager().SetFutureReady<DeviceLostEvent>(
      mDeviceLostFutureId, dropDevice, WGPUDeviceLostReason_Destroyed,
      "Device was destroyed.");

  emwgpuDeviceDestroy(this);
}

WGPUQueue WGPUDeviceImpl::GetQueue() const {
  auto queue = mQueue;
  return ReturnToAPI(std::move(queue));
}

WGPUFuture WGPUDeviceImpl::GetLostFuture() const {
  if (IsImported()) {
    DEBUG_PRINTF("GetLostFuture cannot be called on an imported device.\n");
    assert(false);
  }
  assert(mDeviceLostFutureId != kNullFutureId);
  return WGPUFuture{mDeviceLostFutureId};
}

void WGPUDeviceImpl::OnUncapturedError(WGPUErrorType type,
                                       char const* message) {
  if (mUncapturedErrorCallbackInfo.callback) {
    WGPUDeviceImpl* device = this;
    mUncapturedErrorCallbackInfo.callback(
        &device, type,
        WGPUStringView{.data = message, .length = std::strlen(message)},
        mUncapturedErrorCallbackInfo.userdata1,
        mUncapturedErrorCallbackInfo.userdata2);
  }
}

void WGPUDeviceImpl::WillDropLastExternalRef() {
  if (IsImported()) {
    // Note Destroy is responsible for readying the DeviceLostEvent. If the
    // device was imported, that doesn't happen, which is fine because no
    // callback can have been set.
    assert(mDeviceLostFutureId == kNullFutureId);
  } else {
    Destroy(DropDeviceFromDeviceLostEvent::Yes);
  }
}

// ----------------------------------------------------------------------------
// WGPUInstanceImpl implementations.
// ----------------------------------------------------------------------------

Ref<WGPUInstanceImpl> WGPUInstanceImpl::Create(
    const WGPUInstanceDescriptor* desc) {
  auto instance = AcquireRef(new WGPUInstanceImpl());

  // Validate and apply the descriptor
  if (desc) {
    // Features
    auto features =
        std::span(desc->requiredFeatures, desc->requiredFeatureCount);
    for (auto feature : features) {
      switch (feature) {
        case WGPUInstanceFeatureName_TimedWaitAny:
          if (!emscripten_has_asyncify()) {
            DEBUG_PRINTF(
                "timedWaitAnyEnable requested, but requires Asyncify or JSPI "
                "which is not available.\n");
            return {};
          }
          if (desc->requiredLimits) {
            if (desc->requiredLimits->timedWaitAnyMaxCount >
                kTimedWaitAnyMaxCountMax) {
              DEBUG_PRINTF("timedWaitAnyMaxCount %zu not supported (max %zu)\n",
                           desc->requiredLimits->timedWaitAnyMaxCount,
                           kTimedWaitAnyMaxCountMax);
              return {};
            }
            instance->mTimedWaitAnyMaxCount =
                desc->requiredLimits->timedWaitAnyMaxCount;
          }
          if (instance->mTimedWaitAnyMaxCount < kTimedWaitAnyMaxCountDefault) {
            instance->mTimedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
          }
          continue;
        case WGPUInstanceFeatureName_MultipleDevicesPerAdapter:
          DEBUG_PRINTF(
              "MultipleDevicesPerAdapter requested, but not supported in "
              "Wasm.\n");
          return {};
        case WGPUInstanceFeatureName_ShaderSourceSPIRV:
          DEBUG_PRINTF(
              "ShaderSourceSPIRV requested, but not supported in Wasm.\n");
          return {};
        case WGPUInstanceFeatureName_Force32:
          return {};
      }
      // Reject any unrecognized feature name.
      return {};
    }

    // Limits
    if (desc->requiredLimits && desc->requiredLimits->nextInChain != nullptr) {
      DEBUG_PRINTF("No WGPULimits extensions are supported.\n");
      return {};
    }
  }

  return instance;
}

WGPUInstanceImpl::WGPUInstanceImpl() : EventSource(GetNextInstanceId()) {
  GetEventManager().RegisterInstance(GetInstanceId());
}

WGPUInstanceImpl::~WGPUInstanceImpl() {
  GetEventManager().UnregisterInstance(GetInstanceId());
}

void WGPUInstanceImpl::ProcessEvents() {
  GetEventManager().ProcessEvents(GetInstanceId());
}

WGPUWaitStatus WGPUInstanceImpl::WaitAny(size_t count,
                                         WGPUFutureWaitInfo* infos,
                                         uint64_t timeoutNS) {
  if (timeoutNS > 0) {
    if (mTimedWaitAnyMaxCount == 0) {
      // Timed wait not valid unless enabled on the instance.
      DEBUG_PRINTF(
          "WaitAny timeoutNS (%" PRIu64
          ") > 0, but TimedWaitAny not enabled at wgpuCreateInstance\n",
          timeoutNS);
      return WGPUWaitStatus_Error;
    }
    // Cannot handle timeouts without Asyncify. Assert should never fail,
    // because wgpuCreateInstance should disallow timedWaitAnyEnable.
    // TODO(crbug.com/377760848): Use the preprocessor to remove all this code
    // (and emwgpuWaitAny) when building without Asyncify.
    assert(emscripten_has_asyncify());

    if (count > mTimedWaitAnyMaxCount) {
      DEBUG_PRINTF(
          "WaitAny count (%zu) > the timedWaitAnyMaxCount (%zu) which was "
          "enabled at wgpuCreateInstance\n",
          count, mTimedWaitAnyMaxCount);
      return WGPUWaitStatus_Error;
    }
  }

  return GetEventManager().WaitAny(GetInstanceId(), count, infos, timeoutNS);
}

InstanceID WGPUInstanceImpl::GetNextInstanceId() {
  static std::atomic<InstanceID> kNextInstanceId = 1;
  return kNextInstanceId++;
}

// ----------------------------------------------------------------------------
// WGPUQueueImpl implementations.
// ----------------------------------------------------------------------------

WGPUQueueImpl::WGPUQueueImpl(const EventSource* source) : EventSource(source) {}

// ----------------------------------------------------------------------------
// WGPUShaderModuleImpl implementations.
// ----------------------------------------------------------------------------

WGPUShaderModuleImpl::WGPUShaderModuleImpl(const EventSource* source)
    : EventSource(source) {}

WGPUFuture WGPUShaderModuleImpl::GetCompilationInfo(
    WGPUCompilationInfoCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<CompilationInfoEvent>(
          GetInstanceId(), this, callbackInfo));

  // If we already have the compilation info cached, we don't need to call into
  // JS.
  if (mCompilationInfo) {
    emwgpuOnCompilationInfoCompleted(double(futureId),
                                     WGPUCompilationInfoRequestStatus_Success,
                                     mCompilationInfo.get());
  } else {
    WGPUCompilationInfo* compilationInfo = new WGPUCompilationInfo{
        .nextInChain = nullptr, .messageCount = 0, .messages = nullptr};
    emwgpuShaderModuleGetCompilationInfo(this, futureId, compilationInfo);
  }
  return WGPUFuture{futureId};
}

// ----------------------------------------------------------------------------
// WebGPU function definitions, with methods organized by "class". Note these
// don't need to be extern "C" because they are already declared in webgpu.h.
//
// Also note that the full set of functions declared in webgpu.h are only
// partially implemeted here. The remaining ones are implemented via
// library_webgpu.js.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Common APIs are batch generated via X macros for all objects, including:
//   - AddRef
//   - Release
//   - SetLabel
// ----------------------------------------------------------------------------

#define DEFINE_WGPU_DEFAULT_ADDREF_RELEASE(Name) \
  void wgpu##Name##AddRef(WGPU##Name o) {        \
    o->AddRef();                                 \
  }                                              \
  void wgpu##Name##Release(WGPU##Name o) {       \
    if (o->Release()) {                          \
      delete o;                                  \
      emwgpuDelete(o);                           \
    }                                            \
  }
WGPU_OBJECTS(DEFINE_WGPU_DEFAULT_ADDREF_RELEASE)

#define DEFINE_WGPU_DEFAULT_SETLABEL(Name)                        \
  void wgpu##Name##SetLabel(WGPU##Name o, WGPUStringView label) { \
    emwgpuSetLabel(o, label.data, label.length);                  \
  }
WGPU_OBJECTS(DEFINE_WGPU_DEFAULT_SETLABEL)

// ----------------------------------------------------------------------------
// FreeMember functions
// ----------------------------------------------------------------------------

void wgpuAdapterInfoFreeMembers(WGPUAdapterInfo value) {
  // The strings are allocated via a single malloc, so freeing the first pointer
  // frees all of the strings in the struct.
  free(const_cast<char*>(value.vendor.data));
}

void wgpuSupportedFeaturesFreeMembers(WGPUSupportedFeatures value) {
  free(const_cast<WGPUFeatureName*>(value.features));
}

void wgpuSupportedWGSLLanguageFeaturesFreeMembers(
    WGPUSupportedWGSLLanguageFeatures value) {
  free(const_cast<WGPUWGSLLanguageFeatureName*>(value.features));
}

void wgpuSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities) {
  // wgpuSurfaceCapabilities doesn't currently allocate anything.
}

void wgpuSupportedInstanceFeaturesFreeMembers(WGPUSupportedInstanceFeatures) {
  // Nothing to do, .features is statically allocated.
}

// ----------------------------------------------------------------------------
// Standalone (non-method) functions
// ----------------------------------------------------------------------------

namespace {

const std::span<WGPUInstanceFeatureName> GetSupportedFeatures() {
  static std::optional<std::vector<WGPUInstanceFeatureName>> sSupportedFeatures;
  if (!sSupportedFeatures) {
    sSupportedFeatures = std::vector<WGPUInstanceFeatureName>();
    if (emscripten_has_asyncify()) {
      sSupportedFeatures->push_back(WGPUInstanceFeatureName_TimedWaitAny);
    }
  }
  return *sSupportedFeatures;
}

}  // namespace

WGPUBool wgpuHasInstanceFeature(WGPUInstanceFeatureName feature) {
  auto supportedFeatures = GetSupportedFeatures();
  return std::find(supportedFeatures.begin(), supportedFeatures.end(),
                   feature) != supportedFeatures.end();
}

void wgpuGetInstanceFeatures(WGPUSupportedInstanceFeatures* features) {
  assert(features != nullptr);

  auto supportedFeatures = GetSupportedFeatures();
  features->featureCount = supportedFeatures.size();
  features->features = supportedFeatures.data();
}

WGPUStatus wgpuGetInstanceLimits(WGPUInstanceLimits* limits) {
  assert(limits != nullptr);
  if (limits->nextInChain != nullptr) {
    DEBUG_PRINTF("No WGPULimits extensions are supported.\n");
    return WGPUStatus_Error;
  }

  limits->timedWaitAnyMaxCount = kTimedWaitAnyMaxCountMax;
  return WGPUStatus_Success;
}

WGPUInstance wgpuCreateInstance(
    [[maybe_unused]] const WGPUInstanceDescriptor* descriptor) {
  return ReturnToAPI(WGPUInstanceImpl::Create(descriptor));
}

// ----------------------------------------------------------------------------
// Methods of Adapter
// ----------------------------------------------------------------------------

WGPUFuture wgpuAdapterRequestDevice(
    WGPUAdapter adapter,
    const WGPUDeviceDescriptor* descriptor,
    WGPURequestDeviceCallbackInfo callbackInfo) {
  static const WGPUDeviceDescriptor kDefaultDescriptor =
      WGPU_DEVICE_DESCRIPTOR_INIT;
  if (descriptor == nullptr) {
    descriptor = &kDefaultDescriptor;
  }

  if (!ValidateCallbackMode(callbackInfo) ||
      !ValidateCallbackMode(descriptor->deviceLostCallbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<RequestDeviceEvent>(
          adapter->GetInstanceId(), callbackInfo));

  // For RequestDevice, we always create a Device and Queue up front. The
  // Device is also immediately associated with the DeviceLostEvent.
  WGPUQueue queue = new WGPUQueueImpl(adapter);
  WGPUDevice device = new WGPUDeviceImpl(adapter, *descriptor, queue);
  auto deviceLostFutureId = device->GetLostFuture().id;

  emwgpuAdapterRequestDevice(adapter, futureId, deviceLostFutureId, device,
                             queue, descriptor);
  return WGPUFuture{futureId};
}

// ----------------------------------------------------------------------------
// Methods of BindGroup
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of BindGroupLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Buffer
// ----------------------------------------------------------------------------

void wgpuBufferDestroy(WGPUBuffer buffer) {
  buffer->Destroy();
}

const void* wgpuBufferGetConstMappedRange(WGPUBuffer buffer,
                                          size_t offset,
                                          size_t size) {
  return buffer->GetConstMappedRange(offset, size);
}

WGPUBufferMapState wgpuBufferGetMapState(WGPUBuffer buffer) {
  return buffer->GetMapState();
}

void* wgpuBufferGetMappedRange(WGPUBuffer buffer, size_t offset, size_t size) {
  return buffer->GetMappedRange(offset, size);
}

WGPUStatus wgpuBufferWriteMappedRange(WGPUBuffer buffer,
                                      size_t offset,
                                      void const* data,
                                      size_t size) {
  return buffer->WriteMappedRange(offset, data, size);
}

WGPUStatus wgpuBufferReadMappedRange(WGPUBuffer buffer,
                                     size_t offset,
                                     void* data,
                                     size_t size) {
  return buffer->ReadMappedRange(offset, data, size);
}

WGPUFuture wgpuBufferMapAsync(WGPUBuffer buffer,
                              WGPUMapMode mode,
                              size_t offset,
                              size_t size,
                              WGPUBufferMapCallbackInfo callbackInfo) {
  return buffer->MapAsync(mode, offset, size, callbackInfo);
}

void wgpuBufferUnmap(WGPUBuffer buffer) {
  buffer->Unmap();
}

// ----------------------------------------------------------------------------
// Methods of CommandBuffer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of CommandEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Device
// ----------------------------------------------------------------------------

WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device,
                                  const WGPUBufferDescriptor* descriptor) {
  Ref<WGPUBufferImpl> buffer =
      AcquireRef(new WGPUBufferImpl(device, descriptor->mappedAtCreation));
  if (!emwgpuDeviceCreateBuffer(device, descriptor, buffer.Get())) {
    return nullptr;
  }
  return ReturnToAPI(std::move(buffer));
}

WGPUFuture wgpuDeviceCreateComputePipelineAsync(
    WGPUDevice device,
    const WGPUComputePipelineDescriptor* descriptor,
    WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<CreateComputePipelineEvent>(
          device->GetInstanceId(), callbackInfo));

  WGPUComputePipeline pipeline = emwgpuCreateComputePipeline(device);
  emwgpuDeviceCreateComputePipelineAsync(device, futureId, descriptor,
                                         pipeline);
  return WGPUFuture{futureId};
}

WGPUFuture wgpuDeviceCreateRenderPipelineAsync(
    WGPUDevice device,
    const WGPURenderPipelineDescriptor* descriptor,
    WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<CreateRenderPipelineEvent>(
          device->GetInstanceId(), callbackInfo));

  WGPURenderPipeline pipeline = emwgpuCreateRenderPipeline(device);
  emwgpuDeviceCreateRenderPipelineAsync(device, futureId, descriptor, pipeline);
  return WGPUFuture{futureId};
}

WGPUShaderModule wgpuDeviceCreateShaderModule(
    WGPUDevice device,
    const WGPUShaderModuleDescriptor* descriptor) {
  WGPUShaderModule shader = new WGPUShaderModuleImpl(device);
  emwgpuDeviceCreateShaderModule(device, descriptor, shader);
  return shader;
}

void wgpuDeviceDestroy(WGPUDevice device) {
  device->Destroy(DropDeviceFromDeviceLostEvent::No);
}

WGPUFuture wgpuDeviceGetLostFuture(WGPUDevice device) {
  return device->GetLostFuture();
}

WGPUQueue wgpuDeviceGetQueue(WGPUDevice device) {
  return device->GetQueue();
}

WGPUFuture wgpuDevicePopErrorScope(WGPUDevice device,
                                   WGPUPopErrorScopeCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<PopErrorScopeEvent>(
          device->GetInstanceId(), callbackInfo));

  emwgpuDevicePopErrorScope(device, futureId);
  return WGPUFuture{futureId};
}

// ----------------------------------------------------------------------------
// Methods of Instance
// ----------------------------------------------------------------------------

void wgpuInstanceProcessEvents(WGPUInstance instance) {
  instance->ProcessEvents();
}

WGPUFuture wgpuInstanceRequestAdapter(
    WGPUInstance instance,
    WGPURequestAdapterOptions const* options,
    WGPURequestAdapterCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId =
      GetEventManager().TrackEvent(std::make_unique<RequestAdapterEvent>(
          instance->GetInstanceId(), callbackInfo));

  WGPUAdapter adapter = emwgpuCreateAdapter(instance);
  emwgpuInstanceRequestAdapter(instance, futureId, options, adapter);
  return WGPUFuture{futureId};
}

WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance instance,
                                   size_t futureCount,
                                   WGPUFutureWaitInfo* futures,
                                   uint64_t timeoutNS) {
  return instance->WaitAny(futureCount, futures, timeoutNS);
}

// ----------------------------------------------------------------------------
// Methods of PipelineLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of QuerySet
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Queue
// ----------------------------------------------------------------------------

WGPUFuture wgpuQueueOnSubmittedWorkDone(
    WGPUQueue queue,
    WGPUQueueWorkDoneCallbackInfo callbackInfo) {
  if (!ValidateCallbackMode(callbackInfo)) {
    return WGPUFuture{kNullFutureId};
  }
  FutureID futureId = GetEventManager().TrackEvent(
      std::make_unique<WorkDoneEvent>(queue->GetInstanceId(), callbackInfo));

  emwgpuQueueOnSubmittedWorkDone(queue, futureId);
  return WGPUFuture{futureId};
}

// ----------------------------------------------------------------------------
// Methods of RenderBundle
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderBundleEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Sampler
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ShaderModule
// ----------------------------------------------------------------------------

WGPUFuture wgpuShaderModuleGetCompilationInfo(
    WGPUShaderModule shader,
    WGPUCompilationInfoCallbackInfo callbackInfo) {
  return shader->GetCompilationInfo(callbackInfo);
}

// ----------------------------------------------------------------------------
// Methods of Surface
// ----------------------------------------------------------------------------

WGPUStatus wgpuSurfaceGetCapabilities(WGPUSurface,
                                      WGPUAdapter,
                                      WGPUSurfaceCapabilities* capabilities) {
  assert(capabilities->nextInChain ==
         nullptr);  // TODO: Return WGPUStatus_Error

  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsRGBAFirst = {
      WGPUTextureFormat_RGBA8Unorm,
      WGPUTextureFormat_BGRA8Unorm,
      WGPUTextureFormat_RGBA16Float,
  };
  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsBGRAFirst = {
      WGPUTextureFormat_BGRA8Unorm,
      WGPUTextureFormat_RGBA8Unorm,
      WGPUTextureFormat_RGBA16Float,
  };
  WGPUTextureFormat preferredFormat = emwgpuGetPreferredFormat();
  switch (preferredFormat) {
    case WGPUTextureFormat_RGBA8Unorm:
      capabilities->formatCount = kSurfaceFormatsRGBAFirst.size();
      capabilities->formats = kSurfaceFormatsRGBAFirst.data();
      break;
    case WGPUTextureFormat_BGRA8Unorm:
      capabilities->formatCount = kSurfaceFormatsBGRAFirst.size();
      capabilities->formats = kSurfaceFormatsBGRAFirst.data();
      break;
    default:
      assert(false);
      return WGPUStatus_Error;
  }

  {
    static constexpr WGPUPresentMode kPresentMode = WGPUPresentMode_Fifo;
    capabilities->presentModeCount = 1;
    capabilities->presentModes = &kPresentMode;
  }

  {
    static constexpr std::array<WGPUCompositeAlphaMode, 2> kAlphaModes = {
        WGPUCompositeAlphaMode_Opaque,
        WGPUCompositeAlphaMode_Premultiplied,
    };
    capabilities->alphaModeCount = kAlphaModes.size();
    capabilities->alphaModes = kAlphaModes.data();
  }

  return WGPUStatus_Success;
}

// ----------------------------------------------------------------------------
// Methods of Texture
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of TextureView
// ----------------------------------------------------------------------------
