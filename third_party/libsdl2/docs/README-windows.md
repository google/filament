Windows
================================================================================

================================================================================
OpenGL ES 2.x support
================================================================================

SDL has support for OpenGL ES 2.x under Windows via two alternative 
implementations. 
The most straightforward method consists in running your app in a system with 
a graphic card paired with a relatively recent (as of November of 2013) driver 
which supports the WGL_EXT_create_context_es2_profile extension. Vendors known 
to ship said extension on Windows currently include nVidia and Intel.

The other method involves using the ANGLE library (https://code.google.com/p/angleproject/)
If an OpenGL ES 2.x context is requested and no WGL_EXT_create_context_es2_profile
extension is found, SDL will try to load the libEGL.dll library provided by
ANGLE.
To obtain the ANGLE binaries, you can either compile from source from
https://chromium.googlesource.com/angle/angle or copy the relevant binaries from
a recent Chrome/Chromium install for Windows. The files you need are:
    
    * libEGL.dll
    * libGLESv2.dll
    * d3dcompiler_46.dll (supports Windows Vista or later, better shader compiler)
    or...
    * d3dcompiler_43.dll (supports Windows XP or later)
    
If you compile ANGLE from source, you can configure it so it does not need the
d3dcompiler_* DLL at all (for details on this, see their documentation). 
However, by default SDL will try to preload the d3dcompiler_46.dll to
comply with ANGLE's requirements. If you wish SDL to preload d3dcompiler_43.dll (to
support Windows XP) or to skip this step at all, you can use the 
SDL_HINT_VIDEO_WIN_D3DCOMPILER hint (see SDL_hints.h for more details).

Known Bugs:
    
    * SDL_GL_SetSwapInterval is currently a no op when using ANGLE. It appears
      that there's a bug in the library which prevents the window contents from
      refreshing if this is set to anything other than the default value.
     
Vulkan Surface Support
==============

Support for creating Vulkan surfaces is configured on by default. To disable it change the value of `SDL_VIDEO_VULKAN` to 0 in `SDL_config_windows.h`. You must install the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) in order to use Vulkan graphics in your application.
