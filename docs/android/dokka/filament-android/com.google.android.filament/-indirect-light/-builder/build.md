//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndirectLight](../index.md)

Creates the IndirectLight object and returns a pointer to it.

#### Return

A newly created `IndirectLight`

#### Parameters

main

| | |
|---|---|
| engine | The [Engine](../../-engine/index.md) to associate this `IndirectLight` with. |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if a parameter to a builder function was invalid. |
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources. |
