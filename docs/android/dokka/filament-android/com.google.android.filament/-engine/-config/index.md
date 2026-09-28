//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)

# Config

[main]\
open class [Config](index.md)

Config is used to define the memory footprint used by the engine, such as the command buffer size. 

Config can be used to customize engine requirements based on the applications needs.

.perRenderPassArenaSizeMB (default: 3 MiB) +--------------------------+ | | | .perFrameCommandsSizeMB | | (default 2 MiB) | | | +--------------------------+ | (froxel, etc...) | +--------------------------+

.commandBufferSizeMB (default 3MiB) +--------------------------+ | .minCommandBufferSizeMB | +--------------------------+ | .minCommandBufferSizeMB | +--------------------------+ | .minCommandBufferSizeMB | +--------------------------+ : : : :

## Constructors

| | |
|---|---|
| [Config](-config.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [ShaderLanguage](-shader-language/index.md) | [main]<br>enum [ShaderLanguage](-shader-language/index.md) |

## Properties

| Name | Summary |
|---|---|
| [assertNativeWindowIsValid](assert-native-window-is-valid.md) | [main]<br>open var [assertNativeWindowIsValid](assert-native-window-is-valid.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Assert the native window associated to a SwapChain is valid when calling makeCurrent(). |
| [asynchronousMode](asynchronous-mode.md) | [main]<br>open var [asynchronousMode](asynchronous-mode.md): [Engine.AsynchronousMode](../-asynchronous-mode/index.md)<br>Asynchronous mode for the engine. |
| [commandBufferSizeMB](command-buffer-size-m-b.md) | [main]<br>open var [commandBufferSizeMB](command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the low-level command buffer arena. |
| [disableHandleUseAfterFreeCheck](disable-handle-use-after-free-check.md) | [main]<br>open var [disableHandleUseAfterFreeCheck](disable-handle-use-after-free-check.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [disableParallelShaderCompile](disable-parallel-shader-compile.md) | [main]<br>open var [disableParallelShaderCompile](disable-parallel-shader-compile.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Set to `true` to forcibly disable parallel shader compilation in the backend. |
| [driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md) | [main]<br>open var [driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the backend's handle arena. |
| [enableMultipleDirectionalLights](enable-multiple-directional-lights.md) | [main]<br>open var [enableMultipleDirectionalLights](enable-multiple-directional-lights.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether a scene can contain more than one directional light. |
| [forceGLES2Context](force-g-l-e-s2-context.md) | [main]<br>open var [forceGLES2Context](force-g-l-e-s2-context.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [gpuContextPriority](gpu-context-priority.md) | [main]<br>open var [gpuContextPriority](gpu-context-priority.md): [Engine.GpuContextPriority](../-gpu-context-priority/index.md)<br>GPU context priority level. |
| [jobSystemThreadCount](job-system-thread-count.md) | [main]<br>open var [jobSystemThreadCount](job-system-thread-count.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Number of threads to use in Engine's JobSystem. |
| [materialCacheCapacity](material-cache-capacity.md) | [main]<br>open var [materialCacheCapacity](material-cache-capacity.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Capacity of the LRU cache for material definitions. |
| [metalDisablePanicOnDrawableFailure](metal-disable-panic-on-drawable-failure.md) | [main]<br>open var [metalDisablePanicOnDrawableFailure](metal-disable-panic-on-drawable-failure.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>The action to take if a Drawable cannot be acquired. |
| [metalUploadBufferSizeBytes](metal-upload-buffer-size-bytes.md) | [main]<br>open var [metalUploadBufferSizeBytes](metal-upload-buffer-size-bytes.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>When uploading vertex or index data, the Filament Metal backend copies data into a shared staging area before transferring it to the GPU. |
| [minCommandBufferSizeMB](min-command-buffer-size-m-b.md) | [main]<br>open var [minCommandBufferSizeMB](min-command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Minimum size in MiB of a low-level command buffer. |
| [perFrameCommandsSizeMB](per-frame-commands-size-m-b.md) | [main]<br>open var [perFrameCommandsSizeMB](per-frame-commands-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the per-frame high level command buffer. |
| [perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md) | [main]<br>open var [perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the per-frame data arena. |
| [preferredShaderLanguage](preferred-shader-language.md) | [main]<br>open var [preferredShaderLanguage](preferred-shader-language.md): [Engine.Config.ShaderLanguage](-shader-language/index.md) |
| [programCacheCapacity](program-cache-capacity.md) | [main]<br>open var [programCacheCapacity](program-cache-capacity.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Capacity of the LRU cache for program specializations. |
| [resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md) | [main]<br>open var [resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [resourceAllocatorCacheSizeMB](resource-allocator-cache-size-m-b.md) | [main]<br>open var [resourceAllocatorCacheSizeMB](resource-allocator-cache-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md) | [main]<br>open var [sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>The initial size in bytes of the shared uniform buffer used for material instance batching. |
| [SINGLE_THREADED](-s-i-n-g-l-e_-t-h-r-e-a-d-e-d.md) | [main]<br>val [SINGLE_THREADED](-s-i-n-g-l-e_-t-h-r-e-a-d-e-d.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 4294967295<br>Special value for jobSystemThreadCount, forcing the JobSystem to be single-threaded. |
| [stereoscopicEyeCount](stereoscopic-eye-count.md) | [main]<br>open var [stereoscopicEyeCount](stereoscopic-eye-count.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [stereoscopicType](stereoscopic-type.md) | [main]<br>open var [stereoscopicType](stereoscopic-type.md): [Engine.StereoscopicType](../-stereoscopic-type/index.md) |
