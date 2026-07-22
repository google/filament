//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)

# Config

[main]\
open class [Config](index.md)

Parameters for customizing the initialization of [Engine](../index.md).

## Constructors

| | |
|---|---|
| [Config](-config.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [ShaderLanguage](-shader-language/index.md) | [main]<br>enum [ShaderLanguage](-shader-language/index.md)<br>Sets a preferred shader language for Filament to use. |

## Properties

| Name | Summary |
|---|---|
| [assertNativeWindowIsValid](assert-native-window-is-valid.md) | [main]<br>open var [assertNativeWindowIsValid](assert-native-window-is-valid.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Assert the native window associated to a SwapChain is valid when calling makeCurrent(). |
| [commandBufferSizeMB](command-buffer-size-m-b.md) | [main]<br>open var [commandBufferSizeMB](command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the low-level command buffer arena. |
| [disableHandleUseAfterFreeCheck](disable-handle-use-after-free-check.md) | [main]<br>open var [disableHandleUseAfterFreeCheck](disable-handle-use-after-free-check.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Disable backend handles use-after-free checks. |
| [disableParallelShaderCompile](disable-parallel-shader-compile.md) | [main]<br>open var [disableParallelShaderCompile](disable-parallel-shader-compile.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Set to `true` to forcibly disable parallel shader compilation in the backend. |
| [driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md) | [main]<br>open var [driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the backend's handle arena. |
| [forceGLES2Context](force-g-l-e-s2-context.md) | [main]<br>open var [forceGLES2Context](force-g-l-e-s2-context.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>When the OpenGL ES backend is used, setting this value to true will force a GLES2.0 context if supported by the Platform, or if not, will have the backend pretend it's a GLES2 context. |
| [gpuContextPriority](gpu-context-priority.md) | [main]<br>open var [gpuContextPriority](gpu-context-priority.md): [Engine.GpuContextPriority](../-gpu-context-priority/index.md)<br>GPU context priority level. |
| [jobSystemThreadCount](job-system-thread-count.md) | [main]<br>open var [jobSystemThreadCount](job-system-thread-count.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Number of threads to use in Engine's JobSystem. |
| [minCommandBufferSizeMB](min-command-buffer-size-m-b.md) | [main]<br>open var [minCommandBufferSizeMB](min-command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Minimum size in MiB of a low-level command buffer. |
| [perFrameCommandsSizeMB](per-frame-commands-size-m-b.md) | [main]<br>open var [perFrameCommandsSizeMB](per-frame-commands-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the per-frame high level command buffer. |
| [perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md) | [main]<br>open var [perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Size in MiB of the per-frame data arena. |
| [preferredShaderLanguage](preferred-shader-language.md) | [main]<br>open var [preferredShaderLanguage](preferred-shader-language.md): [Engine.Config.ShaderLanguage](-shader-language/index.md) |
| [resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md) | [main]<br>open var [resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>This value determines how many frames texture entries are kept for in the cache. |
| [resourceAllocatorCacheSizeMB](resource-allocator-cache-size-m-b.md) | [main]<br>open var [resourceAllocatorCacheSizeMB](resource-allocator-cache-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md) | [main]<br>open var [sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>The initial size in bytes of the shared uniform buffer used for material instance batching. |
| [stereoscopicEyeCount](stereoscopic-eye-count.md) | [main]<br>open var [stereoscopicEyeCount](stereoscopic-eye-count.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>The number of eyes to render when stereoscopic rendering is enabled. |
| [stereoscopicType](stereoscopic-type.md) | [main]<br>open var [stereoscopicType](stereoscopic-type.md): [Engine.StereoscopicType](../-stereoscopic-type/index.md)<br>The type of technique for stereoscopic rendering. |
| [textureUseAfterFreePoolSize](texture-use-after-free-pool-size.md) | [main]<br>open var [textureUseAfterFreePoolSize](texture-use-after-free-pool-size.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Number of most-recently destroyed textures to track for use-after-free. |
