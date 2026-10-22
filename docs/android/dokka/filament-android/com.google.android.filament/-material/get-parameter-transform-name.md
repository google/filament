//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Material](index.md)/[getParameterTransformName](get-parameter-transform-name.md)

# getParameterTransformName

[main]\
open fun [getParameterTransformName](get-parameter-transform-name.md)(samplerName: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [String](https://developer.android.com/reference/kotlin/java/lang/String.html)

Gets the name of the transform field associated for the given sampler parameter. 

In the case where the parameter does not have a transform name field, it will return nullptr.

#### Return

If exists, the transform name value otherwise returns a nullptr.

#### Parameters

main

| | |
|---|---|
| samplerName | the name of the sampler parameter to query. |
