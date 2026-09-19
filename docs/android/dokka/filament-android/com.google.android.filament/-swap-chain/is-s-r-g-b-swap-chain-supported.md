//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md)

# isSRGBSwapChainSupported

[main]\
open fun [isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Return whether createSwapChain supports the CONFIG_SRGB_COLORSPACE flag. 

The default implementation returns false.

#### Return

true if CONFIG_SRGB_COLORSPACE is supported, false otherwise.

#### Parameters

main

| | |
|---|---|
| engine | A pointer to the filament Engine |
