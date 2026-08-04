//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[VsmShadowOptions](index.md)/[anisotropy](anisotropy.md)

# anisotropy

[main]\
open var [anisotropy](anisotropy.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Sets the number of anisotropic samples to use when sampling a VSM shadow map. If greater than 0, mipmaps will automatically be generated each frame for all lights. 

The number of anisotropic samples = 2 ^ vsmAnisotropy.
