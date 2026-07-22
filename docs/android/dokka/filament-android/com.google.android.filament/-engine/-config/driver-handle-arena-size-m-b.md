//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md)

# driverHandleArenaSizeMB

[main]\
open var [driverHandleArenaSizeMB](driver-handle-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Size in MiB of the backend's handle arena. Backends will fallback to slower heap-based allocations when running out of space and log this condition. If 0, then the default value for the given platform is used This value affects the application's memory usage.
