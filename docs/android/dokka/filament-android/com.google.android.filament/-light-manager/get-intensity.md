//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[getIntensity](get-intensity.md)

# getIntensity

[main]\
open fun [getIntensity](get-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

returns the light's luminous intensity in *lumens*. 

**note:** for [FOCUSED_SPOT](-type/-f-o-c-u-s-e-d_-s-p-o-t/index.md) lights, the returned value depends on the outer cone angle. 

#### Return

luminous intensity in *lumen*.

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
