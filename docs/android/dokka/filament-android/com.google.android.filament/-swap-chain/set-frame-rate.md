//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[setFrameRate](set-frame-rate.md)

# setFrameRate

[main]\
open fun [setFrameRate](set-frame-rate.md)(frameRate: Float)

Sets the intended frame rate for this SwapChain.

#### Parameters

main

| | |
|---|---|
| frameRate | The intended frame rate in frames per second. 0.0f clears/resets the rate. |

[main]\
open fun [setFrameRate](set-frame-rate.md)(frameRate: Float, compatibility: [SwapChain.FrameRateCompatibility](-frame-rate-compatibility/index.md))

Sets the intended frame rate for this SwapChain.

#### Parameters

main

| | |
|---|---|
| frameRate | The intended frame rate in frames per second. 0.0f clears/resets the rate. |
| compatibility | Frame rate compatibility mode (default: DEFAULT). |

[main]\
open fun [setFrameRate](set-frame-rate.md)(frameRate: Float, compatibility: [SwapChain.FrameRateCompatibility](-frame-rate-compatibility/index.md), strategy: [SwapChain.ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md))

Sets the intended frame rate for this SwapChain.

#### Parameters

main

| | |
|---|---|
| frameRate | The intended frame rate in frames per second. 0.0f clears/resets the rate. |
| compatibility | Frame rate compatibility mode (default: DEFAULT). |
| strategy | Change strategy for non-seamless transitions (default: ONLY_IF_SEAMLESS). |
