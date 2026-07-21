//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[channelMixer](channel-mixer.md)

# channelMixer

[main]\
open fun [channelMixer](channel-mixer.md)(outRed: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, outGreen: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, outBlue: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [ColorGrading.Builder](index.md)

The channel mixer adjustment modifies each output color channel using the specified mix of the source color channels. By default each output color channel is set to use 100% of the corresponding source channel and 0% of the other channels. For instance, the output red channel is set to `{1.0, 0.0, 1.0}` or 100% red, 0% green and 0% blue. Each output channel can add or subtract data from the source channel by using values in the range `[-2.0..+2.0]`. Values outside of that range will be clipped to that range. Using the channel mixer adjustment you can for instance create a monochrome output by setting all 3 output channels to the same mix. For instance: {0.4, 0.4, 0.2} for all 3 output channels(40% red, 40% green and 20% blue). More complex mixes can be used to create more complex effects. For instance, here is a mix that creates a sepia tone effect: 

- 
   `outRed   = {0.255, 0.858, 0.087}`
- 
   `outGreen = {0.213, 0.715, 0.072}`
- 
   `outBlue  = {0.170, 0.572, 0.058}`

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| outRed | The mix of source RGB for the output red channel, between -2.0 and +2.0 |
| outGreen | The mix of source RGB for the output green channel, between -2.0 and +2.0 |
| outBlue | The mix of source RGB for the output blue channel, between -2.0 and +2.0 |
