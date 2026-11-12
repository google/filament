# Design: `Renderer::skipFrame` Intent Hints

## 1. Summary

`Renderer::skipFrame()` currently applies one fixed garbage collection policy to every skipped frame. That single policy conflicts with the two primary reasons clients skip frames: letting an overloaded GPU catch up, and pausing rendering while reclaiming transient memory.

This design adds a `Renderer::SkipFrameHint` enum to the public client API and a private `backend::SkipFrameFlags` bitmask to the backend driver API. Clients specify *why* they skip a frame, and `FRenderer` translates that intent into cache and asynchronous worker actions across the frontend and backend.

## 2. Problem Statement

Today, calling `Renderer::skipFrame()` causes two opposite failures:

1. **Premature eviction when the GPU falls behind**: When a client skips a frame to let the GPU catch up, `TextureCache` immediately purges cached textures older than one frame, and `VulkanDriver` periodically evicts staging buffers, pooled GPU buffers, and framebuffers. The next rendered frame must reallocate those resources, which increases CPU and GPU load while the pipeline is already behind. In addition, `skipFrame()` still runs amortized asynchronous uploads on the driver thread, which submits extra GPU transfer work during recovery.
2. **Incomplete reclamation when the client is idle**: When `TextureCache::gc(true)` runs today, it does not advance its frame age counter. As a result, transient FrameGraph textures from the most recently rendered frame remain at age difference `1` and are never freed across consecutive `skipFrame()` calls. Meanwhile, Metal and WebGPU do not run staging pool garbage collection inside `skipFrame()` at all.

## 3. Public Client API

The public header `filament/include/filament/Renderer.h` exposes a four-value intent enum. Matching enums and overloads are added to the Android (`Renderer.java`) and Web (`filament.d.ts`) bindings.

```cpp
class UTILS_PUBLIC Renderer : public FilamentAPI {
public:
    enum class SkipFrameHint : uint8_t {
        IDLE           = 0, // Scene is temporarily static; age caches gradually and run async work.
        CADENCE        = 1, // Pacing below display refresh rate; keep caches warm and run async work.
        GPU_CATCH_UP   = 2, // GPU is behind; keep caches warm and defer amortized async work.
        RECLAIM_MEMORY = 3, // Client stopped rendering; evict unused transient caches immediately.
    };

    void skipFrame(uint64_t vsyncSteadyClockTimeNano = 0u,
            SkipFrameHint hint = SkipFrameHint::IDLE);
};
```

### Synchronous Memory Reclamation

`Renderer::skipFrame()` executes asynchronously on the command stream and does not block the caller. In-flight GPU command buffers also retain references to staging segments, pooled buffers, and transient textures until their GPU fences signal. Callers that stop their render loop and require immediate, synchronous reclamation (such as inside an OS memory trim callback) drain the engine before and after `skipFrame()`:

```cpp
engine->flushAndWait();
renderer->skipFrame(0, Renderer::SkipFrameHint::RECLAIM_MEMORY);
engine->flushAndWait();
```

## 4. Private Backend API and Subsystem Actions

The private backend header `private/backend/Driver.h` defines three behavioral flags (`KEEP_CACHES`, `EVICT_CACHES`, and `DEFER_ASYNC_WORK`) passed through `DriverApi::skipFrame(SkipFrameFlags)` in `private/backend/DriverAPI.inc`. Every `skipFrame()` call still performs mandatory non-evicting housekeeping: retiring signaled fences and command buffers, resetting free staging offsets for reuse, freeing explicitly destroyed client handles, and advancing `FEngine::gc()`.

| Hint | Frontend `TextureCache` | Backend Staging & Buffer Pools | Backend FBO Cache | Amortized Async Work |
| :--- | :--- | :--- | :--- | :--- |
| `IDLE` (default) | Increment age and evict expired entries gradually. | Increment age and evict idle entries past TTL. | Increment age and evict expired entries. | Run frame budget. |
| `CADENCE` | Keep all entries and freeze age. | Reset finished stages for reuse; do not age or destroy pools. | Keep all entries and freeze age. | Run frame budget. |
| `GPU_CATCH_UP` | Keep all entries and freeze age. | Reset finished stages for reuse; do not age or destroy pools. | Keep all entries and freeze age. | Defer for this frame. |
| `RECLAIM_MEMORY` | Evict all cached textures immediately. | Destroy all idle staging buffers, stage images, and pooled buffers. | Evict all unused entries immediately. | Run frame budget. |

## 5. Rationale

* **Two-layer separation**: Exposing a semantic enum on `Renderer` hides internal driver mechanisms from applications and prevents invalid flag combinations such as keeping and evicting caches simultaneously. Keeping behavioral flags in `private/backend/` lets backend drivers implement orthogonal resource rules without knowing about frontend frame pacing.
* **Command ordering for immediate eviction**: `FRenderer::skipFrame()` runs `TextureCache` eviction before enqueuing `driver.skipFrame(flags)`. This order ensures that `destroyTexture` commands reach the driver thread before backend garbage collection runs, allowing Vulkan to release the underlying device allocations in the same pass.
* **Decoupling `Driver::tick()` from cache eviction**: Moving cache eviction out of `VulkanDriver::tick()` and into `VulkanDriver::skipFrame()` prevents periodic `tick()` calls from evicting warm caches during `GPU_CATCH_UP` or when `Renderer::beginFrame()` returns `false`. `FRenderer::skipFrame()` also drops its separate `driver.tick()` call because each backend runs its own lightweight tick polling inside `Driver::skipFrame()`.
