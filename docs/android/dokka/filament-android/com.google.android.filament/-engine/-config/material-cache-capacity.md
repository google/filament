//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[materialCacheCapacity](material-cache-capacity.md)

# materialCacheCapacity

[main]\
open var [materialCacheCapacity](material-cache-capacity.md): Long

Capacity of the LRU cache for material definitions. 

A value of 0 indicates that definitions will be destroyed immediately when they are no longer referenced by any material instances or scenes. A value greater than 0 defines the maximum number of unreferenced definitions to keep alive to avoid re-compilation.
