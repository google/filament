//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)/[getMaxEntityCount](get-max-entity-count.md)

# getMaxEntityCount

[main]\
open fun [getMaxEntityCount](get-max-entity-count.md)(): Int

Retrieves the maximum theoretical upper bound of entities that can exist concurrently. 

Factored around reserved index slots and 2^GENERATION_SHIFT limits.

#### Return

The maximum available 32-bit Entity identity limit.
