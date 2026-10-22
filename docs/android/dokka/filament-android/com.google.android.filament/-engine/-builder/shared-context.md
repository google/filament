//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Builder](index.md)/[sharedContext](shared-context.md)

# sharedContext

[main]\
open fun [sharedContext](shared-context.md)(sharedContext: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Engine.Builder](index.md)

Sets a sharedContext for the Engine.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| sharedContext | A platform-dependant OpenGL context used as a shared context when creating filament's internal context. On Android this parameter **must be** an instance of android.opengl.EGLContext. |
