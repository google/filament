//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[SkinningBuffer](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [SkinningBuffer](../index.md)

Creates and returns the `SkinningBuffer` object.

#### Return

the newly created `SkinningBuffer` object

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../../-engine/index.md) to associate this `SkinningBuffer`with. |

#### See also

| |
|---|
| [setBonesAsMatrices](../set-bones-as-matrices.md) |
| [setBonesAsQuaternions](../set-bones-as-quaternions.md) |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the SkinningBuffer could not be created |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
