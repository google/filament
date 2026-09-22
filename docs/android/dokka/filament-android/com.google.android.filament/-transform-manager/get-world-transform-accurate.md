//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getWorldTransformAccurate](get-world-transform-accurate.md)

# getWorldTransformAccurate

[main]\
open fun [getWorldTransformAccurate](get-world-transform-accurate.md)(ci: Int, out: Array&lt;Double&gt;): Array&lt;Double&gt;

Return the world transform of a transform component.

#### Return

The world transform of the component (i.e. relative to the root). This is the composition of this component's local transform with its parent's world transform.

#### Parameters

main

| | |
|---|---|
| ci | The instance of the transform component to query the world transform from. |

#### See also

| |
|---|
| setTransform |
