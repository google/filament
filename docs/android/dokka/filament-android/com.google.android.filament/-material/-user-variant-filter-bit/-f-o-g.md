//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[UserVariantFilterBit](index.md)/[FOG](-f-o-g.md)

# FOG

[main]\
val [FOG](-f-o-g.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 16

Fog 

Since fog was migrated to specialization constants, filtering this bit no longer removes shader variants. It removes the fog code (and its sampler) from offline compiled materials (.filamat), and prevents fog specializations from being compiled at runtime.
