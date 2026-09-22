//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[absorptionAtDistance](absorption-at-distance.md)

# absorptionAtDistance

[main]\
open fun [absorptionAtDistance](absorption-at-distance.md)(color: Array&lt;Float&gt;, distance: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Computes the Beer-Lambert absorption coefficients from the specified transmittance color and distance. 

The computed absorption will guarantee the white light will become the specified color at the specified distance. The output of this function can be used as the absorption parameter of materials that use refraction.

#### Return

absorption coefficients for the Beer-Lambert law

#### Parameters

main

| | |
|---|---|
| color | the desired linear RGB color in sRGB space |
| distance | the distance at which white light should become the specified color |

[main]\
open fun [absorptionAtDistance](absorption-at-distance.md)(colorx: Float, colory: Float, colorz: Float, distance: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Computes the Beer-Lambert absorption coefficients from the specified transmittance color and distance. 

The computed absorption will guarantee the white light will become the specified color at the specified distance. The output of this function can be used as the absorption parameter of materials that use refraction.

#### Return

absorption coefficients for the Beer-Lambert law

#### Parameters

main

| | |
|---|---|
| colorx | (x component) the desired linear RGB color in sRGB space |
| colory | (y component) the desired linear RGB color in sRGB space |
| colorz | (z component) the desired linear RGB color in sRGB space |
| distance | the distance at which white light should become the specified color |
