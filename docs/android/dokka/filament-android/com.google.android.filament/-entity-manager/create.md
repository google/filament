//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)/[create](create.md)

# create

[main]\
open fun [create](create.md)(entities: Array&lt;Int&gt;)

Allocates and creates a batch of new or recycled Entities.

#### Parameters

main

| | |
|---|---|
| entities | Output array receiving the populated Entity IDs. |

[main]\
open fun [create](create.md)(): Int

Allocates and creates a single new or recycled Entity.

#### Return

The populated Entity ID, or Entity.isNull() upon allocation failure.
