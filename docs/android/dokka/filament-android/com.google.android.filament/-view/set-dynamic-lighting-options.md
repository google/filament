//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setDynamicLightingOptions](set-dynamic-lighting-options.md)

# setDynamicLightingOptions

[main]\
open fun [setDynamicLightingOptions](set-dynamic-lighting-options.md)(zLightNear: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), zLightFar: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Sets options relative to dynamic lighting for this view. 

 Together `zLightNear` and `zLightFar` must be chosen so that the visible influence of lights is spread between these two values. 

#### Parameters

main

| | |
|---|---|
| zLightNear | Distance from the camera where the lights are expected to shine. This parameter can affect performance and is useful because depending on the scene, lights that shine close to the camera may not be visible -- in this case, using a larger value can improve performance. e.g. when standing and looking straight, several meters of the ground isn't visible and if lights are expected to shine there, there is no point using a short zLightNear. (Default 5m). |
| zLightFar | Distance from the camera after which lights are not expected to be visible. Similarly to zLightNear, setting this value properly can improve performance. (Default 100m). |
