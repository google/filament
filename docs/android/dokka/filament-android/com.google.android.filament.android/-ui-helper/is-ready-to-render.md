//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[UiHelper](index.md)/[isReadyToRender](is-ready-to-render.md)

# isReadyToRender

[main]\
open fun [isReadyToRender](is-ready-to-render.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Checks whether we are ready to render into the attached surface. Using OpenGL ES when this returns true, will result in drawing commands being lost, HOWEVER, GLES state will be preserved. This is useful to initialize the engine.

#### Return

true: rendering is possible, false: rendering is not possible.
