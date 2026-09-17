//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Material](index.md)/[createInstance](create-instance.md)

# createInstance

[main]\
open fun [createInstance](create-instance.md)(): [MaterialInstance](../-material-instance/index.md)

Creates a new instance of this material. 

Material instances should be freed using Engine::destroy(const MaterialInstance*).

#### Return

A pointer to the new instance.

[main]\
open fun [createInstance](create-instance.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [MaterialInstance](../-material-instance/index.md)

Creates a new instance of this material. 

Material instances should be freed using Engine::destroy(const MaterialInstance*).

#### Return

A pointer to the new instance.

#### Parameters

main

| | |
|---|---|
| name | Optional name to associate with the given material instance. If this is null, then the instance inherits the material's name. |
