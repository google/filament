//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[FrameRateOptions](index.md)

# FrameRateOptions

open class [FrameRateOptions](index.md)

Use FrameRateOptions to set the desired frame rate and control how quickly the system reacts to GPU load changes. interval: desired frame interval in multiple of the refresh period, set in DisplayInfo (as 1 / DisplayInfo.refreshRate) The parameters below are relevant when some Views are using dynamic resolution scaling: headRoomRatio: additional headroom for the GPU as a ratio of the targetFrameTime. Useful for taking into account constant costs like post-processing or GPU drivers on different platforms. history: History size. higher values, tend to filter more (clamped to 30) scaleRate: rate at which the gpu load is adjusted to reach the target frame rate This value can be computed as 1 / N, where N is the number of frames needed to reach 64% of the target scale factor. Higher values make the dynamic resolution react faster.

#### See also

| |
|---|
| [View.DynamicResolutionOptions](../../-view/-dynamic-resolution-options/index.md) |
| [Renderer.DisplayInfo](../-display-info/index.md) |

## Constructors

| | |
|---|---|
| [FrameRateOptions](-frame-rate-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [headRoomRatio](head-room-ratio.md) | [main]<br>open var [headRoomRatio](head-room-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Additional headroom for the GPU as a ratio of the targetFrameTime. |
| [history](history.md) | [main]<br>open var [history](history.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>History size. |
| [interval](interval.md) | [main]<br>open var [interval](interval.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Desired frame interval in unit of 1 / DisplayInfo.refreshRate. |
| [scaleRate](scale-rate.md) | [main]<br>open var [scaleRate](scale-rate.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Rate at which the scale will change to reach the target frame rate. |
