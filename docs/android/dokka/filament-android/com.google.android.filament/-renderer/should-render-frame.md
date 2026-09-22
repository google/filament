//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[shouldRenderFrame](should-render-frame.md)

# shouldRenderFrame

[main]\
open fun [shouldRenderFrame](should-render-frame.md)(): Boolean

Returns true if the current frame should be rendered. 

This is a convenience method that returns the same value as beginFrame().

This method will return false once a backend exception has been delivered to the main thread.

#### Return

*false* the current frame should be skipped, or an unrecoverable backend exception has occurred. *true* the current frame can be rendered

#### See also

| |
|---|
| beginFrame |
