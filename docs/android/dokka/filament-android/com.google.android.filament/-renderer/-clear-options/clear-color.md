//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[ClearOptions](index.md)/[clearColor](clear-color.md)

# clearColor

[main]\
open var [clearColor](clear-color.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Color (sRGB linear) to use to clear the RenderTarget (typically the SwapChain). The RenderTarget is cleared using this color, which won't be tone-mapped since tone-mapping is part of View rendering (this is not). When a View is rendered, there are 3 scenarios to consider: - Pixels rendered by the View replace the clear color (or blend with it in `BlendMode.TRANSLUCENT` mode). - With blending mode set to `BlendMode.TRANSLUCENT`, Pixels untouched by the View are considered fulling transparent and let the clear color show through. - With blending mode set to `BlendMode.OPAQUE`, Pixels untouched by the View are set to the clear color. However, because it is now used in the context of a View, it will go through the post-processing stage, which includes tone-mapping. For consistency, it is recommended to always use a Skybox to clear an opaque View's background, or to use black or fully-transparent (i.e. {0,0,0,0}) as the clear color.
