//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[destroyMaterial](destroy-material.md)

# destroyMaterial

[main]\
open fun [destroyMaterial](destroy-material.md)(material: [Material](../-material/index.md))

Destroys a [Material](../-material/index.md) and frees all its associated resources. 

 All [MaterialInstance](../-material-instance/index.md) of the specified [Material](../-material/index.md) must be destroyed before destroying it.

#### Parameters

main

| | |
|---|---|
| material | the [Material](../-material/index.md) to destroy |

#### Throws

| | |
|---|---|
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if some MaterialInstances remain. |
