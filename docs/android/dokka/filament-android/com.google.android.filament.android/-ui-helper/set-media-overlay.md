//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[UiHelper](index.md)/[setMediaOverlay](set-media-overlay.md)

# setMediaOverlay

[main]\
open fun [setMediaOverlay](set-media-overlay.md)(overlay: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Controls whether the surface of the SurfaceView used as a render target should be positioned above other surfaces but below the activity's surface. This property only has an effect when used in combination with [setOpaque(false)](set-opaque.md) and does not affect TextureView targets. Must be called before calling [attachTo](attach-to.md) or [attachTo](attach-to.md). Has no effect when using [attachTo](attach-to.md).

#### Parameters

main

| | |
|---|---|
| overlay | Indicates whether the render target should be rendered below the activity's surface when transparent. |
