//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[pick](pick.md)

# pick

[main]\
open fun [pick](pick.md)(x: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), y: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [View.OnPickCallback](-on-pick-callback/index.md))

Creates a picking query. Multiple queries can be created (e.g.: multi-touch). Picking queries are all executed when [render](../-renderer/render.md) is called on this View. The provided callback is guaranteed to be called at some point in the future. Typically it takes a couple frames to receive the result of a picking query.

#### Parameters

main

| | |
|---|---|
| x | Horizontal coordinate to query in the viewport with origin on the left. |
| y | Vertical coordinate to query on the viewport with origin at the bottom. |
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | User callback executed by `handler` when the picking query result is available. |
