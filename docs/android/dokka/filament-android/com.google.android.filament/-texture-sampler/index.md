//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TextureSampler](index.md)

# TextureSampler

[main]\
open class [TextureSampler](index.md)

TextureSampler defines how a texture is accessed.

## Constructors

| | |
|---|---|
| [TextureSampler](-texture-sampler.md) | [main]<br>constructor()<br>Creates a default sampler.<br>constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md))<br>Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.<br>constructor(minMag: [TextureSampler.MagFilter](-mag-filter/index.md), str: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.<br>constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md))<br>Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.<br>constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), str: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.<br>constructor(min: [TextureSampler.MinFilter](-min-filter/index.md), mag: [TextureSampler.MagFilter](-mag-filter/index.md), s: [TextureSampler.WrapMode](-wrap-mode/index.md), t: [TextureSampler.WrapMode](-wrap-mode/index.md), r: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.<br>constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md))<br>Creates a TextureSampler with the default parameters but setting the compare mode and function<br>constructor(mode: [TextureSampler.CompareMode](-compare-mode/index.md), func: [TextureSampler.CompareFunc](-compare-func/index.md))<br>Creates a TextureSampler with the default parameters but setting the compare mode and function |

## Types

| Name | Summary |
|---|---|
| [CompareFunc](-compare-func/index.md) | [main]<br>enum [CompareFunc](-compare-func/index.md)<br>comparison function for the depth / stencil sampler |
| [CompareMode](-compare-mode/index.md) | [main]<br>enum [CompareMode](-compare-mode/index.md)<br>Sampler compare mode |
| [MagFilter](-mag-filter/index.md) | [main]<br>enum [MagFilter](-mag-filter/index.md)<br>Sampler magnification filter |
| [MinFilter](-min-filter/index.md) | [main]<br>enum [MinFilter](-min-filter/index.md)<br>Sampler minification filter |
| [WrapMode](-wrap-mode/index.md) | [main]<br>enum [WrapMode](-wrap-mode/index.md)<br>Sampler Wrap mode |

## Functions

| Name | Summary |
|---|---|
| [getAnisotropy](get-anisotropy.md) | [main]<br>open fun [getAnisotropy](get-anisotropy.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the anisotropy value |
| [getCompareFunc](get-compare-func.md) | [main]<br>open fun [getCompareFunc](get-compare-func.md)(): [TextureSampler.CompareFunc](-compare-func/index.md)<br>returns the compare function |
| [getCompareMode](get-compare-mode.md) | [main]<br>open fun [getCompareMode](get-compare-mode.md)(): [TextureSampler.CompareMode](-compare-mode/index.md)<br>returns the compare mode |
| [getMagFilter](get-mag-filter.md) | [main]<br>open fun [getMagFilter](get-mag-filter.md)(): [TextureSampler.MagFilter](-mag-filter/index.md)<br>returns the magnification filter value |
| [getMinFilter](get-min-filter.md) | [main]<br>open fun [getMinFilter](get-min-filter.md)(): [TextureSampler.MinFilter](-min-filter/index.md)<br>returns the minification filter value |
| [getWrapModeR](get-wrap-mode-r.md) | [main]<br>open fun [getWrapModeR](get-wrap-mode-r.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md)<br>returns the r-coordinate wrap mode (depth) |
| [getWrapModeS](get-wrap-mode-s.md) | [main]<br>open fun [getWrapModeS](get-wrap-mode-s.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md)<br>returns the s-coordinate wrap mode (horizontal) |
| [getWrapModeT](get-wrap-mode-t.md) | [main]<br>open fun [getWrapModeT](get-wrap-mode-t.md)(): [TextureSampler.WrapMode](-wrap-mode/index.md)<br>returns the t-coordinate wrap mode (vertical) |
| [setAnisotropy](set-anisotropy.md) | [main]<br>open fun [setAnisotropy](set-anisotropy.md)(anisotropy: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>This controls anisotropic filtering. |
| [setCompareMode](set-compare-mode.md) | [main]<br>open fun [setCompareMode](set-compare-mode.md)(mode: [TextureSampler.CompareMode](-compare-mode/index.md))<br>open fun [setCompareMode](set-compare-mode.md)(mode: [TextureSampler.CompareMode](-compare-mode/index.md), func: [TextureSampler.CompareFunc](-compare-func/index.md))<br>Sets the compare mode and function. |
| [setMagFilter](set-mag-filter.md) | [main]<br>open fun [setMagFilter](set-mag-filter.md)(v: [TextureSampler.MagFilter](-mag-filter/index.md))<br>Sets the magnification filter |
| [setMinFilter](set-min-filter.md) | [main]<br>open fun [setMinFilter](set-min-filter.md)(v: [TextureSampler.MinFilter](-min-filter/index.md))<br>Sets the minification filter |
| [setWrapModeR](set-wrap-mode-r.md) | [main]<br>open fun [setWrapModeR](set-wrap-mode-r.md)(v: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrap mode for the r (depth, for 3D textures) texture coordinate |
| [setWrapModeS](set-wrap-mode-s.md) | [main]<br>open fun [setWrapModeS](set-wrap-mode-s.md)(v: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrap mode for the s (horizontal) texture coordinate |
| [setWrapModeT](set-wrap-mode-t.md) | [main]<br>open fun [setWrapModeT](set-wrap-mode-t.md)(v: [TextureSampler.WrapMode](-wrap-mode/index.md))<br>Sets the wrap mode for the t (vertical) texture coordinate |
