//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[intensity](intensity.md)

# intensity

[main]\
open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [IndirectLight.Builder](index.md)

Environment intensity (optional). 

Because the environment is encoded usually relative to some reference, the range can be adjusted with this method.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| envIntensity | Scale factor applied to the environment and irradiance such that the result is in *lux*, or *lumen/m^2* (default = 30000) |
