//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[programCacheCapacity](program-cache-capacity.md)

# programCacheCapacity

[main]\
open var [programCacheCapacity](program-cache-capacity.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Capacity of the LRU cache for program specializations. 

Similar to materialCacheCapacity, but applies to the underlying shader programs generated for materials. A value of 0 means immediate destruction of unreferenced programs. A positive value caches up to that number of programs.
