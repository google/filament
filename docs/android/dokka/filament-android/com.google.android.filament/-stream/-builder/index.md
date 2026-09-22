//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Stream](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Constructs a Stream object instance. 

By default, Stream objects are ACQUIRED and must have external images pushed to them via 

```kotlin
Stream::setAcquiredImage
```
.

To create a NATIVE stream, call the 

```kotlin
stream
```
 method on the builder.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Stream](../index.md)<br>Creates the Stream object and returns a pointer to it. |
| [height](height.md) | [main]<br>open fun [height](height.md)(height: Int): [Stream.Builder](index.md) |
| [name](name.md) | [main]<br>open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Stream.Builder](index.md)<br>Associate an optional name with this Stream for debugging purposes. |
| [width](width.md) | [main]<br>open fun [width](width.md)(width: Int): [Stream.Builder](index.md) |
