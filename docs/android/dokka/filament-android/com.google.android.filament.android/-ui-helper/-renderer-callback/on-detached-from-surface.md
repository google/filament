//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[UiHelper](../index.md)/[RendererCallback](index.md)/[onDetachedFromSurface](on-detached-from-surface.md)

# onDetachedFromSurface

[main]\
abstract fun [onDetachedFromSurface](on-detached-from-surface.md)()

Called when the surface is going away. After this call `isReadyToRender()` returns false. You MUST have stopped drawing when returning. This is called from detach() or if the surface disappears on its own.
