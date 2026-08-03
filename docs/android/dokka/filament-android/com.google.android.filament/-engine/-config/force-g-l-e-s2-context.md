//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[forceGLES2Context](force-g-l-e-s2-context.md)

# forceGLES2Context

[main]\
open var [forceGLES2Context](force-g-l-e-s2-context.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

When the OpenGL ES backend is used, setting this value to true will force a GLES2.0 context if supported by the Platform, or if not, will have the backend pretend it's a GLES2 context. Ignored on other backends.
