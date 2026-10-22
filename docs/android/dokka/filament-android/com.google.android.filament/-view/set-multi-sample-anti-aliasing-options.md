//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setMultiSampleAntiAliasingOptions](set-multi-sample-anti-aliasing-options.md)

# setMultiSampleAntiAliasingOptions

[main]\
open fun [setMultiSampleAntiAliasingOptions](set-multi-sample-anti-aliasing-options.md)(options: [View.MultiSampleAntiAliasingOptions](-multi-sample-anti-aliasing-options/index.md))

Enables or disable multi-sample antialiasing (MSAA). 

Disabled by default. Note that MSAA is a post-processing effect, and post-processing is disabled at FL0. If the feature level is set to 0, values passed to this function are ignored.

#### Parameters

main

| | |
|---|---|
| options | multi-sample antialiasing options |
