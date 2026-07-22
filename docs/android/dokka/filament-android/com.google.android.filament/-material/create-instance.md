//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Material](index.md)/[createInstance](create-instance.md)

# createInstance

[main]\
open fun [createInstance](create-instance.md)(): [MaterialInstance](../-material-instance/index.md)

Creates a new instance of this material. Material instances should be freed using [destroyMaterialInstance](../-engine/destroy-material-instance.md).

#### Return

the new instance

[main]\
open fun [createInstance](create-instance.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [MaterialInstance](../-material-instance/index.md)

Creates a new instance of this material with a specified name. Material instances should be freed using [destroyMaterialInstance](../-engine/destroy-material-instance.md).

#### Return

the new instance

#### Parameters

main

| | |
|---|---|
| name | arbitrary label to associate with the given material instance |
