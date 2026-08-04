//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[setPaused](set-paused.md)

# setPaused

[main]\
open fun [setPaused](set-paused.md)(paused: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Pause or resume the rendering thread. 

Warning: This is an experimental API. In particular, note the following caveats. 

- Buffer callbacks will never be called as long as the rendering thread is paused. Do not rely on a buffer callback to unpause the thread.
- While the rendering thread is paused, rendering commands will continue to be queued until the buffer limit is reached. When the limit is reached, the program will abort.
