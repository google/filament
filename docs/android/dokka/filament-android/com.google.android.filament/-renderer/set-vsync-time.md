//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setVsyncTime](set-vsync-time.md)

# setVsyncTime

[main]\
open fun [setVsyncTime](set-vsync-time.md)(steadyClockTimeNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

The use of this method is optional. 

It sets the VSYNC time expressed as the duration in nanosecond since epoch of std::chrono::steady_clock. If called, passing 0 to vsyncSteadyClockTimeNano in Renderer::beginFrame will use this time instead.

#### Parameters

main

| | |
|---|---|
| steadyClockTimeNano | duration in nanosecond since epoch of std::chrono::steady_clock |

#### See also

| |
|---|
| [Engine](../-engine/get-steady-clock-time-nano.md) |
| com.google.android.filament.Renderer |
