//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TextureSampler](index.md)/[TextureSampler](-texture-sampler.md)

# TextureSampler

[main]\
constructor()

Creates a default sampler. 

The default parameters are:

- filterMag : NEAREST
- filterMin : NEAREST
- wrapS : CLAMP_TO_EDGE
- wrapT : CLAMP_TO_EDGE
- wrapR : CLAMP_TO_EDGE
- compareMode : NONE
- compareFunc : Less or equal
- no anisotropic filtering

[main]\
constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md))

Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.

#### Parameters

main

| | |
|---|---|
| minMag | filtering for both minification and magnification |

[main]\
constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md), str: [TextureSampler.WrapMode](-wrap-mode/index.md))

Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.

#### Parameters

main

| | |
|---|---|
| minMag | filtering for both minification and magnification |
| str | wrapping mode for all texture coordinate axes |

[main]\
constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md))

Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.

#### Parameters

main

| | |
|---|---|
| min | filtering for minification |
| mag | filtering for magnification |

[main]\
constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), str: [TextureSampler.WrapMode](-wrap-mode/index.md))

Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.

#### Parameters

main

| | |
|---|---|
| min | filtering for minification |
| mag | filtering for magnification |
| str | wrapping mode for all texture coordinate axes |

[main]\
constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), s: [TextureSampler.WrapMode](-wrap-mode/index.md), t: [TextureSampler.WrapMode](-wrap-mode/index.md), r: [TextureSampler.WrapMode](-wrap-mode/index.md))

Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.

#### Parameters

main

| | |
|---|---|
| min | filtering for minification |
| mag | filtering for magnification |
| s | wrap mode for the s (horizontal)texture coordinate |
| t | wrap mode for the t (vertical) texture coordinate |
| r | wrap mode for the r (depth) texture coordinate |

[main]\
constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md))

Creates a TextureSampler with the default parameters but setting the compare mode and function

#### Parameters

main

| | |
|---|---|
| mode | Compare mode |

[main]\
constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md), func: [TextureSampler.CompareFunc](-compare-func/index.md))

Creates a TextureSampler with the default parameters but setting the compare mode and function

#### Parameters

main

| | |
|---|---|
| mode | Compare mode |
| func | Compare function |
