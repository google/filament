//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[getInstance](get-instance.md)

# getInstance

[main]\
open fun [getInstance](get-instance.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Gets an Instance representing the renderable component associated with the given Entity. 

Use Instance::isValid() to make sure the component exists.

#### Return

Non-zero handle if the entity has a renderable component, 0 otherwise.

#### Parameters

main

| | |
|---|---|
| e | An Entity. |

#### See also

| |
|---|
| [hasComponent](has-component.md) |
