//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[ClearOptions](index.md)

# ClearOptions

[main]\
open class [ClearOptions](index.md)

ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.

## Constructors

| | |
|---|---|
| [ClearOptions](-clear-options.md) | [main]<br>constructor()constructor(clearColorX: Double, clearColorY: Double, clearColorZ: Double, clearColorW: Double, clearStencil: Int, clear: Boolean, discard: Boolean)constructor(clearColor: Array&lt;Double&gt;, clearStencil: Int, clear: Boolean, discard: Boolean) |

## Properties

| Name | Summary |
|---|---|
| [clear](clear.md) | [main]<br>open var [clear](clear.md): Boolean |
| [clearStencil](clear-stencil.md) | [main]<br>open var [clearStencil](clear-stencil.md): Int |
| [discard](discard.md) | [main]<br>open var [discard](discard.md): Boolean |

## Functions

| Name | Summary |
|---|---|
| [getClear](get-clear.md) | [main]<br>open fun [getClear](get-clear.md)(): Boolean |
| [getClearColor](get-clear-color.md) | [main]<br>open fun [getClearColor](get-clear-color.md)(): Array&lt;Double&gt;<br>open fun [getClearColor](get-clear-color.md)(out: Array&lt;Double&gt;): Array&lt;Double&gt; |
| [getClearColorW](get-clear-color-w.md) | [main]<br>open fun [getClearColorW](get-clear-color-w.md)(): Double |
| [getClearColorX](get-clear-color-x.md) | [main]<br>open fun [getClearColorX](get-clear-color-x.md)(): Double |
| [getClearColorY](get-clear-color-y.md) | [main]<br>open fun [getClearColorY](get-clear-color-y.md)(): Double |
| [getClearColorZ](get-clear-color-z.md) | [main]<br>open fun [getClearColorZ](get-clear-color-z.md)(): Double |
| [getClearStencil](get-clear-stencil.md) | [main]<br>open fun [getClearStencil](get-clear-stencil.md)(): Int |
| [getDiscard](get-discard.md) | [main]<br>open fun [getDiscard](get-discard.md)(): Boolean |
| [setClear](set-clear.md) | [main]<br>open fun [setClear](set-clear.md)(clear: Boolean) |
| [setClearColor](set-clear-color.md) | [main]<br>open fun [setClearColor](set-clear-color.md)(clearColor: Array&lt;Double&gt;)<br>open fun [setClearColor](set-clear-color.md)(clearColorX: Double, clearColorY: Double, clearColorZ: Double, clearColorW: Double) |
| [setClearColorW](set-clear-color-w.md) | [main]<br>open fun [setClearColorW](set-clear-color-w.md)(clearColorW: Double) |
| [setClearColorX](set-clear-color-x.md) | [main]<br>open fun [setClearColorX](set-clear-color-x.md)(clearColorX: Double) |
| [setClearColorY](set-clear-color-y.md) | [main]<br>open fun [setClearColorY](set-clear-color-y.md)(clearColorY: Double) |
| [setClearColorZ](set-clear-color-z.md) | [main]<br>open fun [setClearColorZ](set-clear-color-z.md)(clearColorZ: Double) |
| [setClearStencil](set-clear-stencil.md) | [main]<br>open fun [setClearStencil](set-clear-stencil.md)(clearStencil: Int) |
| [setDiscard](set-discard.md) | [main]<br>open fun [setDiscard](set-discard.md)(discard: Boolean) |
