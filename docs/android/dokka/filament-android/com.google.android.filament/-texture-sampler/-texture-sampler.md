//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TextureSampler](index.md)/[TextureSampler](-texture-sampler.md)

# TextureSampler

[main]\
constructor()

Initializes the `TextureSampler` with default values. Minification filter: [LINEAR_MIPMAP_LINEAR](-min-filter/-l-i-n-e-a-r_-m-i-p-m-a-p_-l-i-n-e-a-r/index.md)Magnification filter: [LINEAR](-mag-filter/-l-i-n-e-a-r/index.md)Wrap modes: [REPEAT](-wrap-mode/-r-e-p-e-a-t/index.md)

[main]\
constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md))

Initializes the `TextureSampler` with default values, but specifying the minification and magnification filters.

#### Parameters

main

| | |
|---|---|
| minMag | [magnification filter](-mag-filter/index.md), the minification filter will be set to the same value. |

[main]\
constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md), wrap: [TextureSampler.WrapMode](-wrap-mode/index.md))

Initializes the `TextureSampler` with user specified values.

#### Parameters

main

| | |
|---|---|
| minMag | [magnification filter](-mag-filter/index.md), the minification filter will be set to the same value. |
| wrap | [wrapping mode](-wrap-mode/index.md) for all directions |

[main]\
constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), wrap: [TextureSampler.WrapMode](-wrap-mode/index.md))

Initializes the `TextureSampler` with user specified values.

#### Parameters

main

| | |
|---|---|
| min | [magnification filter](-mag-filter/index.md) |
| mag | [minification filter](-min-filter/index.md) |
| wrap | [wrapping mode](-wrap-mode/index.md) for all directions |

[main]\
constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), s: [TextureSampler.WrapMode](-wrap-mode/index.md), t: [TextureSampler.WrapMode](-wrap-mode/index.md), r: [TextureSampler.WrapMode](-wrap-mode/index.md))

Initializes the `TextureSampler` with user specified values.

#### Parameters

main

| | |
|---|---|
| min | [magnification filter](-mag-filter/index.md) |
| mag | [minification filter](-min-filter/index.md) |
| s | [wrapping mode](-wrap-mode/index.md) for the s (horizontal) direction |
| t | [wrapping mode](-wrap-mode/index.md) for the t (vertical) direction |
| r | [wrapping mode](-wrap-mode/index.md) fot the r (depth) direction |

[main]\
constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md))

Initializes the `TextureSampler` with user specified comparison mode. The comparison fonction is set to [LESS_EQUAL](-compare-function/-l-e-s-s_-e-q-u-a-l/index.md).

#### Parameters

main

| | |
|---|---|
| mode | [comparison mode](-compare-mode/index.md) |

[main]\
constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md), function: [TextureSampler.CompareFunction](-compare-function/index.md))

Initializes the `TextureSampler` with user specified comparison mode and function.

#### Parameters

main

| | |
|---|---|
| mode | [comparison mode](-compare-mode/index.md) |
| function | [comparison function](-compare-function/index.md) |
