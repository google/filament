//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [VertexBuffer](../index.md)

Creates the `VertexBuffer` object and returns a pointer to it.

#### Return

the newly created `VertexBuffer` object

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../../-engine/index.md) to associate this `VertexBuffer`with |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the VertexBuffer could not be created |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
