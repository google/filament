//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[endFrame](end-frame.md)

# endFrame

[main]\
open fun [endFrame](end-frame.md)()

Finishes the current frame and schedules it for display. 

`endFrame()` schedules the current frame to be displayed on the `Renderer`'s window. 

All calls to render() must happen **before** endFrame().

#### See also

| |
|---|
| [beginFrame](begin-frame.md) |
| [render](render.md) |

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if the backend thread encountered an unrecoverable error, or if called again after a backend exception was already thrown. |
