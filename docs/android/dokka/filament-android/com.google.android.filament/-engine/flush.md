//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[flush](flush.md)

# flush

[main]\
open fun [flush](flush.md)()

Kicks the hardware thread (e.g. the OpenGL, Vulkan or Metal thread) but does not wait for commands to be either executed or the hardware finished. 

This is typically used after creating a lot of objects to start draining the command queue which has a limited size.
