//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[setIndirectLight](set-indirect-light.md)

# setIndirectLight

[main]\
open fun [setIndirectLight](set-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md))

Sets the [IndirectLight](../-indirect-light/index.md) to use when rendering the `Scene`. Currently, a `Scene` may only have a single [IndirectLight](../-indirect-light/index.md). This call replaces the current [IndirectLight](../-indirect-light/index.md).

#### Parameters

main

| | |
|---|---|
| ibl | the [IndirectLight](../-indirect-light/index.md) to use when rendering the `Scene`or `null` to unset. |
