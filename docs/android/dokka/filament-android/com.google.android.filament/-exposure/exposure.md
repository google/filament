//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Exposure](index.md)/[exposure](exposure.md)

# exposure

[main]\
open fun [exposure](exposure.md)(camera: [Camera](../-camera/index.md)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the photometric exposure for the specified camera.

[main]\
open fun [exposure](exposure.md)(aperture: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), shutterSpeed: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), sensitivity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the photometric exposure for the specified exposure parameters. 

This function is equivalent to calling `exposure(ev100(aperture, shutterSpeed, sensitivity))` but is slightly faster and offers higher precision.

[main]\
open fun [exposure](exposure.md)(ev100: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the photometric exposure for the given EV100.
