//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[isFrameRateChangeSupported](is-frame-rate-change-supported.md)

# isFrameRateChangeSupported

[main]\
open fun [isFrameRateChangeSupported](is-frame-rate-change-supported.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Return whether this SwapChain supports the setFrameRate() API. 

When a SwapChain is newly created, the actual surface capability state may not be fully sealed by the underlying OS. In this case, this method returns Indeterminate. Once the platform completes surface connection, the value permanently seals to True or False.

#### Return

A utils::tribool indicating True, False, or Indeterminate.
