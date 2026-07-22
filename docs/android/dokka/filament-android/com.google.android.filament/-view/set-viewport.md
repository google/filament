//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setViewport](set-viewport.md)

# setViewport

[main]\
open fun [setViewport](set-viewport.md)(viewport: [Viewport](../-viewport/index.md))

Specifies the rectangular rendering area. 

 The viewport specifies where the content of the View (i.e. the Scene) is rendered in the render target. The render target is automatically clipped to the Viewport. 

 If you wish subsequent changes to take effect please call this method again in order to propagate the changes down to the native layer. 

#### Parameters

main

| | |
|---|---|
| viewport | The Viewport to render the Scene into. |
