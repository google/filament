//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[UiHelper](../index.md)/[RendererCallback](index.md)

# RendererCallback

interface [RendererCallback](index.md)

Interface used to know when the native surface is created, destroyed or resized.

#### See also

| |
|---|
| [setRenderCallback(RendererCallback)](../set-render-callback.md) |

## Functions

| Name | Summary |
|---|---|
| [onDetachedFromSurface](on-detached-from-surface.md) | [main]<br>abstract fun [onDetachedFromSurface](on-detached-from-surface.md)()<br>Called when the surface is going away. |
| [onNativeWindowChanged](on-native-window-changed.md) | [main]<br>abstract fun [onNativeWindowChanged](on-native-window-changed.md)(surface: Surface)<br>Called when the underlying native window has changed. |
| [onResized](on-resized.md) | [main]<br>abstract fun [onResized](on-resized.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Called when the underlying native window has been resized. |
