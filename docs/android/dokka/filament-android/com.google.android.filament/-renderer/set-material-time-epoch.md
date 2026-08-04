//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setMaterialTimeEpoch](set-material-time-epoch.md)

# setMaterialTimeEpoch

[main]\
open fun [setMaterialTimeEpoch](set-material-time-epoch.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Sets the material time epoch to the specified steady clock timestamp in nanoseconds, i.e. resets the material time to zero relative to that time. 

Use this method to keep the precision of time high in materials, in practice it should be called at least when the application is paused, e.g. `Activity.onPause` in Android.

#### Parameters

main

| | |
|---|---|
| monotonicClockNanos | The steady clock timestamp in nanoseconds to set as the material time epoch. |

#### See also

| |
|---|
| [getMaterialTime](get-material-time.md) |
