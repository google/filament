//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[UserVariantFilterBit](index.md)/[DYNAMIC_LIGHTING](-d-y-n-a-m-i-c_-l-i-g-h-t-i-n-g.md)

# DYNAMIC_LIGHTING

[main]\
open var [DYNAMIC_LIGHTING](-d-y-n-a-m-i-c_-l-i-g-h-t-i-n-g.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Dynamic lighting Since dynamic lighting was migrated to specialization constants, filtering this bit no longer affects the size of offline compiled materials (.filamat). However, we keep it for pruning unnecessary pipeline compilations at runtime.
