//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[setActiveFeatureLevel](set-active-feature-level.md)

# setActiveFeatureLevel

[main]\
open fun [setActiveFeatureLevel](set-active-feature-level.md)(featureLevel: [Engine.FeatureLevel](-feature-level/index.md)): [Engine.FeatureLevel](-feature-level/index.md)

Activate all features of a given feature level. 

If an explicit feature level is not specified at Engine initialization time via Builder::featureLevel, the default feature level is FeatureLevel::FEATURE_LEVEL_0 on devices not compatible with GLES 3.0; otherwise, the default is FeatureLevel::FEATURE_LEVEL_1. The selected feature level must not be higher than the value returned by getActiveFeatureLevel() and it's not possible lower the active feature level. Additionally, it is not possible to modify the feature level at all if the Engine was initialized at FeatureLevel::FEATURE_LEVEL_0.

#### Return

the active feature level.

#### Parameters

main

| | |
|---|---|
| featureLevel | the feature level to activate. If featureLevel is lower than getActiveFeatureLevel(), the current (higher) feature level is kept. If featureLevel is higher than getSupportedFeatureLevel(), or if the engine was initialized at feature level 0, an exception is thrown, or the program is terminated if exceptions are disabled. |

#### See also

| |
|---|
| [Engine.Builder](-builder/feature-level.md) |
| [getSupportedFeatureLevel](get-supported-feature-level.md) |
| [getActiveFeatureLevel](get-active-feature-level.md) |
