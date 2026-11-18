//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setTighterShadowCasterCullingEnabled](set-tighter-shadow-caster-culling-enabled.md)

# setTighterShadowCasterCullingEnabled

[main]\
open fun [setTighterShadowCasterCullingEnabled](set-tighter-shadow-caster-culling-enabled.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables or disables tighter shadow caster culling for directional shadow maps. 

Disabled by default. This can improve performance by culling shadow casters more aggressively against the view frustum silhouette.

**Warning:** We discourage enabling this option when using advanced shadow filtering techniques such as Variance Shadow Maps (VSM), DPCF, PCSS, or shadow mipmapping. Because these filtering techniques sample across wide spatial regions or large blur kernels, shadow casters physically outside the unblurred view silhouette volume may contribute soft shadow penumbras inside the visible screen boundary. Culling them can introduce shadow boundary/edge clipping artifacts.

#### Parameters

main

| | |
|---|---|
| enabled | true enables tighter shadow caster culling, false disables it. |
