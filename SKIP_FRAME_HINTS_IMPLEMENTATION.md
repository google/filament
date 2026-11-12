# Implementation Specification: `Renderer::skipFrame` Intent Hints

This document specifies the implementation steps for `Renderer::SkipFrameHint` and `backend::SkipFrameFlags`. Follow the steps in order. Adhere to the repository rules in [AGENTS.md](file:///Users/powei/work/tree/android-frame-skip/AGENTS.md) and [CODE_STYLE.md](file:///Users/powei/work/tree/android-frame-skip/CODE_STYLE.md).

---

## 1. Architectural Invariants

1. **Public vs. private boundary**: Define `Renderer::SkipFrameHint` in [Renderer.h](file:///Users/powei/work/tree/android-frame-skip/filament/include/filament/Renderer.h). Define `backend::SkipFrameFlags` in [Driver.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/Driver.h). Do not place `SkipFrameFlags` in [DriverEnums.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/backend/DriverEnums.h), because `include/backend/` is public to applications.
2. **Always-on housekeeping**: Every `skipFrame` call must still poll fences, retire completed command buffers, reset empty staging buffer offsets for reuse, free explicitly destroyed client handles, update `FrameInfoManager` user history, and run `FEngine::gc()`.
3. **Command-stream ordering**: In `FRenderer::skipFrame()`, execute `mResourceAllocator->gc(...)` before calling `driver.skipFrame(flags)`. This order ensures that `driver.destroyTexture()` commands enter the command stream before the backend driver runs handle garbage collection.
4. **No cache eviction inside `Driver::tick()`**: Keep `DriverApi::tick()` for `FRenderer::beginFrame()`, `FRenderer::endFrame()`, and timer query tests, because `beginFrame()` returning `false` does not call `driver.skipFrame()`. Remove all cache aging and cache eviction from `VulkanDriver::tick()`. Remove the separate `driver.tick()` call from `FRenderer::skipFrame()` and run per-tick polling inside each backend's `Driver::skipFrame()` implementation instead.
5. **Drain retired Vulkan handles before pool eviction**: Dropping a `fvkmemory::resource_ptr` only queues destruction onto `ResourceManager::mGcList`; destructors run inside `ResourceManager::gc()`. Backend pool reclamation (`VulkanStagePool::reclaimCompleted()`, `VulkanStagePool::evictAll()`, `VulkanBufferCache::evictAll()`) therefore requires retired `VulkanStage::Segment`, `VulkanStageImage::Resource`, `VulkanBuffer`, and `VulkanFramebuffer` handles to be destroyed first.

---

## 2. Core Implementation (This PR)

### Step 1: Update Public C++ `Renderer` API

Modify [Renderer.h](file:///Users/powei/work/tree/android-frame-skip/filament/include/filament/Renderer.h) and [Renderer.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/Renderer.cpp):

1. Add `enum class SkipFrameHint : uint8_t` inside `filament::Renderer` above `skipFrame()`:
   ```cpp
   /**
    * Hints informing Filament why a frame is being skipped so it can adjust cache eviction and
    * asynchronous background work accordingly.
    *
    * @see skipFrame()
    */
   enum class SkipFrameHint : uint8_t {
       /**
        * The client is temporarily idle (for example, when scene content has not changed).
        * Transient caches age out gradually over consecutive skipped frames, and queued
        * asynchronous operations continue to run.
        */
       IDLE = 0,

       /**
        * The frame is skipped to maintain a target frame rate cadence below the display refresh
        * rate. Transient caches are kept warm for the next rendered frame, and queued
        * asynchronous operations continue to run.
        */
       CADENCE = 1,

       /**
        * The frame is skipped because the GPU has fallen behind. Transient caches are kept warm
        * for the next rendered frame, and amortized asynchronous operations are deferred to avoid
        * adding GPU load.
        */
       GPU_CATCH_UP = 2,

       /**
        * The client is stopping rendering and wants unused transient caches (such as FrameGraph
        * textures and staging buffers) evicted immediately without blocking.
        *
        * Because command execution is asynchronous and in-flight GPU frames retain references to
        * transient resources until completion, a single non-blocking call may not free resources
        * used by frames still executing on the GPU. To synchronously reclaim all transient memory
        * before returning (for example, when pausing rendering or handling a system memory trim
        * callback), drain the GPU before and after calling skipFrame():
        *
        * ~~~~~~~~~~~{.cpp}
        * engine->flushAndWait();
        * renderer->skipFrame(0, Renderer::SkipFrameHint::RECLAIM_MEMORY);
        * engine->flushAndWait();
        * ~~~~~~~~~~~
        */
       RECLAIM_MEMORY = 3,
   };
   ```
2. Update the `Renderer::skipFrame` declaration in [Renderer.h](file:///Users/powei/work/tree/android-frame-skip/filament/include/filament/Renderer.h#L304):
   ```cpp
   void skipFrame(uint64_t vsyncSteadyClockTimeNano = 0u,
           SkipFrameHint hint = SkipFrameHint::IDLE);
   ```
3. Update the trampoline in [Renderer.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/Renderer.cpp#L74-L76) to forward `hint` to `FRenderer::skipFrame(vsyncSteadyClockTimeNano, hint)`.

---

### Step 2: Add Private Backend `SkipFrameFlags` and Update `DriverAPI.inc`

Modify [Driver.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/Driver.h) and [DriverAPI.inc](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/DriverAPI.inc):

1. Include `<utils/BitmaskEnum.h>` and `<type_traits>` in [Driver.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/Driver.h) (note that `utils::EnableBitMaskOperators` is defined in [BitmaskEnum.h](file:///Users/powei/work/tree/android-frame-skip/libs/utils/include/utils/BitmaskEnum.h), not `bitset.h`).
2. Define `SkipFrameFlags` in `namespace filament::backend` before `class Driver` in [Driver.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/Driver.h), close `namespace filament::backend` to specialize `utils::EnableBitMaskOperators` at global scope, and reopen `namespace filament::backend` for `class Driver`:
   ```cpp
   namespace filament::backend {

   enum class SkipFrameFlags : uint8_t {
       NONE             = 0,
       KEEP_CACHES      = 1u << 0,
       EVICT_CACHES     = 1u << 1,
       DEFER_ASYNC_WORK = 1u << 2,
   };

   } // namespace filament::backend

   template<>
   struct utils::EnableBitMaskOperators<filament::backend::SkipFrameFlags>
           : public std::true_type {};

   namespace filament::backend {
   ```
3. Replace `DECL_DRIVER_API_0(skipFrame)` in [DriverAPI.inc](file:///Users/powei/work/tree/android-frame-skip/filament/backend/include/private/backend/DriverAPI.inc#L170) with:
   ```cpp
   DECL_DRIVER_API_N(skipFrame,
           backend::SkipFrameFlags, flags)
   ```
   Note that `DECL_DRIVER_API_N` does not permit default parameter values in `DriverAPI.inc`. Update all direct `DriverApi::skipFrame` call sites accordingly.

---

### Step 3: Refactor Frontend `TextureCache`

Modify [TextureCache.h](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.h) and [TextureCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.cpp):

1. Add a `CachePolicy` enum to `TextureCache` in [TextureCache.h](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.h):
   ```cpp
   enum class CachePolicy : uint8_t {
       AGE,       // Advance mAge and evict entries past mCacheMaxAge (rate-limited) or unique-age limit.
       KEEP,      // Do not advance mAge and do not evict any entries.
       EVICT_ALL, // Immediately purge all entries currently in mTextureCache.
   };

   void gc(CachePolicy policy = CachePolicy::AGE) noexcept;
   ```
2. Update `EvictionReason` in [TextureCache.h](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.h#L88-L92) by replacing `SKIPPED_FRAME` with `FORCED_EVICTION` (and update the debug log switch in [TextureCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.cpp#L198-L208)).
3. Implement `TextureCache::gc(CachePolicy policy)` in [TextureCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/TextureCache.cpp#L253-L327):
   * If `policy == CachePolicy::KEEP`, return immediately without modifying `mAge` or evicting textures.
   * If `policy == CachePolicy::EVICT_ALL`, iterate over `mTextureCache` and call `purge(it, EvictionReason::FORCED_EVICTION)` on every entry until `mTextureCache` is empty, age the debugger if enabled, and return.
   * If `policy == CachePolicy::AGE`, execute the standard per-frame logic: capture `const size_t age = mAge`, increment `mAge++`, evict up to `MAX_EVICTION_COUNT` (1) entries where `age - entry.age >= mCacheMaxAge` with `EvictionReason::AGED_OUT`, and evict entries exceeding `MAX_UNIQUE_AGE_COUNT` (3) with `EvictionReason::UNIQUE_AGE_LIMIT`.

---

### Step 4: Implement `FRenderer::skipFrame` Translation and Ordering

Modify [details/Renderer.h](file:///Users/powei/work/tree/android-frame-skip/filament/src/details/Renderer.h#L124) and [details/Renderer.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/src/details/Renderer.cpp#L358-L391):

1. Update `FRenderer::skipFrame` signature to `void skipFrame(uint64_t vsyncSteadyClockTimeNano, SkipFrameHint hint)`.
2. Map `SkipFrameHint` to `TextureCache::CachePolicy` and `backend::SkipFrameFlags`:
   * `SkipFrameHint::IDLE` maps to `CachePolicy::AGE` and `SkipFrameFlags::NONE`.
   * `SkipFrameHint::CADENCE` maps to `CachePolicy::KEEP` and `SkipFrameFlags::KEEP_CACHES`.
   * `SkipFrameHint::GPU_CATCH_UP` maps to `CachePolicy::KEEP` and `SkipFrameFlags::KEEP_CACHES | SkipFrameFlags::DEFER_ASYNC_WORK`.
   * `SkipFrameHint::RECLAIM_MEMORY` maps to `CachePolicy::EVICT_ALL` and `SkipFrameFlags::EVICT_CACHES`.
3. Remove `driver.tick()` from `FRenderer::skipFrame()`.
4. Call `mResourceAllocator->gc(cachePolicy)` **before** `driver.skipFrame(skipFlags)`, and add a comment documenting why `destroyTexture` commands must precede `driver.skipFrame(skipFlags)` on the command stream.
5. Keep `engine.flush()`, `engine.gc()`, and `mFrameSkipper.frameSkipped()` after `driver.skipFrame(skipFlags)`.

---

### Step 5: Update Vulkan Backend Caches and `VulkanDriver`

1. **`ResourceManager`** ([ResourceManager.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/memory/ResourceManager.h), [ResourceManager.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/memory/ResourceManager.cpp#L59-L63)):
   * Add `void drain() noexcept` looping `while (!mThreadSafeGcList.empty() || !mGcList.empty()) { gc(); }` (and reuse it in `terminate()`). This allows `skipFrame(EVICT_CACHES)` to release multi-hop chains such as `VulkanFramebuffer -> VulkanRenderTarget -> VulkanTexture -> VulkanTextureState` in one pass.
2. **`VulkanStagePool`** ([VulkanStagePool.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanStagePool.h), [VulkanStagePool.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanStagePool.cpp#L266-L320)):
   * Add `void reclaimCompleted() noexcept`: iterate `mStages` and reset (`stage->reset()`) any stage where `stage->isSafeToReset()` is true, reinserting it with full `stage->capacity()`. Do not destroy stages, do not increment `mCurrentFrame`, and do not evict `mFreeImages`.
   * Add `void evictAll() noexcept`: for `mStages`, destroy every stage where `stage->isSafeToReset()` is true (retaining `0` empty stages) and keep in-use stages in `mStages`. Destroy and clear all entries in `mFreeImages`.
   * Keep `void gc() noexcept` for normal TTL-based aging (`++mCurrentFrame`, retaining up to `MAX_EMPTY_STAGES_TO_RETAIN` empty stages, and evicting `mFreeImages` older than `TIME_BEFORE_EVICTION`).
3. **`VulkanBufferCache`** ([VulkanBufferCache.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanBufferCache.h), [VulkanBufferCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanBufferCache.cpp#L80-L120)):
   * Add `void evictAll() noexcept` destroying all pooled unused `VulkanGpuBuffer` entries across `mGpuBufferPools` and clearing each pool (reused by `terminate()`).
   * Keep `void gc() noexcept` for normal TTL-based eviction.
4. **`VulkanFboCache`** ([VulkanFboCache.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanFboCache.h), [VulkanFboCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanFboCache.cpp#L369-L424)):
   * Add `void evictAll() noexcept`: call `resetFramebuffers()` to clear `mFramebufferCache` (decrementing `mRenderPassRefCount` for each entry) and erase all `VkRenderPass` entries whose `mRenderPassRefCount` is `0`.
5. **`VulkanDriver::tick(int)`** ([VulkanDriver.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanDriver.cpp#L455-L469)):
   * Keep `mCommands.updateFences()` and `mReadPixels.gc()`.
   * Replace the `collectGarbage()` call inside the `++mTicksSinceLastGc == MAX_TICKS_BETWEEN_GC` block with non-evicting cleanup only: call `flush()`, `mCommands.gc()`, `mResourceManager.gc()`, and `mStagePool.reclaimCompleted()` (in that order so retired `Segment`s are destroyed before `reclaimCompleted()` checks `isSafeToReset()`), and reset `mTicksSinceLastGc = 0`. Do not call `mStagePool.gc()`, `mBufferCache.gc()`, `mFramebufferCache.gc()`, or `mPipelineCache.gc()` from `tick()`.
6. **`VulkanDriver::skipFrame(SkipFrameFlags flags)`** ([VulkanDriver.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/vulkan/VulkanDriver.cpp#L540-L544)):
   * Assert `!(any(flags & SkipFrameFlags::KEEP_CACHES) && any(flags & SkipFrameFlags::EVICT_CACHES))`.
   * Call `endCommandRecording()` so any pending transfer or transition commands recorded outside a frame are submitted.
   * Call `mCommands.updateFences()`, `mReadPixels.gc()`, `mCommands.gc()`, and `mDescriptorSetCache.gc()`. Reset `mTicksSinceLastGc = 0`.
   * Branch on cache flags:
     * If `any(flags & SkipFrameFlags::KEEP_CACHES)`: call `mResourceManager.gc()`, then `mStagePool.reclaimCompleted()`. Do not call `gc()` or `evictAll()` on `mStagePool`, `mBufferCache`, `mFramebufferCache`, or `mPipelineCache`.
     * Else if `any(flags & SkipFrameFlags::EVICT_CACHES)`: call `mFramebufferCache.evictAll()` first, then `mResourceManager.drain()` (so retired `Segment`, `StageImage`, `VulkanBuffer`, and `VulkanFramebuffer -> VulkanRenderTarget -> VulkanTexture` chains are destroyed and returned before pool eviction), then call `mStagePool.evictAll()`, `mBufferCache.evictAll()`, and `mPipelineCache.gc()`.
     * Else (`SkipFrameFlags::NONE`): call `mResourceManager.gc()`, then `mStagePool.gc()`, `mBufferCache.gc()`, `mFramebufferCache.gc()`, `mPipelineCache.gc()`, and `mResourceManager.gc()` again to free any framebuffers/render passes evicted during `mFramebufferCache.gc()`.
   * If `none(flags & SkipFrameFlags::DEFER_ASYNC_WORK)` and `getJobWorker()` is non-null, call `getJobWorker()->processFrame()`.

---

### Step 6: Update Metal, OpenGL, WebGPU, and Noop Backends

1. **`MetalBufferPool` and `MetalDriver`** ([MetalBufferPool.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/metal/MetalBufferPool.h), [MetalBufferPool.mm](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/metal/MetalBufferPool.mm#L88-L116), [MetalDriver.mm](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/metal/MetalDriver.mm#L412-L416)):
   * Add `void evictAll() noexcept` to `MetalBufferPool`: lock `mMutex`, delete all entries in `mFreeStages`, and clear `mFreeStages` without asserting `mUsedStages.empty()` (unlike `reset()`, in-flight stages may still reside in `mUsedStages`).
   * In `MetalDriver::skipFrame(SkipFrameFlags flags)`:
     * Call `tick(0)` and `submitPendingCommands(mContext)`.
     * If `any(flags & SkipFrameFlags::EVICT_CACHES)`: call `mContext->bufferPool->evictAll()` and `CVMetalTextureCacheFlush(mContext->textureCache, 0)`.
     * Else if `none(flags & SkipFrameFlags::KEEP_CACHES)`: call `mContext->bufferPool->gc()` and `CVMetalTextureCacheFlush(mContext->textureCache, 0)`.
     * If `none(flags & SkipFrameFlags::DEFER_ASYNC_WORK)` and `getJobWorker()` is non-null, call `getJobWorker()->processFrame()`.
2. **`OpenGLDriver`** ([OpenGLDriver.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/opengl/OpenGLDriver.cpp#L4630-L4634)):
   * Update signature to `void OpenGLDriver::skipFrame(SkipFrameFlags flags)`.
   * Call `tick(0)`.
   * If `none(flags & SkipFrameFlags::DEFER_ASYNC_WORK)` and `getJobWorker()` is non-null, call `getJobWorker()->processFrame()`.
3. **`WebGPUStagePool` and `WebGPUDriver`** ([WebGPUStagePool.h](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/webgpu/WebGPUStagePool.h), [WebGPUStagePool.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/webgpu/WebGPUStagePool.cpp#L74-L87), [WebGPUDriver.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/webgpu/WebGPUDriver.cpp#L232-L236)):
   * Add `void evictAll()` to `WebGPUStagePool`: iterate `mInProgress` directly and call `buffer.Destroy()` on every entry with `st->getStatus() == FenceStatus::CONDITION_SATISFIED` (retaining incomplete entries in `mInProgress`; do not call `gc()`, because `recycleBuffer()` triggers async `MapAsync` callbacks that later reinsert buffers into `mBuffers`). Then lock `mMutex`, call `buffer.Destroy()` on every buffer in `mBuffers`, and clear `mBuffers`.
   * In `WebGPUDriver::skipFrame(SkipFrameFlags flags)`:
     * Call `mQueueManager.flush()` and `tick(0)` (so Dawn processes work-done events before checking `mInProgress`).
     * If `any(flags & SkipFrameFlags::EVICT_CACHES)`, call `mStagePool.evictAll()`; otherwise call `mStagePool.gc()` (which only recycles completed `mInProgress` buffers into `mBuffers`).
     * If `none(flags & SkipFrameFlags::DEFER_ASYNC_WORK)` and `getJobWorker()` is non-null, call `getJobWorker()->processFrame()`.
4. **`NoopDriver`** ([NoopDriver.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/src/noop/NoopDriver.cpp#L100)):
   * Update signature to `void NoopDriver::skipFrame(SkipFrameFlags)`.

---

### Step 7: Regenerate Android Bindings via APIGen

Follow [skills/bindings_synchronization/SKILL.md](file:///Users/powei/work/tree/android-frame-skip/skills/bindings_synchronization/SKILL.md) and [tools/apigen/apigen.md](file:///Users/powei/work/tree/android-frame-skip/tools/apigen/apigen.md):

1. Do **not** hand-edit [Renderer.java](file:///Users/powei/work/tree/android-frame-skip/android/filament-android/src/main/java/com/google/android/filament/Renderer.java) or [android/filament-android/src/main/cpp/Renderer.cpp](file:///Users/powei/work/tree/android-frame-skip/android/filament-android/src/main/cpp/Renderer.cpp); both are generated from [Renderer.h](file:///Users/powei/work/tree/android-frame-skip/filament/include/filament/Renderer.h).
2. Run APIGen to regenerate and validate the Android bindings:
   ```bash
   python3 tools/apigen/generate_android.py -j --validate Renderer
   python3 tools/apigen/verify_bindings.py
   python3 -m unittest discover tools/apigen/tests
   ```
   Verify that `git diff android/` adds `Renderer.SkipFrameHint`, the `skipFrame(long, SkipFrameHint)` overload (and preserves `skipFrame()` and `skipFrame(long)` delegating to `SkipFrameHint.IDLE`), and updates `nSkipFrame` in both Java and JNI C++.

---

### Step 8: Synchronize Web / TypeScript Bindings

1. **`jsenums.cpp`** ([web/filament-js/jsenums.cpp](file:///Users/powei/work/tree/android-frame-skip/web/filament-js/jsenums.cpp)):
   * Include `<filament/Renderer.h>` and register `enum_<Renderer::SkipFrameHint>("Renderer$SkipFrameHint")` with values `IDLE`, `CADENCE`, `GPU_CATCH_UP`, and `RECLAIM_MEMORY`.
2. **`jsbindings.cpp`** ([web/filament-js/jsbindings.cpp](file:///Users/powei/work/tree/android-frame-skip/web/filament-js/jsbindings.cpp#L816)):
   * Replace `.function("skipFrame", &Renderer::skipFrame)` with arity-dispatched `EMBIND_LAMBDA` overloads (each with `, allow_raw_pointers()`) for `(Renderer* self)`, `(Renderer* self, uint64_t vsyncSteadyClockTimeNano)`, and `(Renderer* self, uint64_t vsyncSteadyClockTimeNano, Renderer::SkipFrameHint hint)`.
3. **`filament.d.ts`** ([web/filament-js/filament.d.ts](file:///Users/powei/work/tree/android-frame-skip/web/filament-js/filament.d.ts#L570)):
   * Declare `export enum Renderer$SkipFrameHint` and update `Renderer.skipFrame` to `public skipFrame(vsyncSteadyClockTimeNano?: number, hint?: Renderer$SkipFrameHint): void;`.

---

### Step 9: Update Tests and Run Verification Pipeline

1. **Frontend unit tests**:
   * Update [test_TextureCache.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/test/test_TextureCache.cpp#L69-L108) to replace `cache.gc(bool)` with `CachePolicy` tests covering:
     1. `CachePolicy::KEEP` preserves all cached textures across multiple skipped frames, including multi-frame entries (`ageDiff >= 2`).
     2. `CachePolicy::AGE` increments `mAge` and evicts even the most recently rendered frame's textures once `mCacheMaxAge` skipped frames elapse.
     3. `CachePolicy::EVICT_ALL` immediately purges every cached texture in a single call.
   * Add a `Renderer::skipFrame` test in [filament_test.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/test/filament_test.cpp#L3094-L3105) exercising `IDLE`, `CADENCE`, `GPU_CATCH_UP`, and `RECLAIM_MEMORY`.
2. **Backend unit tests**:
   * Update the four `api.skipFrame()` calls in [test_AsyncUploads.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/test/test_AsyncUploads.cpp#L111) to `api.skipFrame(SkipFrameFlags::NONE)`.
   * Add a test in [test_AsyncUploads.cpp](file:///Users/powei/work/tree/android-frame-skip/filament/backend/test/test_AsyncUploads.cpp) verifying that `api.skipFrame(SkipFrameFlags::DEFER_ASYNC_WORK)` does not execute queued amortization work, while a subsequent `api.skipFrame(SkipFrameFlags::NONE)` completes it. Because `BackendTest` configures `THREAD_PREFERRED` (`ThreadWorker`) on Metal and OpenGL, either gate the `BackendTest` test with `SKIP_IF_NOT(Backend::WEBGPU, ...)` or run against a driver explicitly configured with `Platform::AsynchronousMode::AMORTIZATION`.
3. **Mandatory verification pipeline** (per [skills/verification_protocols/SKILL.md](file:///Users/powei/work/tree/android-frame-skip/skills/verification_protocols/SKILL.md)):
   * Run `./tools/reorganize-headers/run.py` on modified C++ files and check header self-containment per [skills/header_self_containment/SKILL.md](file:///Users/powei/work/tree/android-frame-skip/skills/header_self_containment/SKILL.md) and [skills/cpp_header_inclusion/SKILL.md](file:///Users/powei/work/tree/android-frame-skip/skills/cpp_header_inclusion/SKILL.md).
   * Build desktop debug targets (`./build.sh -ip desktop debug`) and run `test_utils`, `test_filament` (including `TextureCacheTest.*`), and `backend_test` (including `AsyncUploads*`) per [skills/filament_desktop_testing/SKILL.md](file:///Users/powei/work/tree/android-frame-skip/skills/filament_desktop_testing/SKILL.md).

---

## 3. Follow-up PRs and Deferred Nice-to-Haves

The items below are intentionally separated from the core `skipFrame` implementation PR so this change stays focused and self-contained:

1. **Follow-up PR — Adopt `SkipFrameHint::GPU_CATCH_UP` in `FramePacer` callers and docs**:
   * Update the C++ code snippet in [FramePacer.h](file:///Users/powei/work/tree/android-frame-skip/filament/include/filament/FramePacer.h#L70-L73) to pass `filament::Renderer::SkipFrameHint::GPU_CATCH_UP`, and regenerate [FramePacer.java](file:///Users/powei/work/tree/android-frame-skip/android/filament-android/src/main/java/com/google/android/filament/FramePacer.java#L58-L61) with APIGen.
   * Update [ModelViewer.kt](file:///Users/powei/work/tree/android-frame-skip/android/filament-utils-android/src/main/java/com/google/android/filament/utils/ModelViewer.kt#L353-L406) to call `renderer.skipFrame(frameTimeNanos, Renderer.SkipFrameHint.GPU_CATCH_UP)` in the `Long` overload and `renderer.skipFrame(frameData.frameTimeNanos, Renderer.SkipFrameHint.GPU_CATCH_UP)` in the `Choreographer.FrameData` overload when `framePacer.hasGpuFallenBehind(renderer)` is true.
   * Update [MainActivity.kt](file:///Users/powei/work/tree/android-frame-skip/android/samples/sample-time-projection/src/main/java/com/google/android/filament/timeprojection/MainActivity.kt#L291-L294) to pass `Renderer.SkipFrameHint.GPU_CATCH_UP`.
2. **Follow-up PR — Dedicated backend cache-eviction unit tests**:
   * Add backend-level regression tests directly validating `RECLAIM_MEMORY` vs. `CADENCE`/`GPU_CATCH_UP` behavior on `VulkanStagePool`, `VulkanBufferCache`, `VulkanFboCache`, `MetalBufferPool`, and `WebGPUStagePool`.
3. **Deferred Nice-to-Haves (only if needed later)**:
   * `operator<<(utils::io::ostream&, SkipFrameFlags)` for `#if DEBUG_COMMAND_STREAM` builds (`DEBUG_COMMAND_STREAM` is disabled by default).
   * Flag-conflict `assert_invariant` checks in `MetalDriver`, `OpenGLDriver`, and `WebGPUDriver` (kept in `VulkanDriver::skipFrame()`).
   * Calling `mDescriptorSetCache.gc()` inside `VulkanDriver::tick()`.
