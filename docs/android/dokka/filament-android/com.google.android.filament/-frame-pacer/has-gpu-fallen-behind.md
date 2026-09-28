//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)/[hasGpuFallenBehind](has-gpu-fallen-behind.md)

# hasGpuFallenBehind

[main]\
open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(renderer: [Renderer](../-renderer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Checks if the GPU rendering pipeline has fallen behind the CPU submission rate. 

If the GPU is delayed, this method returns true and automatically rolls back any internal cadence state advanced by `setupFrame()`. The client should skip the frame (via `Renderer::skipFrame()`) and refrain from calling `applyPresentationTime()` or `beginFrame()`.

#### Return

true if the GPU has fallen behind, false otherwise.

#### Parameters

main

| | |
|---|---|
| renderer | The Filament Renderer displaying the target View. |
