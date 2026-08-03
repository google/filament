//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[compile](compile.md)

# compile

[main]\
open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](../-material/-compiler-priority-queue/index.md), variants: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously ensures that a subset of this MaterialInstance's variants are compiled. 

This function behaves identically to [compile](../-material/compile.md), but takes into account the specific constants overridden by setConstant.

#### Parameters

main

| | |
|---|---|
| priority | Priority of the compile command. |
| variants | Variants to include to the compile command. |
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | callback called on the main thread when the compilation is done on by backend. |

#### See also

| |
|---|
| [Material](../-material/compile.md) |
| setConstant |
