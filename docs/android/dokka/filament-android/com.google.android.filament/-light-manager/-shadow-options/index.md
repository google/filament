//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)

# ShadowOptions

[main]\
open class [ShadowOptions](index.md)

Control the quality / performance of the shadow map associated to this light

## Constructors

| | |
|---|---|
| [ShadowOptions](-shadow-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [blurWidth](blur-width.md) | [main]<br>open var [blurWidth](blur-width.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Blur width for the VSM blur. |
| [cascadeSplitPositions](cascade-split-positions.md) | [main]<br>open var [cascadeSplitPositions](cascade-split-positions.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>The split positions for shadow cascades. |
| [constantBias](constant-bias.md) | [main]<br>open var [constantBias](constant-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Constant bias in world units (e.g. |
| [elvsm](elvsm.md) | [main]<br>open var [elvsm](elvsm.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>When elvsm is set to true, &quot;Exponential Layered VSM without Layers&quot; are used. |
| [lispsm](lispsm.md) | [main]<br>open var [lispsm](lispsm.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>LiSPSM, or light-space perspective shadow-mapping is a technique allowing to better optimize the use of the shadow-map texture. |
| [mapSize](map-size.md) | [main]<br>open var [mapSize](map-size.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Size of the shadow map in texels. |
| [maxPenumbraRatio](max-penumbra-ratio.md) | [main]<br>open var [maxPenumbraRatio](max-penumbra-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a light-specific maximum geometric ratio applied to Percentage-Closer Soft Shadows (PCSS), overriding the global default. |
| [maxSearchRadius](max-search-radius.md) | [main]<br>open var [maxSearchRadius](max-search-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a light-specific maximum world-space radius used during the PCSS blocker search, overriding the global default. |
| [maxShadowDistance](max-shadow-distance.md) | [main]<br>open var [maxShadowDistance](max-shadow-distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Maximum shadow-occluder distance for screen-space contact shadows (world units). |
| [normalBias](normal-bias.md) | [main]<br>open var [normalBias](normal-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Amount by which the maximum sampling error is scaled. |
| [penumbraRatioScale](penumbra-ratio-scale.md) | [main]<br>open var [penumbraRatioScale](penumbra-ratio-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a light-specific scale factor applied to the PCSS geometric ratio before clamping. |
| [penumbraScale](penumbra-scale.md) | [main]<br>open var [penumbraScale](penumbra-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a light-specific scale factor applied to the final penumbra size of PCSS shadows. |
| [screenSpaceContactShadows](screen-space-contact-shadows.md) | [main]<br>open var [screenSpaceContactShadows](screen-space-contact-shadows.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether screen-space contact shadows are used. |
| [shadowBulbRadius](shadow-bulb-radius.md) | [main]<br>open var [shadowBulbRadius](shadow-bulb-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Light bulb radius used for soft shadows. |
| [shadowCascades](shadow-cascades.md) | [main]<br>open var [shadowCascades](shadow-cascades.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Number of shadow cascades to use for this light. |
| [shadowFar](shadow-far.md) | [main]<br>open var [shadowFar](shadow-far.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Distance from the camera after which shadows are clipped. |
| [shadowFarHint](shadow-far-hint.md) | [main]<br>open var [shadowFarHint](shadow-far-hint.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Optimize the quality of shadows in front of this distance from the camera. |
| [shadowNearHint](shadow-near-hint.md) | [main]<br>open var [shadowNearHint](shadow-near-hint.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Optimize the quality of shadows from this distance from the camera. |
| [stable](stable.md) | [main]<br>open var [stable](stable.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Controls whether the shadow map should be optimized for resolution or stability. |
| [stepCount](step-count.md) | [main]<br>open var [stepCount](step-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Number of ray-marching steps for screen-space contact shadows (8 by default). |
| [transform](transform.md) | [main]<br>open var [transform](transform.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Transforms the shadow direction. |
