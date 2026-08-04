//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[levels](levels.md)

# levels

[main]\
open fun [levels](levels.md)(levels: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)

Specifies the number of mipmap levels

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| levels | must be at least 1 and less or equal to `floor(log2(max(width, height))) + 1`. Default is 1. |
