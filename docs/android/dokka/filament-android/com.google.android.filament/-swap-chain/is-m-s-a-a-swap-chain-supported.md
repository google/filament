//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)

# isMSAASwapChainSupported

[main]\
open fun [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)(engine: [Engine](../-engine/index.md), samples: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Return whether createSwapChain supports the CONFIG_MSAA_*_SAMPLES flag. The default implementation returns false.

#### Return

true if CONFIG_MSAA_*_SAMPLES is supported, false otherwise.

#### Parameters

main

| | |
|---|---|
| engine | A reference to the filament Engine |
| samples | The number of samples |

#### See also

| | |
|---|---|
| com.google.android.filament.SwapChainFlags | *_SAMPLES |
