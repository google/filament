//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setBonesAsQuaternions](set-bones-as-quaternions.md)

# setBonesAsQuaternions

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int, offset: Int)

Updates the bone transforms in the range [offset, offset + count). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | buffer containing transforms data |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (structured element count) in the destination buffer or component |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int)

Updates the bone transforms in the range [0, count). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | buffer containing transforms data |
| count | number of elements (structured element count) in `transforms` |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, arrayOffset: Int, count: Int, offset: Int)

Updates the bone transforms in the range [offset, offset + count). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | array containing transforms data |
| arrayOffset | offset in elements (structured element count) in `transforms` to skip |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (structured element count) in the destination buffer or component |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int, offset: Int)

Updates the bone transforms in the range [offset, offset + count). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | array containing transforms data |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (structured element count) in the destination buffer or component |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int)

Updates the bone transforms in the range [0, count). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | array containing transforms data |
| count | number of elements (structured element count) in `transforms` |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;)

Updates the bone transforms in the range [0, transforms.length / 8). 

The bones must be pre-allocated using Builder::skinning().

#### Parameters

main

| | |
|---|---|
| instance | instance of the component obtained from getInstance() |
| transforms | array containing transforms data |
