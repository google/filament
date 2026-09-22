//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[getSpotLightInnerCone](get-spot-light-inner-cone.md)

# getSpotLightInnerCone

[main]\
open fun [getSpotLightInnerCone](get-spot-light-inner-cone.md)(i: Int): Float

returns the inner cone angle in *radians* between 0 and pi/2. 

The value is recomputed from the initial values, thus is not precisely the same as the one passed to setSpotLightCone() or Builder.spotLightCone().

#### Return

the inner cone angle of this light.

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
