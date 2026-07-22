//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[channel](channel.md)

# channel

[main]\
open fun [channel](channel.md)(channel: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Set the channel this renderable is associated to. There can be 8 channels. 

All renderables in a given channel are rendered together, regardless of anything else. They are sorted as usual within a channel.

Channels work similarly to priorities, except that they enforce the strongest ordering.

Channels 0 and 1 may not have render primitives using a material with `refractionType` set to `screenspace`.

#### Return

Builder reference for chaining calls.

#### Parameters

main

| | |
|---|---|
| channel | clamped to the range [0..7], defaults to 2. |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |
| [RenderableManager](../index.md) | ::setBlendOrderAt() |
