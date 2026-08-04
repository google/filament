//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(): [Engine](../index.md)

Creates an instance of Engine

#### Return

A newly created `Engine`, or `null` if the GPU driver couldn't be initialized, for instance if it doesn't support the right version of OpenGL or OpenGL ES.

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if there isn't enough memory to allocate the command buffer. |
