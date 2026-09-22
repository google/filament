//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)

# isMSAASwapChainSupported

[main]\
open fun [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)(engine: [Engine](../-engine/index.md), samples: Int): Boolean

Return whether createSwapChain supports the CONFIG_MSAA_*_SAMPLES flag. 

The default implementation returns false.

#### Return

true if CONFIG_MSAA_*_SAMPLES is supported, false otherwise.

#### Parameters

main

| | |
|---|---|
| engine | A pointer to the filament Engine |
| samples | The number of samples |
