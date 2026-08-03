//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[compile](compile.md)

# compile

[main]\
open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](../-material/-compiler-priority-queue/index.md), material: [Material](../-material/index.md), view: [View](../-view/index.md), shadowReceiver: [Engine.FeatureState](-feature-state/index.md), skinning: [Engine.FeatureState](-feature-state/index.md), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously ensures that the variants of the specified Material required to render it in the provided View are compiled. This determines the necessary permutations of feature flags based on the supplied View, and compiles the corresponding shader variants. See [compile](../-material/compile.md) for important details about the compilation process, callback scheduling, priorities, and flushing the engine.

#### Parameters

main

| | |
|---|---|
| priority | Which priority queue to use, LOW or HIGH. |
| material | The Material whose variants will be compiled. |
| view | The View in which the material will be rendered. |
| shadowReceiver | Indicates whether to compile variants where the material receives shadows. Use FeatureState.INDETERMINATE to compile both permutations. |
| skinning | Indicates whether to compile variants with skinning. Use FeatureState.INDETERMINATE to compile both permutations. |
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | callback called on the main thread when the compilation is done by backend. |
