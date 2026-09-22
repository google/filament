//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)

# Colors

[main]\
open class [Colors](index.md)

Utilities to manipulate and convert colors

## Types

| Name | Summary |
|---|---|
| [ColorConversion](-color-conversion/index.md) | [main]<br>enum [ColorConversion](-color-conversion/index.md)<br>type of color conversion to use when converting to/from sRGB and linear spaces |
| [LinearColor](-linear-color/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [SOURCE](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#SOURCE))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [LinearColor](-linear-color/index.md) |
| [RgbaType](-rgba-type/index.md) | [main]<br>enum [RgbaType](-rgba-type/index.md)<br>types of RGBA colors |
| [RgbType](-rgb-type/index.md) | [main]<br>enum [RgbType](-rgb-type/index.md)<br>types of RGB colors |

## Functions

| Name | Summary |
|---|---|
| [absorptionAtDistance](absorption-at-distance.md) | [main]<br>open fun [absorptionAtDistance](absorption-at-distance.md)(color: Array&lt;Float&gt;, distance: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>open fun [absorptionAtDistance](absorption-at-distance.md)(colorx: Float, colory: Float, colorz: Float, distance: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Computes the Beer-Lambert absorption coefficients from the specified transmittance color and distance. |
| [cct](cct.md) | [main]<br>open fun [cct](cct.md)(K: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Converts a correlated color temperature to a linear RGB color in sRGB space the temperature must be expressed in kelvin and must be in the range 1,000K to 15,000K. |
| [illuminantD](illuminant-d.md) | [main]<br>open fun [illuminantD](illuminant-d.md)(K: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Converts a CIE standard illuminant series D to a linear RGB color in sRGB space the temperature must be expressed in kelvin and must be in the range 4,000K to 25,000K |
| [toLinear](to-linear.md) | [main]<br>open fun [toLinear](to-linear.md)(color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>open fun [toLinear](to-linear.md)(colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>converts an RGB color in sRGB space to an RGB color in linear space<br>[main]<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>converts an RGB color to linear space, the conversion depends on the specified type<br>[main]<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>converts an RGBA color to linear space, the conversion depends on the specified type<br>[main]<br>open fun [toLinear](to-linear.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Converts an RGBA color in Rec.709-sRGB-D65 (sRGB) space to an RGBA color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space the alpha component is left unmodified. |
| [toSRGB](to-s-r-g-b.md) | [main]<br>open fun [toSRGB](to-s-r-g-b.md)(color: Array&lt;Float&gt;, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>open fun [toSRGB](to-s-r-g-b.md)(colorx: Float, colory: Float, colorz: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Converts an RGB color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGB color in Rec.709-sRGB-D65 (sRGB) space.<br>[main]<br>open fun [toSRGB](to-s-r-g-b.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Converts an RGBA color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGBA color in Rec.709-sRGB-D65 (sRGB) space the alpha component is left unmodified. |
