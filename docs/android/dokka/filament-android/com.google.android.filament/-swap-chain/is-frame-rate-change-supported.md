//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[isFrameRateChangeSupported](is-frame-rate-change-supported.md)

# isFrameRateChangeSupported

[main]\
open fun [isFrameRateChangeSupported](is-frame-rate-change-supported.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Return whether this SwapChain supports the setFrameRate() API. When a SwapChain is newly created, the actual surface capability state may not be fully determined by the underlying OS, in which case this method will return false. Once the platform completes surface connection, this method will authoritatively return true or false.

#### Return

true if setFrameRate() is definitively supported, false otherwise.
