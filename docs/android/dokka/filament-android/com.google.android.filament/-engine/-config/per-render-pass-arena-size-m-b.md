//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md)

# perRenderPassArenaSizeMB

[main]\
open var [perRenderPassArenaSizeMB](per-render-pass-arena-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Size in MiB of the per-frame data arena. This is the main arena used for allocations when preparing a frame. e.g.: Froxel data and high-level commands are allocated from this arena. If this size is too small, the program will abort on debug builds and have undefined behavior otherwise. This value affects the application's memory usage.
