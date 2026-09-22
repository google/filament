//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[toSRGB](to-s-r-g-b.md)

# toSRGB

[main]\
open fun [toSRGB](to-s-r-g-b.md)(color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Converts an RGB color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGB color in Rec.709-sRGB-D65 (sRGB) space.

[main]\
open fun [toSRGB](to-s-r-g-b.md)(colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Converts an RGB color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGB color in Rec.709-sRGB-D65 (sRGB) space.

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |

[main]\
open fun [toSRGB](to-s-r-g-b.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Converts an RGBA color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGBA color in Rec.709-sRGB-D65 (sRGB) space the alpha component is left unmodified.

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |
| colorw | (w component) |
