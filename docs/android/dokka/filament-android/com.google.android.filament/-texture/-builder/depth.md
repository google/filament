//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[depth](depth.md)

# depth

[main]\
open fun [depth](depth.md)(depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)

Specifies the texture's number of layers. Values greater than 1 create a 3D texture. 

This `Texture` instance must use [SAMPLER_2D_ARRAY](../-sampler/-s-a-m-p-l-e-r_2-d_-a-r-r-a-y/index.md) or it has no effect.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| depth | texture number of layers. Default is 1. |
