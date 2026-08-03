//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setScissor](set-scissor.md)

# setScissor

[main]\
open fun [setScissor](set-scissor.md)(left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bottom: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Set-up a custom scissor rectangle; by default it is disabled. 

 The scissor rectangle gets clipped by the View's viewport, in other words, the scissor cannot affect fragments outside of the View's Viewport. 

 Currently the scissor is not compatible with dynamic resolution and should always be disabled when dynamic resolution is used. 

#### Parameters

main

| | |
|---|---|
| left | left coordinate of the scissor box relative to the viewport |
| bottom | bottom coordinate of the scissor box relative to the viewport |
| width | width of the scissor box |
| height | height of the scissor box |

#### See also

| |
|---|
| [unsetScissor](unset-scissor.md) |
| [View](../-view/set-dynamic-resolution-options.md) |
