//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[setIndirectLight](set-indirect-light.md)

# setIndirectLight

[main]\
open fun [setIndirectLight](set-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md))

Set the IndirectLight to use when rendering the Scene. 

Currently, a Scene may only have a single IndirectLight. This call replaces the current IndirectLight.

#### Parameters

main

| | |
|---|---|
| ibl | The IndirectLight to use when rendering the Scene or nullptr to unset. |

#### See also

| |
|---|
| [getIndirectLight](get-indirect-light.md) |
