//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[InstanceBuffer](../index.md)/[Builder](index.md)/[name](name.md)

# name

[main]\
open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [InstanceBuffer.Builder](index.md)

Associate an optional name with this InstanceBuffer for debugging purposes. 

This method should be avoided in C++ in favor of the `StaticString` overload. It is provided primarily for bindings to other languages.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| name | A string to identify this InstanceBuffer |
