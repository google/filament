//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[GpuContextPriority](index.md)

# GpuContextPriority

[main]\
enum [GpuContextPriority](index.md)

This controls the priority level for GPU work scheduling, which helps prioritize the submitted GPU work and enables preemption.

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md)<br>Backend default GPU context priority (typically MEDIUM) |
| [LOW](-l-o-w/index.md) | [main]<br>[LOW](-l-o-w/index.md)<br>For non-interactive, deferrable workloads. This should not interfere with standard applications. |
| [MEDIUM](-m-e-d-i-u-m/index.md) | [main]<br>[MEDIUM](-m-e-d-i-u-m/index.md)<br>The default priority level for standard applications. |
| [HIGH](-h-i-g-h/index.md) | [main]<br>[HIGH](-h-i-g-h/index.md)<br>For high-priority, latency-sensitive workloads that are more important than standard applications. |
| [REALTIME](-r-e-a-l-t-i-m-e/index.md) | [main]<br>[REALTIME](-r-e-a-l-t-i-m-e/index.md)<br>The highest priority, intended for system-critical, real-time applications where missing deadlines is unacceptable (e.g., VR/AR compositors or other system-critical tasks). |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Engine.GpuContextPriority](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Engine.GpuContextPriority](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
