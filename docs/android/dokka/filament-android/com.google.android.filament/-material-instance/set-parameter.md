//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setParameter](set-parameter.md)

# setParameter

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: Float)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: Array&lt;Float&gt;)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), value: Boolean)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), texture: [Texture](../-texture/index.md), sampler: [TextureSampler](../-texture-sampler/index.md))

inline helper to provide the name as a null-terminated C string

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Int, valuey: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Float, valuey: Float)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Boolean, valuey: Boolean)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| | |
|---|---|
| valuex | (x component) |
| valuey | (y component) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Int, valuey: Int, valuez: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Float, valuey: Float, valuez: Float)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Boolean, valuey: Boolean, valuez: Boolean)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| | |
|---|---|
| valuex | (x component) |
| valuey | (y component) |
| valuez | (z component) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Int, valuey: Int, valuez: Int, valuew: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Float, valuey: Float, valuez: Float, valuew: Float)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), valuex: Boolean, valuey: Boolean, valuez: Boolean, valuew: Boolean)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| | |
|---|---|
| valuex | (x component) |
| valuey | (y component) |
| valuez | (z component) |
| valuew | (w component) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.FloatElement](-float-element/index.md), values: Array&lt;Float&gt;, offset: Int, count: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.IntElement](-int-element/index.md), values: Array&lt;Int&gt;, offset: Int, count: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.BooleanElement](-boolean-element/index.md), values: Array&lt;Boolean&gt;, offset: Int, count: Int)

Set a uniform array by name

#### Parameters

main

| | |
|---|---|
| name | Name of the parameter array as defined by Material. |
| type | the number of components for each individual parameter |
| values | Array of values to set to the named parameter array. |
| offset | the number of elements in `values` to skip |

#### See also

| |
|---|
| [Material](../-material/has-parameter.md) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.FloatElement](-float-element/index.md), values: Array&lt;Float&gt;, count: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.IntElement](-int-element/index.md), values: Array&lt;Int&gt;, count: Int)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [MaterialInstance.BooleanElement](-boolean-element/index.md), values: Array&lt;Boolean&gt;, count: Int)

Set a uniform array by name

#### Parameters

main

| | |
|---|---|
| name | Name of the parameter array as defined by Material. |
| type | the number of components for each individual parameter |
| values | Array of values to set to the named parameter array. |

#### See also

| |
|---|
| [Material](../-material/has-parameter.md) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [Colors.RgbType](../-colors/-rgb-type/index.md), colorx: Float, colory: Float, colorz: Float)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [Colors.RgbType](../-colors/-rgb-type/index.md), color: Array&lt;Float&gt;)

open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [Colors.RgbaType](../-colors/-rgba-type/index.md), color: Array&lt;Float&gt;)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| |
|---|
| name |
| type |
| color |

[main]\
open fun [setParameter](set-parameter.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html), type: [Colors.RgbaType](../-colors/-rgba-type/index.md), colorx: Float, colory: Float, colorz: Float, colorw: Float)

inline helper to provide the name as a null-terminated C string

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |
| colorw | (w component) |
