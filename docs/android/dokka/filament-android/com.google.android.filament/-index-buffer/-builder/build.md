//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndexBuffer](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndexBuffer](../index.md)

Creates and returns the `IndexBuffer` object. After creation, the index buffer is uninitialized. Use setBuffer to initialized the `IndexBuffer`.

#### Return

the newly created `IndexBuffer` object

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../../-engine/index.md) to associate this `IndexBuffer`with |

#### See also

| |
|---|
| setBuffer |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the IndexBuffer could not be created |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources, or if a parameter to a builder function was invalid. |
