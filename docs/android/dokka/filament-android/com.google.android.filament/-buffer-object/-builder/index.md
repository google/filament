//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[BufferObject](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [bindingType](binding-type.md) | [main]<br>open fun [bindingType](binding-type.md)(bindingType: [BufferObject.BindingType](../-binding-type/index.md)): [BufferObject.Builder](index.md)<br>The binding type for this buffer object. |
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [BufferObject](../index.md)<br>Creates the BufferObject and returns a pointer to it. |
| [name](name.md) | [main]<br>open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [BufferObject.Builder](index.md)<br>Associate an optional name with this BufferObject for debugging purposes. |
| [size](size.md) | [main]<br>open fun [size](size.md)(byteCount: Int): [BufferObject.Builder](index.md)<br>Size of the buffer in bytes. |
