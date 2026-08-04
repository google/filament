//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[getVisibleRenderableCount](get-visible-renderable-count.md)

# getVisibleRenderableCount

[main]\
open fun [getVisibleRenderableCount](get-visible-renderable-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Returns the most recent number of visible renderables for the current Scene as calculated the last time Renderer::render() was called with this View and Scene. Returns -1 if the cache is invalid (e.g. before the first render call, or if the scene was detached).

#### Return

the number of visible renderables, or -1 if no value is available.
