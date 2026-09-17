//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Material](index.md)/[compile](compile.md)

# compile

[main]\
open fun [compile](compile.md)(priority: [Material.CompilerPriorityQueue](-compiler-priority-queue/index.md), variants: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously ensures that a subset of this Material's variants are compiled. 

After issuing several Material::compile() calls in a row, it is recommended to call Engine::flush() such that the backend can start the compilation work as soon as possible. The provided callback is guaranteed to be called on the main thread after all specified variants of the material are compiled. This can take hundreds of milliseconds.

If all the material's variants are already compiled, the callback will be scheduled as soon as possible, but this might take a few dozen millisecond, corresponding to how many previous frames are enqueued in the backend. This also varies by backend. Therefore, it is recommended to only call this method once per material shortly after creation.

If the same variant is scheduled for compilation multiple times, the first scheduling takes precedence; later scheduling are ignored.

caveat: A consequence is that if a variant is scheduled on the low priority queue and later scheduled again on the high priority queue, the later scheduling is ignored. Therefore, the second callback could be called before the variant is compiled. However, the first callback, if specified, will trigger as expected.

The callback is guaranteed to be called. If the engine is destroyed while some material variants are still compiling or in the queue, these will be discarded and the corresponding callback will be called. In that case however the Material pointer passed to the callback is guaranteed to be invalid (either because it's been destroyed by the user already, or, because it's been cleaned-up by the Engine).

UserVariantFilterMask::ALL should be used with caution. Only variants that an application needs should be included in the variants argument. For example, the STE variant is only used for stereoscopic rendering. If an application is not planning to render in stereo, this bit should be turned off to avoid unnecessary material compilations.

Note that it is possible to override specialization constants on a per-MaterialInstance basis (

#### Parameters

main

| | |
|---|---|
| priority | Which priority queue to use, LOW or HIGH. |
| variants | Variants to include to the compile command. |
| handler | Handler to dispatch the callback or nullptr for the default handler |
| callback | callback called on the main thread when the compilation is done on by backend. |

#### See also

| |
|---|
| [Material](compile.md) |
