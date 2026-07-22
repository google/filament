//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Constructs `Engine` objects using a builder pattern.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [backend](backend.md) | [main]<br>open fun [backend](backend.md)(backend: [Engine.Backend](../-backend/index.md)): [Engine.Builder](index.md)<br>Sets the [Backend](../-backend/index.md) for the Engine. |
| [build](build.md) | [main]<br>open fun [build](build.md)(): [Engine](../index.md)<br>Creates an instance of Engine |
| [colorGrading](color-grading.md) | [main]<br>open fun [colorGrading](color-grading.md)(colorGrading: [ColorGrading.Builder](../../-color-grading/-builder/index.md)): [Engine.Builder](index.md)<br>Sets the builder used to create the default ColorGrading object. |
| [config](config.md) | [main]<br>open fun [config](config.md)(config: [Engine.Config](../-config/index.md)): [Engine.Builder](index.md)<br>Configure the Engine with custom parameters. |
| [feature](feature.md) | [main]<br>open fun [feature](feature.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Engine.Builder](index.md)<br>Set a feature flag value. |
| [featureLevel](feature-level.md) | [main]<br>open fun [featureLevel](feature-level.md)(featureLevel: [Engine.FeatureLevel](../-feature-level/index.md)): [Engine.Builder](index.md)<br>Sets the initial featureLevel for the Engine. |
| [paused](paused.md) | [main]<br>open fun [paused](paused.md)(paused: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Engine.Builder](index.md)<br>Sets the initial paused state of the rendering thread. |
| [sharedContext](shared-context.md) | [main]<br>open fun [sharedContext](shared-context.md)(sharedContext: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Engine.Builder](index.md)<br>Sets a sharedContext for the Engine. |
