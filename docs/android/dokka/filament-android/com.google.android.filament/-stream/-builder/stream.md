//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Stream](../index.md)/[Builder](index.md)/[stream](stream.md)

# stream

[main]\
open fun [stream](stream.md)(streamSource: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Stream.Builder](index.md)

Creates a [NATIVE](../-stream-type/-n-a-t-i-v-e/index.md) stream. Native streams can sample data directly from an opaque platform object such as a SurfaceTexture on Android.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| streamSource | an opaque native stream handle, e.g.: on Android this must be a SurfaceTexture object |

#### See also

| |
|---|
| [Texture](../../-texture/set-external-stream.md) |
