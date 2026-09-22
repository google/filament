//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[destroy](destroy.md)

# destroy

[main]\
open fun [destroy](destroy.md)(e: Int)

Destroys this component from the given entity, children are orphaned. 

If this transform had children, these are orphaned, which means their local transform becomes a world transform. Usually it's nonsensical. It's recommended to make sure that a destroyed transform doesn't have children.

#### Parameters

main

| | |
|---|---|
| e | An entity. |

#### See also

| |
|---|
| create |
