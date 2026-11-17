//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)/[getIndex](get-index.md)

# getIndex

[main]\
open fun [getIndex](get-index.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Use carefully, several entities can have the same index across time (never simultaneously while alive or pending EBR reclamation).

#### Return

The 17-bit entity index.

#### Parameters

main

| | |
|---|---|
| e | The entity to query. |
