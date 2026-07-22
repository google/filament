//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[stable](stable.md)

# stable

[main]\
open var [stable](stable.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Controls whether the shadow map should be optimized for resolution or stability. When set to true, all resolution enhancing features that can affect stability are disabling, resulting in significantly lower resolution shadows, albeit stable ones. Setting this flag to true always disables LiSPSM (see below).
