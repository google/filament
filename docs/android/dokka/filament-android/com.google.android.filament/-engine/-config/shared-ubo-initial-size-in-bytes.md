//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md)

# sharedUboInitialSizeInBytes

[main]\
open var [sharedUboInitialSizeInBytes](shared-ubo-initial-size-in-bytes.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

The initial size in bytes of the shared uniform buffer used for material instance batching. If the buffer runs out of space during a frame, it will be automatically reallocated with a larger capacity. Setting an appropriate initial size can help avoid runtime reallocations, which can cause a minor performance stutter, at the cost of higher initial memory usage.
