//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Fence](index.md)

# Fence

[main]\
open class [Fence](index.md)

## Types

| Name | Summary |
|---|---|
| [FenceStatus](-fence-status/index.md) | [main]<br>enum [FenceStatus](-fence-status/index.md) |
| [Mode](-mode/index.md) | [main]<br>enum [Mode](-mode/index.md) |

## Properties

| Name | Summary |
|---|---|
| [WAIT_FOR_EVER](-w-a-i-t_-f-o-r_-e-v-e-r.md) | [main]<br>val [WAIT_FOR_EVER](-w-a-i-t_-f-o-r_-e-v-e-r.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = -1 |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [wait](wait.md) | [main]<br>open fun [wait](wait.md)(mode: [Fence.Mode](-mode/index.md), timeoutNanoSeconds: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Fence.FenceStatus](-fence-status/index.md)<br>Client-side wait on the Fence. |
| [waitAndDestroy](wait-and-destroy.md) | [main]<br>open fun [waitAndDestroy](wait-and-destroy.md)(fence: [Fence](index.md), mode: [Fence.Mode](-mode/index.md)): [Fence.FenceStatus](-fence-status/index.md)<br>Client-side wait on a Fence and destroy the Fence. |
