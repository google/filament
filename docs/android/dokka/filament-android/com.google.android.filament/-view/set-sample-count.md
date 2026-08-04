//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setSampleCount](set-sample-count.md)

# setSampleCount

[main]\
open fun [~~setSampleCount~~](set-sample-count.md)(count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

---

### Deprecated

---

Sets how many samples are to be used for MSAA in the post-process stage. Default is 1 and disables MSAA. 

 Note that anti-aliasing can also be performed in the post-processing stage, generally at lower cost. See the FXAA option in [setAntiAliasing](set-anti-aliasing.md). 

#### Deprecated

use setMultiSampleAntiAliasingOptions instead

#### Parameters

main

| | |
|---|---|
| count | number of samples to use for multi-sampled anti-aliasing. |
