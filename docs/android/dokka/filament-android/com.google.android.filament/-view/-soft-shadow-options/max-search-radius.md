//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[SoftShadowOptions](index.md)/[maxSearchRadius](max-search-radius.md)

# maxSearchRadius

[main]\
open var [maxSearchRadius](max-search-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets the global default maximum world-space radius used during the PCSS blocker search. In PCSS, the shadow algorithm searches a region of the shadow map to find the average depth of occluders. For lights with a large angular size (or objects very far from the light), this search region can become massive. * If the search region expands too much, it may inadvertently overlap distinct foreground geometry (like a streetlight fixture) or climb vertical surfaces (like a pole), causing the shadow to detach from the contact point and appear to float. maxSearchRadius limits the physical footprint of this search. This global value acts as the baseline for all lights. Individual lights can explicitly override this default via the LightManager API to clamp the search footprint for specific geometric setups without affecting the rest of the scene. The maximum search radius in world-space meters: - A smaller value (e.g., 0.05 to 0.1) tightly anchors shadows to their contact points and prevents artifacts near complex light fixtures. - A larger value provides more physically accurate blocker averaging for massive area lights but increases the risk of floating geometry.

#### See also

| | |
|---|---|
| [LightManager](../../-light-manager/index.md) | ::ShadowOptions::maxSearchRadius |
