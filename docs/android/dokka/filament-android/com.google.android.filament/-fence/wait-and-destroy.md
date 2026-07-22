//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Fence](index.md)/[waitAndDestroy](wait-and-destroy.md)

# waitAndDestroy

[main]\
open fun [waitAndDestroy](wait-and-destroy.md)(fence: [Fence](index.md), mode: [Fence.Mode](-mode/index.md)): [Fence.FenceStatus](-fence-status/index.md)

Client-side wait on a Fence and destroy the Fence.

#### Return

FenceStatus::CONDITION_SATISFIED on success, FenceStatus::ERROR otherwise.

#### Parameters

main

| | |
|---|---|
| fence | Fence object to wait on. |
| mode | Whether the command stream is flushed before waiting or not. |

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if the backend thread encountered an unrecoverable error. |
