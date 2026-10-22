//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[hasGpuFallenBehind](has-gpu-fallen-behind.md)

# hasGpuFallenBehind

[main]\
open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Checks if the GPU rendering pipeline has fallen behind the CPU submission rate.

#### Return

true if the GPU has fallen behind, false otherwise.

#### Parameters

main

| | |
|---|---|
| renderer | The Filament Renderer displaying the target View. |
