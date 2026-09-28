//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[samples](samples.md)

# samples

[main]\
open fun [samples](samples.md)(samples: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)

Specifies the numbers of samples used for MSAA (Multisample Anti-Aliasing). 

Calling this method implicitly indicates the texture is used as a render target. Hence, this method should not be used in conjunction with other methods that are semantically conflicting like `setImage`.

If this is invoked for array textures, it means this texture is used for multiview.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| samples | Number of samples for this texture. |
