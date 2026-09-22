//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[VsmShadowOptions](index.md)

# VsmShadowOptions

open class [VsmShadowOptions](index.md)

View-level options for VSM Shadowing. 

**Warning:** This API is still experimental and subject to change.

#### See also

| |
|---|
| [setVsmShadowOptions](../set-vsm-shadow-options.md) |

## Constructors

| | |
|---|---|
| [VsmShadowOptions](-vsm-shadow-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [anisotropy](anisotropy.md) | [main]<br>open var [anisotropy](anisotropy.md): Int<br>Sets the number of anisotropic samples to use when sampling a VSM shadow map. |
| [highPrecision](high-precision.md) | [main]<br>open var [highPrecision](high-precision.md): Boolean<br>Whether to use a 32-bits or 16-bits texture format for VSM shadow maps. |
| [lightBleedReduction](light-bleed-reduction.md) | [main]<br>open var [lightBleedReduction](light-bleed-reduction.md): Float<br>VSM light bleeding reduction amount, between 0 and 1. |
| [minVarianceScale](min-variance-scale.md) | [main]<br>open var [~~minVarianceScale~~](min-variance-scale.md): Float |
| [mipmapping](mipmapping.md) | [main]<br>open var [mipmapping](mipmapping.md): Boolean<br>Whether to generate mipmaps for all VSM shadow maps. |
| [msaaSamples](msaa-samples.md) | [main]<br>open var [msaaSamples](msaa-samples.md): Int<br>The number of MSAA samples to use when rendering VSM shadow maps. |
