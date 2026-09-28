//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[ClearOptions](index.md)

# ClearOptions

[main]\
open class [ClearOptions](index.md)

ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.

## Constructors

| | |
|---|---|
| [ClearOptions](-clear-options.md) | [main]<br>constructor()constructor(clearColorX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorW: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearStencil: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), clear: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html), discard: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))constructor(clearColor: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, clearStencil: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), clear: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html), discard: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [clear](clear.md) | [main]<br>open var [clear](clear.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [clearStencil](clear-stencil.md) | [main]<br>open var [clearStencil](clear-stencil.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [discard](discard.md) | [main]<br>open var [discard](discard.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getClear](get-clear.md) | [main]<br>open fun [getClear](get-clear.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [getClearColor](get-clear-color.md) | [main]<br>open fun [getClearColor](get-clear-color.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>open fun [getClearColor](get-clear-color.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt; |
| [getClearColorW](get-clear-color-w.md) | [main]<br>open fun [getClearColorW](get-clear-color-w.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html) |
| [getClearColorX](get-clear-color-x.md) | [main]<br>open fun [getClearColorX](get-clear-color-x.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html) |
| [getClearColorY](get-clear-color-y.md) | [main]<br>open fun [getClearColorY](get-clear-color-y.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html) |
| [getClearColorZ](get-clear-color-z.md) | [main]<br>open fun [getClearColorZ](get-clear-color-z.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html) |
| [getClearStencil](get-clear-stencil.md) | [main]<br>open fun [getClearStencil](get-clear-stencil.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getDiscard](get-discard.md) | [main]<br>open fun [getDiscard](get-discard.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [setClear](set-clear.md) | [main]<br>open fun [setClear](set-clear.md)(clear: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)) |
| [setClearColor](set-clear-color.md) | [main]<br>open fun [setClearColor](set-clear-color.md)(clearColor: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>open fun [setClearColor](set-clear-color.md)(clearColorX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), clearColorW: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)) |
| [setClearColorW](set-clear-color-w.md) | [main]<br>open fun [setClearColorW](set-clear-color-w.md)(clearColorW: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)) |
| [setClearColorX](set-clear-color-x.md) | [main]<br>open fun [setClearColorX](set-clear-color-x.md)(clearColorX: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)) |
| [setClearColorY](set-clear-color-y.md) | [main]<br>open fun [setClearColorY](set-clear-color-y.md)(clearColorY: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)) |
| [setClearColorZ](set-clear-color-z.md) | [main]<br>open fun [setClearColorZ](set-clear-color-z.md)(clearColorZ: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)) |
| [setClearStencil](set-clear-stencil.md) | [main]<br>open fun [setClearStencil](set-clear-stencil.md)(clearStencil: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [setDiscard](set-discard.md) | [main]<br>open fun [setDiscard](set-discard.md)(discard: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)) |
