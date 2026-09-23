//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[configure](configure.md)

# configure

[main]\
open fun [configure](configure.md)(config: [FramePacer.Configuration](-configuration/index.md))

Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).

#### Parameters

main

| | |
|---|---|
| config | The new configuration targets to scale to on subsequent frames. |

[main]\
open fun [configure](configure.md)(targetFrameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), latencyNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))
