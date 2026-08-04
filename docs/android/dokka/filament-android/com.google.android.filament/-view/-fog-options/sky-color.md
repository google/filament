//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[skyColor](sky-color.md)

# skyColor

[main]\
open var [skyColor](sky-color.md): [Texture](../../-texture/index.md)

skyTexture must be a mipmapped cubemap. When provided, the fog color will be sampled from this texture, higher resolution mip levels will be used for objects at the far clip plane, and lower resolution mip levels for objects closer to the camera. The skyTexture should typically be heavily blurred; a typical way to produce this texture is to blur the base level with a strong gaussian filter or even an irradiance filter and then generate mip levels as usual. How blurred the base level is somewhat of an artistic decision. 

This simulates a more anisotropic phase-function.

`fogColorFromIbl` is ignored when skyTexture is specified.

In `linearFog` mode mipmap level 0 is always used.

#### See also

| |
|---|
| [Texture](../../-texture/index.md) |
| [fogColorFromIbl](fog-color-from-ibl.md) |
