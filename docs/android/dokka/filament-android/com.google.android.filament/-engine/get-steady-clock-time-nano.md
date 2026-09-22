//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[getSteadyClockTimeNano](get-steady-clock-time-nano.md)

# getSteadyClockTimeNano

[main]\
open fun [getSteadyClockTimeNano](get-steady-clock-time-nano.md)(): Long

Get the current time. 

This is a convenience function that simply returns the time in nanosecond since epoch of std::chrono::steady_clock. A possible implementation is:

```kotlin

    return std::chrono::steady_clock::now().time_since_epoch().count();

```

#### Return

current time in nanosecond since epoch of std::chrono::steady_clock.

#### See also

| |
|---|
| com.google.android.filament.Renderer |
