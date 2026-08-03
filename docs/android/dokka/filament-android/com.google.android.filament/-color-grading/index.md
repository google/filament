//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[ColorGrading](index.md)

# ColorGrading

open class [ColorGrading](index.md)

`ColorGrading` is used to transform (either to modify or correct) the colors of the HDR buffer rendered by Filament. Color grading transforms are applied after lighting, and after any lens effects (bloom for instance), and include tone mapping. 

# Creation, usage and destruction

 A ColorGrading object is created using the ColorGrading::Builder and destroyed by calling Engine::destroy(const ColorGrading*). A ColorGrading object is meant to be set on a View. ```kotlin
 Engine engine = Engine.create();

 ColorGrading colorGrading = ColorGrading.Builder()
             .toneMapping(ColorGrading.ToneMapping.ACES)
             .build(engine);

 myView.setColorGrading(colorGrading);

 engine.destroy(colorGrading);

```

# Performance

 Creating a new ColorGrading object may be more expensive than other Filament objects as a LUT may need to be generated. The generation of this LUT, if necessary, may happen on the CPU. 

# Ordering

 The various transforms held by ColorGrading are applied in the following order: 

- Exposure
- Night adaptation
- White balance
- Channel mixer
- Shadows/mid-tones/highlights
- Slope/offset/power (CDL)
- Contrast
- Vibrance
- Saturation
- Curves
- Tone mapping
- Luminance scaling
- Gamut mapping

# Defaults

 Here are the default color grading options: 

- Exposure: 0.0
- Night adaptation: 0.0
- White balance: temperature `0.0`, and tint `0.0`
- Channel mixer: red `{1,0,0}`, green `{0,1,0}`, blue `{0,0,1}`
- Shadows/mid-tones/highlights: shadows `{1,1,1,0}`, mid-tones `{1,1,1,0}`, highlights `{1,1,1,0}`, ranges `{0,0.333,0.550,1}`
- Slope/offset/power: slope `1.0`, offset `0.0`, and power `1.0`
- Contrast: `1.0`
- Vibrance: `1.0`
- Saturation: `1.0`
- Curves: gamma `{1,1,1}`, midPoint `{1,1,1}`, and scale `{1,1,1}`
- Tone mapping: [ToneMapper.ACESLegacy](../-tone-mapper/-a-c-e-s-legacy/index.md)
- Luminance scaling: false
- Gamut mapping: false

#### See also

| |
|---|
| [View](../-view/index.md) |
| [ToneMapper](../-tone-mapper/index.md) |

## Constructors

| | |
|---|---|
| [ColorGrading](-color-grading.md) | [main]<br>constructor(colorGrading: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use `Builder` to construct a `ColorGrading` object instance. |
| [LutFormat](-lut-format/index.md) | [main]<br>enum [LutFormat](-lut-format/index.md)<br>Color grading LUT format. |
| [QualityLevel](-quality-level/index.md) | [main]<br>enum [QualityLevel](-quality-level/index.md)<br>Color grading quality level. |
| [ToneMapping](-tone-mapping/index.md) | [main]<br>enum [~~ToneMapping~~](-tone-mapping/index.md)<br>List of available tone-mapping operators. |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
