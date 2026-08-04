//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TextureSampler](index.md)

# TextureSampler

[main]\
open class [TextureSampler](index.md)

`TextureSampler` defines how a texture is accessed.

## Constructors

| | |
|---|---|
| [TextureSampler](-texture-sampler.md) | [main]<br>constructor()<br>Initializes the `TextureSampler` with default values.<br>constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md))<br>Initializes the `TextureSampler` with default values, but specifying the minification and magnification filters.<br>constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md), wrap: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Initializes the `TextureSampler` with user specified values.<br>constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), wrap: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Initializes the `TextureSampler` with user specified values.<br>constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), s: [TextureSampler.WrapMode](-wrap-mode/index.md), t: [TextureSampler.WrapMode](-wrap-mode/index.md), r: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Initializes the `TextureSampler` with user specified values.<br>constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md))<br>Initializes the `TextureSampler` with user specified comparison mode.<br>constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md), function: [TextureSampler.CompareFunction](-compare-function/index.md))<br>Initializes the `TextureSampler` with user specified comparison mode and function. |

## Types

| Name | Summary |
|---|---|
| [CompareFunction](-compare-function/index.md) | [main]<br>enum [CompareFunction](-compare-function/index.md)<br>Comparison functions for the depth sampler. |
| [CompareMode](-compare-mode/index.md) | [main]<br>enum [CompareMode](-compare-mode/index.md) |
| [MagFilter](-mag-filter/index.md) | [main]<br>enum [MagFilter](-mag-filter/index.md) |
| [MinFilter](-min-filter/index.md) | [main]<br>enum [MinFilter](-min-filter/index.md) |
| [WrapMode](-wrap-mode/index.md) | [main]<br>enum [WrapMode](-wrap-mode/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getAnisotropy](get-anisotropy.md) | [main]<br>open fun [getAnisotropy](get-anisotropy.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [getCompareFunction](get-compare-function.md) | [main]<br>open fun [getCompareFunction](get-compare-function.md)(): [TextureSampler.CompareFunction](-compare-function/index.md) |
| [getCompareMode](get-compare-mode.md) | [main]<br>open fun [getCompareMode](get-compare-mode.md)(): [TextureSampler.CompareMode](-compare-mode/index.md) |
| [getMagFilter](get-mag-filter.md) | [main]<br>open fun [getMagFilter](get-mag-filter.md)(): [TextureSampler.MagFilter](-mag-filter/index.md) |
| [getMinFilter](get-min-filter.md) | [main]<br>open fun [getMinFilter](get-min-filter.md)(): [TextureSampler.MinFilter](-min-filter/index.md) |
| [getWrapModeR](get-wrap-mode-r.md) | [main]<br>open fun [getWrapModeR](get-wrap-mode-r.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md) |
| [getWrapModeS](get-wrap-mode-s.md) | [main]<br>open fun [getWrapModeS](get-wrap-mode-s.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md) |
| [getWrapModeT](get-wrap-mode-t.md) | [main]<br>open fun [getWrapModeT](get-wrap-mode-t.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md) |
| [setAnisotropy](set-anisotropy.md) | [main]<br>open fun [setAnisotropy](set-anisotropy.md)(anisotropy: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>This controls anisotropic filtering. |
| [setCompareFunction](set-compare-function.md) | [main]<br>open fun [setCompareFunction](set-compare-function.md)(function: [TextureSampler.CompareFunction](-compare-function/index.md))<br>Sets the comparison function. |
| [setCompareMode](set-compare-mode.md) | [main]<br>open fun [setCompareMode](set-compare-mode.md)(mode: [TextureSampler.CompareMode](-compare-mode/index.md))<br>Sets the comparison mode. |
| [setMagFilter](set-mag-filter.md) | [main]<br>open fun [setMagFilter](set-mag-filter.md)(filter: [TextureSampler.MagFilter](-mag-filter/index.md))<br>Sets the magnification filter. |
| [setMinFilter](set-min-filter.md) | [main]<br>open fun [setMinFilter](set-min-filter.md)(filter: [TextureSampler.MinFilter](-min-filter/index.md))<br>Sets the minification filter. |
| [setWrapModeR](set-wrap-mode-r.md) | [main]<br>open fun [setWrapModeR](set-wrap-mode-r.md)(mode: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrapping mode in the r (depth) direction. |
| [setWrapModeS](set-wrap-mode-s.md) | [main]<br>open fun [setWrapModeS](set-wrap-mode-s.md)(mode: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrapping mode in the s (horizontal) direction. |
| [setWrapModeT](set-wrap-mode-t.md) | [main]<br>open fun [setWrapModeT](set-wrap-mode-t.md)(mode: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrapping mode in the t (vertical) direction. |
