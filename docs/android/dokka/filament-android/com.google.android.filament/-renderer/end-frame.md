//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[endFrame](end-frame.md)

# endFrame

[main]\
open fun [endFrame](end-frame.md)()

Finishes the current frame and schedules it for display. 

endFrame() schedules the current frame to be displayed on the Renderer's window.

All calls to render() must happen *before* endFrame(). endFrame() must be called if beginFrame() returned true, otherwise, endFrame() must not be called unless the caller ignored beginFrame()'s return value.

#### See also

| |
|---|
| beginFrame |
