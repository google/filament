//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[textureUseAfterFreePoolSize](texture-use-after-free-pool-size.md)

# textureUseAfterFreePoolSize

[main]\
open var [textureUseAfterFreePoolSize](texture-use-after-free-pool-size.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Number of most-recently destroyed textures to track for use-after-free. This will cause the backend to throw an exception when a texture is freed but still bound to a SamplerGroup and used in a draw call. 0 disables completely. Currently only respected by the Metal backend.
