//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[UiHelper](index.md)/[attachTo](attach-to.md)

# attachTo

[main]\
open fun [attachTo](attach-to.md)(view: SurfaceView)

Associate UiHelper with a SurfaceView. As soon as SurfaceView is ready (i.e. has a Surface), we'll create the EGL resources needed, and call user callbacks if needed.

[main]\
open fun [attachTo](attach-to.md)(view: TextureView)

Associate UiHelper with a TextureView. As soon as TextureView is ready (i.e. has a buffer), we'll create the EGL resources needed, and call user callbacks if needed.

[main]\
open fun [attachTo](attach-to.md)(holder: SurfaceHolder)

Associate UiHelper with a SurfaceHolder. As soon as a Surface is created, we'll create the EGL resources needed, and call user callbacks if needed.
