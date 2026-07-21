//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[maxSearchRadius](max-search-radius.md)

# maxSearchRadius

[main]\
open var [maxSearchRadius](max-search-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a light-specific maximum world-space radius used during the PCSS blocker search, overriding the global default. In PCSS, the shadow algorithm searches a region of the shadow map to find the average depth of occluders. If this search region expands too much, it may inadvertently overlap distinct foreground geometry (like the light's own complex fixture) or climb vertical surfaces (like a pole), causing the shadow to detach from the contact point and appear to float. This parameter allows you to clamp the physical footprint of the blocker search for this specific light, fixing floating contact artifacts without compromising the soft shadows of other lights in the scene. The maximum search radius in world-space meters. Setting this to a value <= 0.0f disables the override and reverts the light to using the global default.

#### See also

| |
|---|
| [View.SoftShadowOptions](../../-view/-soft-shadow-options/max-search-radius.md) |
