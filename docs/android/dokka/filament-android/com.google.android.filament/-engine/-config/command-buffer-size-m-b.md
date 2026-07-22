//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[commandBufferSizeMB](command-buffer-size-m-b.md)

# commandBufferSizeMB

[main]\
open var [commandBufferSizeMB](command-buffer-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Size in MiB of the low-level command buffer arena. Each new command buffer is allocated from here. If this buffer is too small the program might terminate or rendering errors might occur. This is typically set to minCommandBufferSizeMB * 3, so that up to 3 frames can be batched-up at once. This value affects the application's memory usage.
