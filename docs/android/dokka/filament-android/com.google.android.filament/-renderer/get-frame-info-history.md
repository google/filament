//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[getFrameInfoHistory](get-frame-info-history.md)

# getFrameInfoHistory

[main]\
open fun [getFrameInfoHistory](get-frame-info-history.md)(outHistory: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Renderer.FrameInfo](-frame-info/index.md)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Retrieve a history of frame timing information. The maximum frame history size is given by [getMaxFrameHistorySize](get-max-frame-history-size.md). 

 All or part of the history can be lost when using a different SwapChain in beginFrame(). Provide a pre-allocated array of [FrameInfo](-frame-info/index.md) to receive historical records without garbage collection allocations. 

#### Return

the number of FrameInfo populated.

#### Parameters

main

| | |
|---|---|
| outHistory | pre-allocated array of FrameInfo. |

#### See also

| |
|---|
| [beginFrame](begin-frame.md) |
