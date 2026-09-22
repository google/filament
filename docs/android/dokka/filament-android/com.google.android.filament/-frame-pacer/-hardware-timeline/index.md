//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[FramePacer](../index.md)/[HardwareTimeline](index.md)

# HardwareTimeline

[main]\
open class [HardwareTimeline](index.md)

Telemetry for a single expected hardware presentation timeline.

## Constructors

| | |
|---|---|
| [HardwareTimeline](-hardware-timeline.md) | [main]<br>constructor()constructor(expectedPresentationTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), deadline: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [deadline](deadline.md) | [main]<br>open var [deadline](deadline.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [expectedPresentationTime](expected-presentation-time.md) | [main]<br>open var [expectedPresentationTime](expected-presentation-time.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getDeadline](get-deadline.md) | [main]<br>open fun [getDeadline](get-deadline.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getExpectedPresentationTime](get-expected-presentation-time.md) | [main]<br>open fun [getExpectedPresentationTime](get-expected-presentation-time.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setDeadline](set-deadline.md) | [main]<br>open fun [setDeadline](set-deadline.md)(deadline: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setExpectedPresentationTime](set-expected-presentation-time.md) | [main]<br>open fun [setExpectedPresentationTime](set-expected-presentation-time.md)(expectedPresentationTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
