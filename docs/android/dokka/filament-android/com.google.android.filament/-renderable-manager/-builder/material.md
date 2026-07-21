//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[material](material.md)

# material

[main]\
open fun [material](material.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), material: [MaterialInstance](../../-material-instance/index.md)): [RenderableManager.Builder](index.md)

Binds a material instance to the specified primitive. 

If no material is specified for a given primitive, Filament will fall back to a basic default material.

#### Parameters

main

| | |
|---|---|
| index | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| material | the material to bind |
