//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndirectLight](index.md)/[setIntensity](set-intensity.md)

# setIntensity

[main]\
open fun [setIntensity](set-intensity.md)(intensity: Float)

Sets the environment's intensity. 

Because the environment is encoded usually relative to some reference, the range can be adjusted with this method.

#### Parameters

main

| | |
|---|---|
| intensity | Scale factor applied to the environment and irradiance such that the result is in lux, or *lumen/m^2* (default = 30000) |
