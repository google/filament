//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[BufferObject](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [BufferObject](../index.md)

Creates and returns the `BufferObject` object. After creation, the buffer is uninitialized. Use setBuffer to initialize the `BufferObject`.

#### Return

the newly created `BufferObject` object

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../../-engine/index.md) to associate this `BufferObject`with |

#### See also

| |
|---|
| setBuffer |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the BufferObject could not be created |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
