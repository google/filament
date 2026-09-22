//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[getFrameInfoHistory](get-frame-info-history.md)

# getFrameInfoHistory

[main]\
open fun [getFrameInfoHistory](get-frame-info-history.md)(outHistory: Array&lt;[Renderer.FrameInfo](-frame-info/index.md)&gt;): Int

Retrieve a history of frame timing information. 

The maximum frame history size is given by getMaxFrameHistorySize(). All or part of the history can be lost when using a different SwapChain in beginFrame().

#### Return

A vector of FrameInfo.

#### Parameters

main

| | |
|---|---|
| outHistory | pre-allocated array of FrameInfo. |

#### See also

| |
|---|
| beginFrame |
