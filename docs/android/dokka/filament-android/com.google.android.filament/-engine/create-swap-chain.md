//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[createSwapChain](create-swap-chain.md)

# createSwapChain

[main]\
open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [SwapChain](../-swap-chain/index.md)

Creates an opaque [SwapChain](../-swap-chain/index.md) from the given OS native window handle.

#### Return

a newly created [SwapChain](../-swap-chain/index.md) object

#### Parameters

main

| | |
|---|---|
| surface | on Android, **must be** an instance of android.view.Surface |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if the SwapChain couldn't be created |

[main]\
open fun [createSwapChain](create-swap-chain.md)(surface: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)

Creates a [SwapChain](../-swap-chain/index.md) from the given OS native window handle.

#### Return

a newly created [SwapChain](../-swap-chain/index.md) object

#### Parameters

main

| | |
|---|---|
| surface | on Android, **must be** an instance of android.view.Surface |
| flags | configuration flags, see [SwapChainFlags](../-swap-chain-flags/index.md) |

#### See also

| |
|---|
| [SwapChainFlags](../-swap-chain-flags/-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md) |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if the SwapChain couldn't be created |

[main]\
open fun [createSwapChain](create-swap-chain.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), flags: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [SwapChain](../-swap-chain/index.md)

Creates a headless [SwapChain](../-swap-chain/index.md)

#### Return

a newly created [SwapChain](../-swap-chain/index.md) object

#### Parameters

main

| | |
|---|---|
| width | width of the rendering buffer |
| height | height of the rendering buffer |
| flags | configuration flags, see [SwapChainFlags](../-swap-chain-flags/index.md) |

#### See also

| |
|---|
| [SwapChainFlags](../-swap-chain-flags/-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md) |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | can be thrown if the SwapChain couldn't be created |
