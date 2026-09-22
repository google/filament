//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setSampleCount](set-sample-count.md)

# setSampleCount

[main]\
open fun [setSampleCount](set-sample-count.md)()

Sets how many samples are to be used for MSAA in the post-process stage. 

Default is 1 and disables MSAA. Note that post-processing is disabled at FL0. If the feature level is set to 0, values passed to this function are ignored.

Antialiasing can also be performed in the post-processing stage, generally at lower cost. See setAntiAliasing.

#### Deprecated

use setMultiSampleAntiAliasingOptions instead

#### See also

| |
|---|
| [setAntiAliasing](set-anti-aliasing.md) |

[main]\
open fun [setSampleCount](set-sample-count.md)(count: Int)

Sets how many samples are to be used for MSAA in the post-process stage. 

Default is 1 and disables MSAA. Note that post-processing is disabled at FL0. If the feature level is set to 0, values passed to this function are ignored.

Antialiasing can also be performed in the post-processing stage, generally at lower cost. See setAntiAliasing.

#### Deprecated

use setMultiSampleAntiAliasingOptions instead

#### Parameters

main

| | |
|---|---|
| count | number of samples to use for multi-sampled antialiasing. 0: treated as 1 1: no antialiasing n: sample count. Effective sample could be different depending on the GPU capabilities. |

#### See also

| |
|---|
| [setAntiAliasing](set-anti-aliasing.md) |
