//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[SoftShadowOptions](index.md)

# SoftShadowOptions

open class [SoftShadowOptions](index.md)

View-level options for PCSS Shadowing.

#### See also

| | |
|---|---|
| [setSoftShadowOptions](../set-soft-shadow-options.md) | **Warning:** This API is still experimental and subject to change. |

## Constructors

| | |
|---|---|
| [SoftShadowOptions](-soft-shadow-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [maxPenumbraRatio](max-penumbra-ratio.md) | [main]<br>open var [maxPenumbraRatio](max-penumbra-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets the global default maximum geometric ratio applied to Percentage-Closer Soft Shadows (PCSS). |
| [maxSearchRadius](max-search-radius.md) | [main]<br>open var [maxSearchRadius](max-search-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets the global default maximum world-space radius used during the PCSS blocker search. |
| [penumbraRatioScale](penumbra-ratio-scale.md) | [main]<br>open var [penumbraRatioScale](penumbra-ratio-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a global scale factor applied to the PCSS geometric ratio before failsafe clamping. |
| [penumbraScale](penumbra-scale.md) | [main]<br>open var [penumbraScale](penumbra-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Sets a global scale factor applied to the final penumbra size of all PCSS shadows. |
