//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[VsmShadowOptions](index.md)/[highPrecision](high-precision.md)

# highPrecision

[main]\
open var [highPrecision](high-precision.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Whether to use a 32-bits or 16-bits texture format for VSM shadow maps. 32-bits precision is rarely needed, but it does reduce light leaks as well as &quot;fading&quot; of the shadows in some situations. Setting highPrecision to true for a single shadow map will double the memory usage of all shadow maps. This may not be supported on all mobile devices.
