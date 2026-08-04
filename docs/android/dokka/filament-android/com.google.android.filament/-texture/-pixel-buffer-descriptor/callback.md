//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[PixelBufferDescriptor](index.md)/[callback](callback.md)

# callback

[main]\
open var [callback](callback.md): [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html)

Callback used to destroy the buffer data. 

 Guarantees: 

- Called on the main filament thread.

 Limitations: 

- Must be lightweight.
- Must not call filament APIs.
