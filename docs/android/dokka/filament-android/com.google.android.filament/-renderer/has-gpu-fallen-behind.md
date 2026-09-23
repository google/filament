//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[hasGpuFallenBehind](has-gpu-fallen-behind.md)

# hasGpuFallenBehind

[main]\
open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Queries whether the GPU execution has fallen behind the CPU rendering execution. 

This is highly useful when managing the application's presentation loop manually (e.g. with the `FramePacer`), allowing the client to proactively detect and react to a latency build-up before continuing with frame execution.

#### Return

true if the GPU pipeline is delayed, false if ready.
