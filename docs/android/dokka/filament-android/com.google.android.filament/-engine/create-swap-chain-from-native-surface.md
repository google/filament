//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md)

# createSwapChainFromNativeSurface

[main]\
open fun [createSwapChainFromNativeSurface](create-swap-chain-from-native-surface.md)(surface: [NativeSurface](../-native-surface/index.md), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)

Creates a [SwapChain](../-swap-chain/index.md) from a [NativeSurface](../-native-surface/index.md).

#### Return

a newly created [SwapChain](../-swap-chain/index.md) object

#### Parameters

main

| | |
|---|---|
| surface | a properly initialized [NativeSurface](../-native-surface/index.md) |
| flags | configuration flags, see [SwapChainFlags](../-swap-chain-flags/index.md) |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if the [SwapChainFlags](../-swap-chain-flags/index.md) couldn't be created |
