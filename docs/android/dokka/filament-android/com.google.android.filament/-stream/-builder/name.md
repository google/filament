//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Stream](../index.md)/[Builder](index.md)/[name](name.md)

# name

[main]\
open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Stream.Builder](index.md)

Associate an optional name with this Stream for debugging purposes. 

This method should be avoided in C++ in favor of the `StaticString` overload. It exists primarily for language bindings and dynamic string generation.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| name | A string to identify this Stream |
