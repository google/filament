//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[setActiveFeatureLevel](set-active-feature-level.md)

# setActiveFeatureLevel

[main]\
open fun [setActiveFeatureLevel](set-active-feature-level.md)(featureLevel: [Engine.FeatureLevel](-feature-level/index.md)): [Engine.FeatureLevel](-feature-level/index.md)

Activate all features of a given feature level. If an explicit feature level is not specified at Engine initialization time via [featureLevel](-builder/feature-level.md), the default feature level is [FEATURE_LEVEL_0](-feature-level/-f-e-a-t-u-r-e_-l-e-v-e-l_0/index.md) on devices not compatible with GLES 3.0; otherwise, the default is [::FEATURE_LEVEL_1](-feature-level/index.md). The selected feature level must not be higher than the value returned by [getActiveFeatureLevel](get-active-feature-level.md) and it's not possible lower the active feature level. Additionally, it is not possible to modify the feature level at all if the Engine was initialized at [FEATURE_LEVEL_0](-feature-level/-f-e-a-t-u-r-e_-l-e-v-e-l_0/index.md).

#### Return

the active feature level.

#### Parameters

main

| | |
|---|---|
| featureLevel | the feature level to activate. If featureLevel is lower than [getActiveFeatureLevel](get-active-feature-level.md), the current (higher) feature level is kept. If featureLevel is higher than [getSupportedFeatureLevel](get-supported-feature-level.md), or if the engine was initialized at feature level 0, an exception is thrown, or the program is terminated if exceptions are disabled. |

#### See also

| |
|---|
| [Engine.Builder](-builder/feature-level.md) |
| [getSupportedFeatureLevel](get-supported-feature-level.md) |
| [getActiveFeatureLevel](get-active-feature-level.md) |

#### Throws

| | |
|---|---|
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if the feature level cannot be set. |
