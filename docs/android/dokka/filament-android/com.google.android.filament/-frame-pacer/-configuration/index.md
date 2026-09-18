//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[FramePacer](../index.md)/[Configuration](index.md)

# Configuration

[main]\
open class [Configuration](index.md)

Holds dynamic pacing targets and latency pipeline depth requirements.

## Constructors

| | |
|---|---|
| [Configuration](-configuration.md) | [main]<br>constructor()constructor(targetFrameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), latency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [latency](latency.md) | [main]<br>open var [latency](latency.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [targetFrameRate](target-frame-rate.md) | [main]<br>open var [targetFrameRate](target-frame-rate.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getLatency](get-latency.md) | [main]<br>open fun [getLatency](get-latency.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTargetFrameRate](get-target-frame-rate.md) | [main]<br>open fun [getTargetFrameRate](get-target-frame-rate.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [setLatency](set-latency.md) | [main]<br>open fun [setLatency](set-latency.md)(latency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setTargetFrameRate](set-target-frame-rate.md) | [main]<br>open fun [setTargetFrameRate](set-target-frame-rate.md)(targetFrameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)) |
