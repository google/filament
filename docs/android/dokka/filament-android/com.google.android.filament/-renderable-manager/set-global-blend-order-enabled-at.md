//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setGlobalBlendOrderEnabledAt](set-global-blend-order-enabled-at.md)

# setGlobalBlendOrderEnabledAt

[main]\
open fun [setGlobalBlendOrderEnabledAt](set-global-blend-order-enabled-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Changes whether the blend order is global or local to this Renderable (by default).

#### Parameters

main

| | |
|---|---|
| instance | the renderable of interest |
| primitiveIndex | the primitive of interest |
| enabled | true for global, false for local blend ordering. |

#### See also

| |
|---|
| [RenderableManager.Builder](-builder/global-blend-order-enabled.md) |
