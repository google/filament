//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)

# ShadowOptions

[main]\
open class [ShadowOptions](index.md)

Control the quality / performance of the shadow map associated to this light

## Constructors

| | |
|---|---|
| [ShadowOptions](-shadow-options.md) | [main]<br>constructor()constructor(mapSize: Int, shadowCascades: Int, cascadeSplitPositions0: Float, cascadeSplitPositions1: Float, cascadeSplitPositions2: Float, constantBias: Float, normalBias: Float, shadowFar: Float, shadowNearHint: Float, shadowFarHint: Float, stable: Boolean, lispsm: Boolean, polygonOffsetConstant: Float, polygonOffsetSlope: Float, screenSpaceContactShadows: Boolean, stepCount: Int, maxShadowDistance: Float, vsmElvsm: Boolean, vsmBlurWidth: Float, shadowBulbRadius: Float, transformX: Float, transformY: Float, transformZ: Float, transformW: Float, penumbraScale: Float, penumbraRatioScale: Float, maxPenumbraRatio: Float, maxSearchRadius: Float)constructor(mapSize: Int, shadowCascades: Int, cascadeSplitPositions: Array&lt;Float&gt;, constantBias: Float, normalBias: Float, shadowFar: Float, shadowNearHint: Float, shadowFarHint: Float, stable: Boolean, lispsm: Boolean, polygonOffsetConstant: Float, polygonOffsetSlope: Float, screenSpaceContactShadows: Boolean, stepCount: Int, maxShadowDistance: Float, vsm: [LightManager.ShadowOptions.Vsm](-vsm/index.md), shadowBulbRadius: Float, transform: Array&lt;Float&gt;, penumbraScale: Float, penumbraRatioScale: Float, maxPenumbraRatio: Float, maxSearchRadius: Float) |

## Types

| Name | Summary |
|---|---|
| [Vsm](-vsm/index.md) | [main]<br>open class [Vsm](-vsm/index.md)<br>Options available when the View's ShadowType is set to VSM. |

## Properties

| Name | Summary |
|---|---|
| [constantBias](constant-bias.md) | [main]<br>open var [constantBias](constant-bias.md): Float |
| [lispsm](lispsm.md) | [main]<br>open var [lispsm](lispsm.md): Boolean |
| [mapSize](map-size.md) | [main]<br>open var [mapSize](map-size.md): Int |
| [maxPenumbraRatio](max-penumbra-ratio.md) | [main]<br>open var [maxPenumbraRatio](max-penumbra-ratio.md): Float |
| [maxSearchRadius](max-search-radius.md) | [main]<br>open var [maxSearchRadius](max-search-radius.md): Float |
| [maxShadowDistance](max-shadow-distance.md) | [main]<br>open var [maxShadowDistance](max-shadow-distance.md): Float |
| [normalBias](normal-bias.md) | [main]<br>open var [normalBias](normal-bias.md): Float |
| [penumbraRatioScale](penumbra-ratio-scale.md) | [main]<br>open var [penumbraRatioScale](penumbra-ratio-scale.md): Float |
| [penumbraScale](penumbra-scale.md) | [main]<br>open var [penumbraScale](penumbra-scale.md): Float |
| [polygonOffsetConstant](polygon-offset-constant.md) | [main]<br>open var [polygonOffsetConstant](polygon-offset-constant.md): Float |
| [polygonOffsetSlope](polygon-offset-slope.md) | [main]<br>open var [polygonOffsetSlope](polygon-offset-slope.md): Float |
| [screenSpaceContactShadows](screen-space-contact-shadows.md) | [main]<br>open var [screenSpaceContactShadows](screen-space-contact-shadows.md): Boolean |
| [shadowBulbRadius](shadow-bulb-radius.md) | [main]<br>open var [shadowBulbRadius](shadow-bulb-radius.md): Float |
| [shadowCascades](shadow-cascades.md) | [main]<br>open var [shadowCascades](shadow-cascades.md): Int |
| [shadowFar](shadow-far.md) | [main]<br>open var [shadowFar](shadow-far.md): Float |
| [shadowFarHint](shadow-far-hint.md) | [main]<br>open var [shadowFarHint](shadow-far-hint.md): Float |
| [shadowNearHint](shadow-near-hint.md) | [main]<br>open var [shadowNearHint](shadow-near-hint.md): Float |
| [stable](stable.md) | [main]<br>open var [stable](stable.md): Boolean |
| [stepCount](step-count.md) | [main]<br>open var [stepCount](step-count.md): Int |

## Functions

| Name | Summary |
|---|---|
| [getCascadeSplitPositions](get-cascade-split-positions.md) | [main]<br>open fun [getCascadeSplitPositions](get-cascade-split-positions.md)(): Array&lt;Float&gt;<br>open fun [getCascadeSplitPositions](get-cascade-split-positions.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt; |
| [getCascadeSplitPositions0](get-cascade-split-positions0.md) | [main]<br>open fun [getCascadeSplitPositions0](get-cascade-split-positions0.md)(): Float |
| [getCascadeSplitPositions1](get-cascade-split-positions1.md) | [main]<br>open fun [getCascadeSplitPositions1](get-cascade-split-positions1.md)(): Float |
| [getCascadeSplitPositions2](get-cascade-split-positions2.md) | [main]<br>open fun [getCascadeSplitPositions2](get-cascade-split-positions2.md)(): Float |
| [getConstantBias](get-constant-bias.md) | [main]<br>open fun [getConstantBias](get-constant-bias.md)(): Float |
| [getLispsm](get-lispsm.md) | [main]<br>open fun [getLispsm](get-lispsm.md)(): Boolean |
| [getMapSize](get-map-size.md) | [main]<br>open fun [getMapSize](get-map-size.md)(): Int |
| [getMaxPenumbraRatio](get-max-penumbra-ratio.md) | [main]<br>open fun [getMaxPenumbraRatio](get-max-penumbra-ratio.md)(): Float |
| [getMaxSearchRadius](get-max-search-radius.md) | [main]<br>open fun [getMaxSearchRadius](get-max-search-radius.md)(): Float |
| [getMaxShadowDistance](get-max-shadow-distance.md) | [main]<br>open fun [getMaxShadowDistance](get-max-shadow-distance.md)(): Float |
| [getNormalBias](get-normal-bias.md) | [main]<br>open fun [getNormalBias](get-normal-bias.md)(): Float |
| [getPenumbraRatioScale](get-penumbra-ratio-scale.md) | [main]<br>open fun [getPenumbraRatioScale](get-penumbra-ratio-scale.md)(): Float |
| [getPenumbraScale](get-penumbra-scale.md) | [main]<br>open fun [getPenumbraScale](get-penumbra-scale.md)(): Float |
| [getPolygonOffsetConstant](get-polygon-offset-constant.md) | [main]<br>open fun [getPolygonOffsetConstant](get-polygon-offset-constant.md)(): Float |
| [getPolygonOffsetSlope](get-polygon-offset-slope.md) | [main]<br>open fun [getPolygonOffsetSlope](get-polygon-offset-slope.md)(): Float |
| [getScreenSpaceContactShadows](get-screen-space-contact-shadows.md) | [main]<br>open fun [getScreenSpaceContactShadows](get-screen-space-contact-shadows.md)(): Boolean |
| [getShadowBulbRadius](get-shadow-bulb-radius.md) | [main]<br>open fun [getShadowBulbRadius](get-shadow-bulb-radius.md)(): Float |
| [getShadowCascades](get-shadow-cascades.md) | [main]<br>open fun [getShadowCascades](get-shadow-cascades.md)(): Int |
| [getShadowFar](get-shadow-far.md) | [main]<br>open fun [getShadowFar](get-shadow-far.md)(): Float |
| [getShadowFarHint](get-shadow-far-hint.md) | [main]<br>open fun [getShadowFarHint](get-shadow-far-hint.md)(): Float |
| [getShadowNearHint](get-shadow-near-hint.md) | [main]<br>open fun [getShadowNearHint](get-shadow-near-hint.md)(): Float |
| [getStable](get-stable.md) | [main]<br>open fun [getStable](get-stable.md)(): Boolean |
| [getStepCount](get-step-count.md) | [main]<br>open fun [getStepCount](get-step-count.md)(): Int |
| [getTransform](get-transform.md) | [main]<br>open fun [getTransform](get-transform.md)(): Array&lt;Float&gt;<br>open fun [getTransform](get-transform.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt; |
| [getTransformW](get-transform-w.md) | [main]<br>open fun [getTransformW](get-transform-w.md)(): Float |
| [getTransformX](get-transform-x.md) | [main]<br>open fun [getTransformX](get-transform-x.md)(): Float |
| [getTransformY](get-transform-y.md) | [main]<br>open fun [getTransformY](get-transform-y.md)(): Float |
| [getTransformZ](get-transform-z.md) | [main]<br>open fun [getTransformZ](get-transform-z.md)(): Float |
| [getVsm](get-vsm.md) | [main]<br>open fun [getVsm](get-vsm.md)(): [LightManager.ShadowOptions.Vsm](-vsm/index.md)<br>open fun [getVsm](get-vsm.md)(out: [LightManager.ShadowOptions.Vsm](-vsm/index.md)): [LightManager.ShadowOptions.Vsm](-vsm/index.md) |
| [setCascadeSplitPositions](set-cascade-split-positions.md) | [main]<br>open fun [setCascadeSplitPositions](set-cascade-split-positions.md)(cascadeSplitPositions: Array&lt;Float&gt;)<br>open fun [setCascadeSplitPositions](set-cascade-split-positions.md)(cascadeSplitPositions0: Float, cascadeSplitPositions1: Float, cascadeSplitPositions2: Float) |
| [setCascadeSplitPositions0](set-cascade-split-positions0.md) | [main]<br>open fun [setCascadeSplitPositions0](set-cascade-split-positions0.md)(cascadeSplitPositions0: Float) |
| [setCascadeSplitPositions1](set-cascade-split-positions1.md) | [main]<br>open fun [setCascadeSplitPositions1](set-cascade-split-positions1.md)(cascadeSplitPositions1: Float) |
| [setCascadeSplitPositions2](set-cascade-split-positions2.md) | [main]<br>open fun [setCascadeSplitPositions2](set-cascade-split-positions2.md)(cascadeSplitPositions2: Float) |
| [setConstantBias](set-constant-bias.md) | [main]<br>open fun [setConstantBias](set-constant-bias.md)(constantBias: Float) |
| [setLispsm](set-lispsm.md) | [main]<br>open fun [setLispsm](set-lispsm.md)(lispsm: Boolean) |
| [setMapSize](set-map-size.md) | [main]<br>open fun [setMapSize](set-map-size.md)(mapSize: Int) |
| [setMaxPenumbraRatio](set-max-penumbra-ratio.md) | [main]<br>open fun [setMaxPenumbraRatio](set-max-penumbra-ratio.md)(maxPenumbraRatio: Float) |
| [setMaxSearchRadius](set-max-search-radius.md) | [main]<br>open fun [setMaxSearchRadius](set-max-search-radius.md)(maxSearchRadius: Float) |
| [setMaxShadowDistance](set-max-shadow-distance.md) | [main]<br>open fun [setMaxShadowDistance](set-max-shadow-distance.md)(maxShadowDistance: Float) |
| [setNormalBias](set-normal-bias.md) | [main]<br>open fun [setNormalBias](set-normal-bias.md)(normalBias: Float) |
| [setPenumbraRatioScale](set-penumbra-ratio-scale.md) | [main]<br>open fun [setPenumbraRatioScale](set-penumbra-ratio-scale.md)(penumbraRatioScale: Float) |
| [setPenumbraScale](set-penumbra-scale.md) | [main]<br>open fun [setPenumbraScale](set-penumbra-scale.md)(penumbraScale: Float) |
| [setPolygonOffsetConstant](set-polygon-offset-constant.md) | [main]<br>open fun [setPolygonOffsetConstant](set-polygon-offset-constant.md)(polygonOffsetConstant: Float) |
| [setPolygonOffsetSlope](set-polygon-offset-slope.md) | [main]<br>open fun [setPolygonOffsetSlope](set-polygon-offset-slope.md)(polygonOffsetSlope: Float) |
| [setScreenSpaceContactShadows](set-screen-space-contact-shadows.md) | [main]<br>open fun [setScreenSpaceContactShadows](set-screen-space-contact-shadows.md)(screenSpaceContactShadows: Boolean) |
| [setShadowBulbRadius](set-shadow-bulb-radius.md) | [main]<br>open fun [setShadowBulbRadius](set-shadow-bulb-radius.md)(shadowBulbRadius: Float) |
| [setShadowCascades](set-shadow-cascades.md) | [main]<br>open fun [setShadowCascades](set-shadow-cascades.md)(shadowCascades: Int) |
| [setShadowFar](set-shadow-far.md) | [main]<br>open fun [setShadowFar](set-shadow-far.md)(shadowFar: Float) |
| [setShadowFarHint](set-shadow-far-hint.md) | [main]<br>open fun [setShadowFarHint](set-shadow-far-hint.md)(shadowFarHint: Float) |
| [setShadowNearHint](set-shadow-near-hint.md) | [main]<br>open fun [setShadowNearHint](set-shadow-near-hint.md)(shadowNearHint: Float) |
| [setStable](set-stable.md) | [main]<br>open fun [setStable](set-stable.md)(stable: Boolean) |
| [setStepCount](set-step-count.md) | [main]<br>open fun [setStepCount](set-step-count.md)(stepCount: Int) |
| [setTransform](set-transform.md) | [main]<br>open fun [setTransform](set-transform.md)(transform: Array&lt;Float&gt;)<br>open fun [setTransform](set-transform.md)(transformX: Float, transformY: Float, transformZ: Float, transformW: Float) |
| [setTransformW](set-transform-w.md) | [main]<br>open fun [setTransformW](set-transform-w.md)(transformW: Float) |
| [setTransformX](set-transform-x.md) | [main]<br>open fun [setTransformX](set-transform-x.md)(transformX: Float) |
| [setTransformY](set-transform-y.md) | [main]<br>open fun [setTransformY](set-transform-y.md)(transformY: Float) |
| [setTransformZ](set-transform-z.md) | [main]<br>open fun [setTransformZ](set-transform-z.md)(transformZ: Float) |
| [setVsm](set-vsm.md) | [main]<br>open fun [setVsm](set-vsm.md)(vsm: [LightManager.ShadowOptions.Vsm](-vsm/index.md)) |
