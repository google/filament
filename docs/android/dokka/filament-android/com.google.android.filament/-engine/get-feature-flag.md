//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[getFeatureFlag](get-feature-flag.md)

# getFeatureFlag

[main]\
open fun [getFeatureFlag](get-feature-flag.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Retrieves the value of any feature flag.

#### Return

the value of the flag if it exists

#### Parameters

main

| | |
|---|---|
| name | name of the feature flag |

#### Throws

| | |
|---|---|
| [IllegalArgumentException](https://developer.android.com/reference/kotlin/java/lang/IllegalArgumentException.html) | is thrown if the feature flag doesn't exist |
