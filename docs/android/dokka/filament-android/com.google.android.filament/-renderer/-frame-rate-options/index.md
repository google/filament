//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[FrameRateOptions](index.md)

# FrameRateOptions

open class [FrameRateOptions](index.md)

Use FrameRateOptions to set the desired frame rate and control how quickly the system reacts to GPU load changes. 

interval: desired frame interval in multiple of the refresh period, set in DisplayInfo (as 1 / DisplayInfo::refreshRate)

The parameters below are relevant when some Views are using dynamic resolution scaling:

headRoomRatio: additional headroom for the GPU as a ratio of the targetFrameTime. Useful for taking into account constant costs like post-processing or GPU drivers on different platforms. history: History size. higher values, tend to filter more (clamped to 31) scaleRate: rate at which the gpu load is adjusted to reach the target frame rate This value can be computed as 1 / N, where N is the number of frames needed to reach 64% of the target scale factor. Higher values make the dynamic resolution react faster.

#### See also

| |
|---|
| [View.DynamicResolutionOptions](../../-view/-dynamic-resolution-options/index.md) |
| [Renderer.DisplayInfo](../-display-info/index.md) |

## Constructors

| | |
|---|---|
| [FrameRateOptions](-frame-rate-options.md) | [main]<br>constructor()constructor(headRoomRatio: Float, scaleRate: Float, history: Int, interval: Int) |

## Properties

| Name | Summary |
|---|---|
| [headRoomRatio](head-room-ratio.md) | [main]<br>open var [headRoomRatio](head-room-ratio.md): Float |
| [history](history.md) | [main]<br>open var [history](history.md): Int |
| [interval](interval.md) | [main]<br>open var [interval](interval.md): Int |
| [scaleRate](scale-rate.md) | [main]<br>open var [scaleRate](scale-rate.md): Float |

## Functions

| Name | Summary |
|---|---|
| [getHeadRoomRatio](get-head-room-ratio.md) | [main]<br>open fun [getHeadRoomRatio](get-head-room-ratio.md)(): Float |
| [getHistory](get-history.md) | [main]<br>open fun [getHistory](get-history.md)(): Int |
| [getInterval](get-interval.md) | [main]<br>open fun [getInterval](get-interval.md)(): Int |
| [getScaleRate](get-scale-rate.md) | [main]<br>open fun [getScaleRate](get-scale-rate.md)(): Float |
| [setHeadRoomRatio](set-head-room-ratio.md) | [main]<br>open fun [setHeadRoomRatio](set-head-room-ratio.md)(headRoomRatio: Float) |
| [setHistory](set-history.md) | [main]<br>open fun [setHistory](set-history.md)(history: Int) |
| [setInterval](set-interval.md) | [main]<br>open fun [setInterval](set-interval.md)(interval: Int) |
| [setScaleRate](set-scale-rate.md) | [main]<br>open fun [setScaleRate](set-scale-rate.md)(scaleRate: Float) |
