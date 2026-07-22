//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[shadowCascades](shadow-cascades.md)

# shadowCascades

[main]\
open var [shadowCascades](shadow-cascades.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Number of shadow cascades to use for this light. Must be between 1 and 4 (inclusive). A value greater than 1 turns on cascaded shadow mapping (CSM). Only applicable to Type.SUN or Type.DIRECTIONAL lights. 

 When using shadow cascades, [cascadeSplitPositions](cascade-split-positions.md) must also be set. 

#### See also

| |
|---|
| [LightManager.ShadowOptions](cascade-split-positions.md) |
