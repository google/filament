//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[GenericToneMapper](index.md)/[GenericToneMapper](-generic-tone-mapper.md)

# GenericToneMapper

[main]\
constructor()

Builds a new generic tone mapper. 

The default values of the constructor parameters approximate an ACES tone mapping curve and the maximum input value is set to 10.0.

[main]\
constructor(contrast: Float)

Builds a new generic tone mapper. 

The default values of the constructor parameters approximate an ACES tone mapping curve and the maximum input value is set to 10.0.

#### Parameters

main

| | |
|---|---|
| contrast | controls the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |

[main]\
constructor(contrast: Float, midGrayIn: Float)

Builds a new generic tone mapper. 

The default values of the constructor parameters approximate an ACES tone mapping curve and the maximum input value is set to 10.0.

#### Parameters

main

| | |
|---|---|
| contrast | controls the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| midGrayIn | sets the input middle gray, between 0.0 and 1.0. |

[main]\
constructor(contrast: Float, midGrayIn: Float, midGrayOut: Float)

Builds a new generic tone mapper. 

The default values of the constructor parameters approximate an ACES tone mapping curve and the maximum input value is set to 10.0.

#### Parameters

main

| | |
|---|---|
| contrast | controls the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| midGrayIn | sets the input middle gray, between 0.0 and 1.0. |
| midGrayOut | sets the output middle gray, between 0.0 and 1.0. |

[main]\
constructor(contrast: Float, midGrayIn: Float, midGrayOut: Float, hdrMax: Float)

Builds a new generic tone mapper. 

The default values of the constructor parameters approximate an ACES tone mapping curve and the maximum input value is set to 10.0.

#### Parameters

main

| | |
|---|---|
| contrast | controls the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| midGrayIn | sets the input middle gray, between 0.0 and 1.0. |
| midGrayOut | sets the output middle gray, between 0.0 and 1.0. |
| hdrMax | defines the maximum input value that will be mapped to output white. Must be >= 1.0. |
