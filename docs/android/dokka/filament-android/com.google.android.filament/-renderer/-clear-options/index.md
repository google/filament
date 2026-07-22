//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[ClearOptions](index.md)

# ClearOptions

[main]\
open class [ClearOptions](index.md)

ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.

## Constructors

| | |
|---|---|
| [ClearOptions](-clear-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [clear](clear.md) | [main]<br>open var [clear](clear.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether the SwapChain should be cleared using the clearColor. |
| [clearColor](clear-color.md) | [main]<br>open var [clearColor](clear-color.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Color (sRGB linear) to use to clear the RenderTarget (typically the SwapChain). |
| [discard](discard.md) | [main]<br>open var [discard](discard.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether the SwapChain content should be discarded. |
