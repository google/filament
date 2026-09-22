//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Exposure](index.md)/[illuminance](illuminance.md)

# illuminance

[main]\
open fun [illuminance](illuminance.md)(camera: [Camera](../-camera/index.md)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the illuminance in lux for the specified camera acting as an incident light meter.

[main]\
open fun [illuminance](illuminance.md)(aperture: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), shutterSpeed: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), sensitivity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the illuminance in lux for the specified exposure parameters of a camera acting as an incident light meter. 

This function is equivalent to calling `illuminance(ev100(aperture, shutterSpeed, sensitivity))` but is slightly faster and offers higher precision.

[main]\
open fun [illuminance](illuminance.md)(ev100: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Converts the specified EV100 to illuminance in lux. 

EV100 is not a measure of illuminance, but an EV100 can be used to denote an illuminance for which a camera would use said EV100 to obtain the nominally correct exposure.
