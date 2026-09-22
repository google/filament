//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[DisplayRangeToneMapper](index.md)

# DisplayRangeToneMapper

[main]\
open class [DisplayRangeToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

A tone mapper that converts the input HDR RGB color into one of 16 debug colors 

that represent the pixel's exposure. When the output is cyan, the input color represents middle gray (18% exposure). Every exposure stop above or below middle gray causes a color shift.

The relationship between exposures and colors is:

- -5EV black
- -4EV darkest blue
- -3EV darker blue
- -2EV dark blue
- -1EV blue
- OEV cyan
- +1EV dark green
- +2EV green
- +3EV yellow
- +4EV yellow-orange
- +5EV orange
- +6EV bright red
- +7EV red
- +8EV magenta
- +9EV purple
- +10EV white This tone mapper is useful to validate and tweak scene lighting.

## Constructors

| | |
|---|---|
| [DisplayRangeToneMapper](-display-range-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
