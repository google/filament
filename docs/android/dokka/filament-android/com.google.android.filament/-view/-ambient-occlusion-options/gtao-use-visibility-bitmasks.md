//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[AmbientOcclusionOptions](index.md)/[gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md)

# gtaoUseVisibilityBitmasks

[main]\
open var [gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Enables or disables visibility bitmasks mode. 

Notes that bent normal doesn't work under this mode. Caution: Changing this option at runtime is very expensive as it may trigger a shader re-compilation.
