//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[DepthOfFieldOptions](index.md)

# DepthOfFieldOptions

open class [DepthOfFieldOptions](index.md)

Options to control Depth of Field (DoF) effect in the scene. 

cocScale can be used to set the depth of field blur independently of the camera aperture, e.g. for artistic reasons. This can be achieved by setting: cocScale = cameraAperture / desiredDoFAperture

#### See also

| |
|---|
| [Camera](../../-camera/index.md) |

## Constructors

| | |
|---|---|
| [DepthOfFieldOptions](-depth-of-field-options.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [Filter](-filter/index.md) | [main]<br>enum [Filter](-filter/index.md) |

## Properties

| Name | Summary |
|---|---|
| [backgroundRingCount](background-ring-count.md) | [main]<br>open var [backgroundRingCount](background-ring-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>number of kernel rings for background tiles |
| [cocAspectRatio](coc-aspect-ratio.md) | [main]<br>open var [cocAspectRatio](coc-aspect-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>width/height aspect ratio of the circle of confusion (simulate anamorphic lenses) |
| [cocScale](coc-scale.md) | [main]<br>open var [cocScale](coc-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>circle of confusion scale factor (amount of blur) |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enable or disable depth of field effect |
| [fastGatherRingCount](fast-gather-ring-count.md) | [main]<br>open var [fastGatherRingCount](fast-gather-ring-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>number of kernel rings for fast tiles |
| [filter](filter.md) | [main]<br>open var [filter](filter.md): [View.DepthOfFieldOptions.Filter](-filter/index.md)<br>filter to use for filling gaps in the kernel |
| [foregroundRingCount](foreground-ring-count.md) | [main]<br>open var [foregroundRingCount](foreground-ring-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>number of kernel rings for foreground tiles |
| [maxApertureDiameter](max-aperture-diameter.md) | [main]<br>open var [maxApertureDiameter](max-aperture-diameter.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>maximum aperture diameter in meters (zero to disable rotation) |
| [maxBackgroundCOC](max-background-c-o-c.md) | [main]<br>open var [maxBackgroundCOC](max-background-c-o-c.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>maximum circle-of-confusion in pixels for the background, must be in [0, 32] range. |
| [maxForegroundCOC](max-foreground-c-o-c.md) | [main]<br>open var [maxForegroundCOC](max-foreground-c-o-c.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>maximum circle-of-confusion in pixels for the foreground, must be in [0, 32] range. |
| [nativeResolution](native-resolution.md) | [main]<br>open var [nativeResolution](native-resolution.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>perform DoF processing at native resolution |
