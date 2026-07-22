//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setShift](set-shift.md)

# setShift

[main]\
open fun [setShift](set-shift.md)(xshift: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), yshift: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets an additional matrix that shifts (translates) the projection matrix. 

 The shift parameters are specified in NDC coordinates, that is, if the translation must be specified in pixels, the xshift and yshift parameters be scaled by 1.0 / viewport.width and 1.0 / viewport.height respectively. 

#### Parameters

main

| | |
|---|---|
| xshift | horizontal shift in NDC coordinates applied after the projection |
| yshift | vertical shift in NDC coordinates applied after the projection |

#### See also

| |
|---|
| com.google.android.filament.Camera |
