//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[getLastDynamicResolutionScale](get-last-dynamic-resolution-scale.md)

# getLastDynamicResolutionScale

[main]\
open fun [getLastDynamicResolutionScale](get-last-dynamic-resolution-scale.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Returns the last dynamic resolution scale factor used by this view. This value is updated when Renderer::render(View*) is called

#### Return

A 2-float array containing the horizontal and the vertical scale factors

#### Parameters

main

| | |
|---|---|
| out | A 2-float array where the value will be stored, or null in which case the array is allocated. |

#### See also

| |
|---|
| [Renderer](../-renderer/render.md) |
