//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setImage](set-image.md)

# setImage

[main]\
open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), zoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

Updates a sub-image of a 3D texture or 2D texture array for a level. 

Cubemaps are treated like a 2D array of six layers.

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to. |
| level | Level to set the image for. |
| xoffset | Left offset of the sub-region to update. |
| yoffset | Bottom offset of the sub-region to update. |
| zoffset | Depth offset of the sub-region to update. |
| width | Width of the sub-region to update. |
| height | Height of the sub-region to update. |
| depth | Depth of the sub-region to update. |
| buffer | Client-side buffer containing the image to set. The driver will invoke the callback associated with this buffer when the data has been consumed.<br>@attention `engine` must be the instance passed to Builder::build() |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |

[main]\
open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

inline helper to update a 2D texture

#### See also

| |
|---|
| setImage |
