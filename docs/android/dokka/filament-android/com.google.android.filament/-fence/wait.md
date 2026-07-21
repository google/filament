//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Fence](index.md)/[wait](wait.md)

# wait

[main]\
open fun [wait](wait.md)(mode: [Fence.Mode](-mode/index.md), timeoutNanoSeconds: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Fence.FenceStatus](-fence-status/index.md)

Client-side wait on the Fence. Blocks the current thread until the Fence signals.

#### Return

FenceStatus::CONDITION_SATISFIED on success, FenceStatus::TIMEOUT_EXPIRED if the time out expired or FenceStatus::ERROR in other cases.

#### Parameters

main

| | |
|---|---|
| mode | Whether the command stream is flushed before waiting or not. |
| timeoutNanoSeconds | Wait time out in nanoseconds. Using a timeout of 0 is a way to query the state of the fence. A timeout value of WAIT_FOR_EVER is used to disable the timeout. |

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if the backend thread encountered an unrecoverable error. |
