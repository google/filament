//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[PixelBufferDescriptor](index.md)/[setCallback](set-callback.md)

# setCallback

[main]\
open fun [setCallback](set-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Set or replace the callback called when the CPU-side data is no longer needed.

#### Parameters

main

| | |
|---|---|
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | A callback executed by `handler` when `storage` is no longer needed. |
