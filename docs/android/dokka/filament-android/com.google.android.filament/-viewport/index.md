//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Viewport](index.md)

# Viewport

[main]\
open class [Viewport](index.md)

Viewport describes a view port in pixel coordinates 

A view port is represented by its left-bottom coordinate, width and height in pixels.

## Constructors

| | |
|---|---|
| [Viewport](-viewport.md) | [main]<br>constructor()constructor(left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bottom: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [bottom](bottom.md) | [main]<br>open var [bottom](bottom.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [height](height.md) | [main]<br>open var [height](height.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [left](left.md) | [main]<br>open var [left](left.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [width](width.md) | [main]<br>open var [width](width.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |

## Functions

| Name | Summary |
|---|---|
| [empty](empty.md) | [main]<br>open fun [empty](empty.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the area of the view port is null. |
| [getBottom](get-bottom.md) | [main]<br>open fun [getBottom](get-bottom.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getHeight](get-height.md) | [main]<br>open fun [getHeight](get-height.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getLeft](get-left.md) | [main]<br>open fun [getLeft](get-left.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getWidth](get-width.md) | [main]<br>open fun [getWidth](get-width.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [right](right.md) | [main]<br>open fun [right](right.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>get the right coordinate in window space of the viewport |
| [setBottom](set-bottom.md) | [main]<br>open fun [setBottom](set-bottom.md)(bottom: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [setHeight](set-height.md) | [main]<br>open fun [setHeight](set-height.md)(height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [setLeft](set-left.md) | [main]<br>open fun [setLeft](set-left.md)(left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [setWidth](set-width.md) | [main]<br>open fun [setWidth](set-width.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [top](top.md) | [main]<br>open fun [top](top.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>get the top coordinate in window space of the viewport |
