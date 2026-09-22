//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct a ColorGrading object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [ColorGrading](../index.md)<br>Creates the ColorGrading object and returns a pointer to it. |
| [channelMixer](channel-mixer.md) | [main]<br>open fun [channelMixer](channel-mixer.md)(outRed: Array&lt;Float&gt;, outGreen: Array&lt;Float&gt;, outBlue: Array&lt;Float&gt;): [ColorGrading.Builder](index.md)<br>open fun [channelMixer](channel-mixer.md)(outRedx: Float, outRedy: Float, outRedz: Float, outGreenx: Float, outGreeny: Float, outGreenz: Float, outBluex: Float, outBluey: Float, outBluez: Float): [ColorGrading.Builder](index.md)<br>The channel mixer adjustment modifies each output color channel using the specified mix of the source color channels. |
| [contrast](contrast.md) | [main]<br>open fun [contrast](contrast.md)(contrast: Float): [ColorGrading.Builder](index.md)<br>Adjusts the contrast of the image. |
| [curves](curves.md) | [main]<br>open fun [curves](curves.md)(shadowGamma: Array&lt;Float&gt;, midPoint: Array&lt;Float&gt;, highlightScale: Array&lt;Float&gt;): [ColorGrading.Builder](index.md)<br>open fun [curves](curves.md)(shadowGammax: Float, shadowGammay: Float, shadowGammaz: Float, midPointx: Float, midPointy: Float, midPointz: Float, highlightScalex: Float, highlightScaley: Float, highlightScalez: Float): [ColorGrading.Builder](index.md)<br>Applies a curve to each RGB channel of the image. |
| [customLut](custom-lut.md) | [main]<br>open fun [customLut](custom-lut.md)(data: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), dimension: Int): [ColorGrading.Builder](index.md)<br>Specifies a custom 3D color grading LUT to map the final sRGB color. |
| [dimensions](dimensions.md) | [main]<br>open fun [dimensions](dimensions.md)(dim: Int): [ColorGrading.Builder](index.md)<br>When color grading is implemented using a 3D LUT, this sets the dimension of the LUT. |
| [exposure](exposure.md) | [main]<br>open fun [exposure](exposure.md)(exposure: Float): [ColorGrading.Builder](index.md)<br>Adjusts the exposure of this image. |
| [fastMath](fast-math.md) | [main]<br>open fun [fastMath](fast-math.md)(fastMath: Boolean): [ColorGrading.Builder](index.md)<br>Hints whether the engine is permitted to use fast mathematical approximations (such as SIMD polynomial transcendentals) during LUT generation when eligible. |
| [format](format.md) | [main]<br>open fun [format](format.md)(format: [ColorGrading.LutFormat](../-lut-format/index.md)): [ColorGrading.Builder](index.md)<br>When color grading is implemented using a 3D LUT, this sets the texture format of of the LUT. |
| [gamutMapping](gamut-mapping.md) | [main]<br>open fun [gamutMapping](gamut-mapping.md)(gamutMapping: Boolean): [ColorGrading.Builder](index.md)<br>Enables or disables gamut mapping to the destination color space's gamut. |
| [luminanceScaling](luminance-scaling.md) | [main]<br>open fun [luminanceScaling](luminance-scaling.md)(luminanceScaling: Boolean): [ColorGrading.Builder](index.md)<br>Enables or disables the luminance scaling component (LICH) from the exposure value invariant luminance system (EVILS). |
| [nightAdaptation](night-adaptation.md) | [main]<br>open fun [nightAdaptation](night-adaptation.md)(adaptation: Float): [ColorGrading.Builder](index.md)<br>Controls the amount of night adaptation to replicate a more natural representation of low-light conditions as perceived by the human vision system. |
| [quality](quality.md) | [main]<br>open fun [quality](quality.md)(qualityLevel: [ColorGrading.QualityLevel](../-quality-level/index.md)): [ColorGrading.Builder](index.md)<br>Sets the quality level of the color grading. |
| [saturation](saturation.md) | [main]<br>open fun [saturation](saturation.md)(saturation: Float): [ColorGrading.Builder](index.md)<br>Adjusts the saturation of the image. |
| [shadowsMidtonesHighlights](shadows-midtones-highlights.md) | [main]<br>open fun [shadowsMidtonesHighlights](shadows-midtones-highlights.md)(shadows: Array&lt;Float&gt;, midtones: Array&lt;Float&gt;, highlights: Array&lt;Float&gt;, ranges: Array&lt;Float&gt;): [ColorGrading.Builder](index.md)<br>open fun [shadowsMidtonesHighlights](shadows-midtones-highlights.md)(shadowsx: Float, shadowsy: Float, shadowsz: Float, shadowsw: Float, midtonesx: Float, midtonesy: Float, midtonesz: Float, midtonesw: Float, highlightsx: Float, highlightsy: Float, highlightsz: Float, highlightsw: Float, rangesx: Float, rangesy: Float, rangesz: Float, rangesw: Float): [ColorGrading.Builder](index.md)<br>Adjusts the colors separately in 3 distinct tonal ranges or zones: shadows, mid-tones, and highlights. |
| [slopeOffsetPower](slope-offset-power.md) | [main]<br>open fun [slopeOffsetPower](slope-offset-power.md)(slope: Array&lt;Float&gt;, offset: Array&lt;Float&gt;, power: Array&lt;Float&gt;): [ColorGrading.Builder](index.md)<br>open fun [slopeOffsetPower](slope-offset-power.md)(slopex: Float, slopey: Float, slopez: Float, offsetx: Float, offsety: Float, offsetz: Float, powerx: Float, powery: Float, powerz: Float): [ColorGrading.Builder](index.md)<br>Applies a slope, offset, and power, as defined by the ASC CDL (American Society of Cinematographers Color Decision List) to the image. |
| [toneMapper](tone-mapper.md) | [main]<br>open fun [toneMapper](tone-mapper.md)(toneMapper: [ToneMapper](../../-tone-mapper/index.md)): [ColorGrading.Builder](index.md)<br>Selects the tone mapping operator to apply to the HDR color buffer as the last operation of the color grading post-processing step. |
| [toneMapping](tone-mapping.md) | [main]<br>open fun [toneMapping](tone-mapping.md)(toneMapping: [ColorGrading.ToneMapping](../-tone-mapping/index.md)): [ColorGrading.Builder](index.md)<br>Selects the tone mapping operator to apply to the HDR color buffer as the last operation of the color grading post-processing step. |
| [vibrance](vibrance.md) | [main]<br>open fun [vibrance](vibrance.md)(vibrance: Float): [ColorGrading.Builder](index.md)<br>Adjusts the saturation of the image based on the input color's saturation level. |
| [whiteBalance](white-balance.md) | [main]<br>open fun [whiteBalance](white-balance.md)(temperature: Float, tint: Float): [ColorGrading.Builder](index.md)<br>Adjusts the while balance of the image. |
