//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[toLinear](to-linear.md)

# toLinear

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGB color to linear space, the conversion depends on the specified type

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGB color to linear space, the conversion depends on the specified type

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGBA color to linear space, the conversion depends on the specified type

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGBA color to linear space, the conversion depends on the specified type

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |
| colorw | (w component) |

[main]\
open fun [toLinear](to-linear.md)(color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGB color in sRGB space to an RGB color in linear space

[main]\
open fun [toLinear](to-linear.md)(colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

converts an RGB color in sRGB space to an RGB color in linear space

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |

[main]\
open fun [toLinear](to-linear.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;

Converts an RGBA color in Rec.709-sRGB-D65 (sRGB) space to an RGBA color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space the alpha component is left unmodified.

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |
| colorw | (w component) |
