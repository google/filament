//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Exposure](index.md)/[luminance](luminance.md)

# luminance

[main]\
open fun [luminance](luminance.md)(camera: [Camera](../-camera/index.md)): Float

Returns the incident luminance in cd / m2 for the specified camera acting as a spot meter.

[main]\
open fun [luminance](luminance.md)(aperture: Float, shutterSpeed: Float, sensitivity: Float): Float

Returns the incident luminance in cd / m2 for the specified exposure parameters of a camera acting as a spot meter. 

This function is equivalent to calling `luminance(ev100(aperture, shutterSpeed, sensitivity))` but is slightly faster and offers higher precision.

[main]\
open fun [luminance](luminance.md)(ev100: Float): Float

Converts the specified EV100 to luminance in cd / m2. 

EV100 is not a measure of luminance, but an EV100 can be used to denote a luminance for which a camera would use said EV100 to obtain the nominally correct exposure
