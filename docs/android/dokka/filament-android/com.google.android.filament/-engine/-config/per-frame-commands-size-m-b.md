//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[perFrameCommandsSizeMB](per-frame-commands-size-m-b.md)

# perFrameCommandsSizeMB

[main]\
open var [perFrameCommandsSizeMB](per-frame-commands-size-m-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Size in MiB of the per-frame high level command buffer. This buffer is related to the number of draw calls achievable within a frame, if it is too small, the program will abort on debug builds and have undefined behavior otherwise. It is allocated from the 'per-render-pass arena' above. Make sure that at least 1 MiB is left in the per-render-pass arena when deciding the size of this buffer. This value does not affect the application's memory usage.
