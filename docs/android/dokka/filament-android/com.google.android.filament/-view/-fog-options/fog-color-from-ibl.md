//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[fogColorFromIbl](fog-color-from-ibl.md)

# fogColorFromIbl

[main]\
open var [fogColorFromIbl](fog-color-from-ibl.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

The fog color will be sampled from the IBL in the view direction and tinted by `color`. Depending on the scene this can produce very convincing results. 

This simulates a more anisotropic phase-function.

`fogColorFromIbl` is ignored when skyTexture is specified.

#### See also

| |
|---|
| [skyColor](sky-color.md) |
