//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[minCommandBufferSizeMB](min-command-buffer-size-m-b.md)

# minCommandBufferSizeMB

[main]\
open var [minCommandBufferSizeMB](min-command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Minimum size in MiB of a low-level command buffer. This is how much space is guaranteed to be available for low-level commands when a new buffer is allocated. If this is too small, the engine might have to stall to wait for more space to become available, this situation is logged. This value does not affect the application's memory usage.
