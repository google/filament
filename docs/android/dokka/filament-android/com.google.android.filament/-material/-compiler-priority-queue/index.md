//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[CompilerPriorityQueue](index.md)

# CompilerPriorityQueue

[main]\
enum [CompilerPriorityQueue](index.md)

Shader compiler priority queue On platforms which support parallel shader compilation, compilation requests will be processed in order of priority, then insertion order. See [compile](../compile.md).

## Entries

| | |
|---|---|
| [CRITICAL](-c-r-i-t-i-c-a-l/index.md) | [main]<br>[CRITICAL](-c-r-i-t-i-c-a-l/index.md)<br>We need this program NOW. When passed as an argument to [compile](../compile.md), if the platform doesn't support parallel compilation, but does support amortized shader compilation, the given shader program will be synchronously compiled. |
| [HIGH](-h-i-g-h/index.md) | [main]<br>[HIGH](-h-i-g-h/index.md)<br>We will need this program soon. |
| [LOW](-l-o-w/index.md) | [main]<br>[LOW](-l-o-w/index.md)<br>We will need this program eventually. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.CompilerPriorityQueue](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.CompilerPriorityQueue](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
