//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[SoftShadowOptions](index.md)/[maxPenumbraRatio](max-penumbra-ratio.md)

# maxPenumbraRatio

[main]\
open var [maxPenumbraRatio](max-penumbra-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets the global default maximum geometric ratio applied to Percentage-Closer Soft Shadows (PCSS). In PCSS, the physical width of a shadow's penumbra is determined by the ratio: (distance_to_receiver - distance_to_blocker) / distance_to_blocker Standard shadow maps store a single depth layer (2.5D). When evaluating complex, overlapping occluders (e.g., foliage, layered floating geometry), the shadow map cannot resolve volumetric depth. This limitation can trick the blocker search into returning an artificially close depth value, driving the denominator toward zero. The resulting explosion in penumbra size causes unnatural, massive &quot;ghost&quot; shadows. maxPenumbraRatio applies a smooth, asymptotic squash to the geometric ratio, acting as both a mathematical failsafe and an artistic control. It guarantees the penumbra will gracefully stop expanding before it destroys the visual coherence of the scene. This global value acts as the baseline for all lights. Individual lights can explicitly override this default via the LightManager API to handle specific geometric artifacts without affecting the rest of the scene. - A lower value (e.g., 2.0) creates generally sharper, highly stable shadows that aggressively suppress 2.5D layered occlusion artifacts. - A higher value (e.g., 5.0+) allows for physically accurate, widely expanding soft shadows, but increases susceptibility to ghosting.

#### See also

| | |
|---|---|
| [LightManager](../../-light-manager/index.md) | ::ShadowOptions::maxPenumbraRatio |
