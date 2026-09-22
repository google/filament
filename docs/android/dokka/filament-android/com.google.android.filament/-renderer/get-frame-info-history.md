//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[getFrameInfoHistory](get-frame-info-history.md)

# getFrameInfoHistory

[main]\
open fun [getFrameInfoHistory](get-frame-info-history.md)(outHistory: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Renderer.FrameInfo](-frame-info/index.md)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

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
