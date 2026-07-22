//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Texture](../index.md)

Creates a new `Texture` instance.

#### Return

A newly created `Texture`

#### Parameters

main

| | |
|---|---|
| engine | The [Engine](../../-engine/index.md) to associate this `Texture` with. |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if a parameter to a builder function was invalid. A mode detailed message about the error is output in the system log. |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
