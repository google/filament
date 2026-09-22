//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setChannelDepthClearEnabled](set-channel-depth-clear-enabled.md)

# setChannelDepthClearEnabled

[main]\
open fun [setChannelDepthClearEnabled](set-channel-depth-clear-enabled.md)(channel: Int, enabled: Boolean)

Sets whether a channel must clear the depth buffer before all primitives are rendered. 

Channel depth clear is off by default for all channels. This is orthogonal to Renderer::setClearOptions().

#### Parameters

main

| | |
|---|---|
| channel | between 0 and 7 |
| enabled | true to enable clear, false to disable |
