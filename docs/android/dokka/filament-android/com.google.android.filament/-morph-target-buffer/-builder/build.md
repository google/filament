//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MorphTargetBuffer](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [MorphTargetBuffer](../index.md)

Creates and returns the `MorphTargetBuffer` object.

#### Return

the newly created `MorphTargetBuffer` object

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../../-engine/index.md) to associate this `MorphTargetBuffer`with. |

#### See also

| |
|---|
| setMorphTargetBufferOffsetAt |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the MorphTargetBuffer could not be created |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
