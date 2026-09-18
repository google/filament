//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setViewport](set-viewport.md)

# setViewport

[main]\
open fun [setViewport](set-viewport.md)(viewport: [Viewport](../-viewport/index.md))

Sets the rectangular region to render to. 

The viewport specifies where the content of the View (i.e. the Scene) is rendered in the render target. The Render target is automatically clipped to the Viewport.

#### Parameters

main

| | |
|---|---|
| viewport | The Viewport to render the Scene into. The Viewport is a value-type, it is therefore copied. The parameter can be discarded after this call returns. |
