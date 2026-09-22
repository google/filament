//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getChildren](get-children.md)

# getChildren

[main]\
open fun [getChildren](get-children.md)(i: Int, children: Array&lt;Int&gt;, count: Int): Int

Gets a list of children for a transform component.

#### Return

The number of children written to the pointer.

#### Parameters

main

| | |
|---|---|
| i | The instance of the transform component to query. |
| children | Pointer to array-of-Entity. The array must have at least &quot;count&quot; elements. |
| count | The maximum number of children to retrieve. |
