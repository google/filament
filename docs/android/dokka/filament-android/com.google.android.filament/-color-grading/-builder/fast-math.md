//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[fastMath](fast-math.md)

# fastMath

[main]\
open fun [fastMath](fast-math.md)(fastMath: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [ColorGrading.Builder](index.md)

Hints whether the engine is permitted to use fast mathematical approximations (such as SIMD polynomial transcendentals) during LUT generation when eligible. 

Setting fastMath to false forces exact C++ scalar libm calculations. The default is true.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| fastMath | true to allow fast mathematical approximations, false otherwise. |
