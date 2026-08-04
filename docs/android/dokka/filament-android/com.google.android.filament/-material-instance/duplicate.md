//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[duplicate](duplicate.md)

# duplicate

[main]\
open fun [duplicate](duplicate.md)(other: [MaterialInstance](index.md), name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [MaterialInstance](index.md)

Creates a new MaterialInstance using another MaterialInstance as a template for initialization. The new MaterialInstance is an instance of the same [Material](../-material/index.md) of the template instance and must be destroyed just like any other MaterialInstance.

#### Return

A new MaterialInstance

#### Parameters

main

| | |
|---|---|
| other | A MaterialInstance to use as a template for initializing a new instance |
| name | A name for the new MaterialInstance or nullptr to use the template's name |
