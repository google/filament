//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[setAccurateTranslationsEnabled](set-accurate-translations-enabled.md)

# setAccurateTranslationsEnabled

[main]\
open fun [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables or disable the accurate translation mode. Disabled by default. When accurate translation mode is active, the translation component of all transforms is maintained at double precision. This is only useful if the mat4 version of setTransform() is used, as well as getTransformAccurate().

#### Parameters

main

| | |
|---|---|
| enable | true to enable the accurate translation mode, false to disable. |

#### See also

| |
|---|
| [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md) |
| [create(int, int, double[])](create.md) |
| [setTransform(int, double[])](set-transform.md) |
| [getTransform(int, double[])](get-transform.md) |
| [getWorldTransform(int, double[])](get-world-transform.md) |
