//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Stream](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use `Builder` to construct an Stream object instance. By default, Stream objects are [ACQUIRED](../-stream-type/-a-c-q-u-i-r-e-d/index.md) and must have external images pushed to them via [setAcquiredImage](../set-acquired-image.md). To create a [NATIVE](../-stream-type/-n-a-t-i-v-e/index.md) stream, call the 

```kotlin
stream
```
 method on the builder.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor()<br>Use `Builder` to construct an Stream object instance. |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Stream](../index.md)<br>Creates a new `Stream` object instance. |
| [height](height.md) | [main]<br>open fun [height](height.md)(height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Stream.Builder](index.md) |
| [stream](stream.md) | [main]<br>open fun [stream](stream.md)(streamSource: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Stream.Builder](index.md)<br>Creates a [NATIVE](../-stream-type/-n-a-t-i-v-e/index.md) stream. |
| [width](width.md) | [main]<br>open fun [width](width.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Stream.Builder](index.md) |
