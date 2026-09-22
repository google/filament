//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Fence](index.md)

# Fence

[main]\
open class [Fence](index.md)

Fence is used to synchronize the application main thread with filament's rendering thread.

## Types

| Name | Summary |
|---|---|
| [FenceStatus](-fence-status/index.md) | [main]<br>enum [FenceStatus](-fence-status/index.md)<br>Error codes for Fence::wait() |
| [Mode](-mode/index.md) | [main]<br>enum [Mode](-mode/index.md)<br>Mode controls the behavior of the command stream when calling wait() @attention It would be unwise to call `wait(..., Mode::DONT_FLUSH)` from the same thread the Fence was created, as it would most certainly create a dead-lock. |

## Properties

| Name | Summary |
|---|---|
| [FENCE_WAIT_FOR_EVER](-f-e-n-c-e_-w-a-i-t_-f-o-r_-e-v-e-r.md) | [main]<br>val [FENCE_WAIT_FOR_EVER](-f-e-n-c-e_-w-a-i-t_-f-o-r_-e-v-e-r.md): Long = -1<br>Special `timeout` value to disable wait()'s timeout. |
| [WAIT_FOR_EVER](-w-a-i-t_-f-o-r_-e-v-e-r.md) | [main]<br>val [WAIT_FOR_EVER](-w-a-i-t_-f-o-r_-e-v-e-r.md): Long = -1 |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [wait](wait.md) | [main]<br>open fun [wait](wait.md)(mode: [Fence.Mode](-mode/index.md)): [Fence.FenceStatus](-fence-status/index.md)<br>open fun [wait](wait.md)(mode: [Fence.Mode](-mode/index.md), timeout: Long): [Fence.FenceStatus](-fence-status/index.md)<br>Client-side wait on the Fence. |
| [waitAndDestroy](wait-and-destroy.md) | [main]<br>open fun [waitAndDestroy](wait-and-destroy.md)(fence: [Fence](index.md)): [Fence.FenceStatus](-fence-status/index.md)<br>open fun [waitAndDestroy](wait-and-destroy.md)(fence: [Fence](index.md), mode: [Fence.Mode](-mode/index.md)): [Fence.FenceStatus](-fence-status/index.md)<br>Client-side wait on a Fence and destroy the Fence. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [Fence](index.md) |
