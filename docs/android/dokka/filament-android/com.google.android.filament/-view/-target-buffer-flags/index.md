//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[TargetBufferFlags](index.md)

# TargetBufferFlags

[main]\
enum [TargetBufferFlags](index.md)

Used to select buffers.

## Entries

| | |
|---|---|
| [COLOR0](-c-o-l-o-r0/index.md) | [main]<br>[COLOR0](-c-o-l-o-r0/index.md)<br>Color 0 buffer selected. |
| [COLOR1](-c-o-l-o-r1/index.md) | [main]<br>[COLOR1](-c-o-l-o-r1/index.md)<br>Color 1 buffer selected. |
| [COLOR2](-c-o-l-o-r2/index.md) | [main]<br>[COLOR2](-c-o-l-o-r2/index.md)<br>Color 2 buffer selected. |
| [COLOR3](-c-o-l-o-r3/index.md) | [main]<br>[COLOR3](-c-o-l-o-r3/index.md)<br>Color 3 buffer selected. |
| [DEPTH](-d-e-p-t-h/index.md) | [main]<br>[DEPTH](-d-e-p-t-h/index.md)<br>Depth buffer selected. |
| [STENCIL](-s-t-e-n-c-i-l/index.md) | [main]<br>[STENCIL](-s-t-e-n-c-i-l/index.md)<br>Stencil buffer selected. |

## Properties

| Name | Summary |
|---|---|
| [ALL](-a-l-l.md) | [main]<br>open var [ALL](-a-l-l.md): [EnumSet](https://developer.android.com/reference/kotlin/java/util/EnumSet.html)&lt;[View.TargetBufferFlags](index.md)&gt;<br>All buffers are selected. |
| [ALL_COLOR](-a-l-l_-c-o-l-o-r.md) | [main]<br>open var [ALL_COLOR](-a-l-l_-c-o-l-o-r.md): [EnumSet](https://developer.android.com/reference/kotlin/java/util/EnumSet.html)&lt;[View.TargetBufferFlags](index.md)&gt; |
| [DEPTH_STENCIL](-d-e-p-t-h_-s-t-e-n-c-i-l.md) | [main]<br>open var [DEPTH_STENCIL](-d-e-p-t-h_-s-t-e-n-c-i-l.md): [EnumSet](https://developer.android.com/reference/kotlin/java/util/EnumSet.html)&lt;[View.TargetBufferFlags](index.md)&gt;<br>Depth and stencil buffer selected. |
| [NONE](-n-o-n-e.md) | [main]<br>open var [NONE](-n-o-n-e.md): [EnumSet](https://developer.android.com/reference/kotlin/java/util/EnumSet.html)&lt;[View.TargetBufferFlags](index.md)&gt; |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [View.TargetBufferFlags](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[View.TargetBufferFlags](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
