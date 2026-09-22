//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[compile](compile.md)

# compile

[main]\
open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](../-material/-compiler-priority-queue/index.md), material: [Material](../-material/index.md), view: [View](../-view/index.md), shadowReceiver: [Engine.FeatureState](-feature-state/index.md), skinning: [Engine.FeatureState](-feature-state/index.md), handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously ensures that the variants of the specified Material needed to render it in the provided View are compiled.

#### Parameters

main

| | |
|---|---|
| priority | Which priority queue to use, LOW or HIGH. |
| material | The Material to compile. |
| view | The View in which the material will be rendered. |
| shadowReceiver | Indicates whether to compile the shadow-receiving variants. Pass `FeatureState::INDETERMINATE` to compile both permutations. |
| skinning | Indicates whether to compile the skinning variants. Pass `FeatureState::INDETERMINATE` to compile both permutations. |
| handler | Handler to dispatch the callback or nullptr for the default handler. |
| callback | Callback called on the main thread when the compilation is done by the backend. |

#### See also

| |
|---|
| [Material](../-material/compile.md) |
