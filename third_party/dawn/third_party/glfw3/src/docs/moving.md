# Moving from GLFW 2 to 3 {#moving_guide}

[TOC]

This is a transition guide for moving from GLFW 2 to 3.  It describes what has
changed or been removed, but does _not_ include
[new features](@ref news) unless they are required when moving an existing code
base onto the new API.  For example, the new multi-monitor functions are
required to create full screen windows with GLFW 3.


## Changed and removed features {#moving_removed}

### Renamed library and header file {#moving_renamed_files}

The GLFW 3 header is named @ref glfw3.h and moved to the `GLFW` directory, to
avoid collisions with the headers of other major versions.  Similarly, the GLFW
3 library is named `glfw3,` except when it's installed as a shared library on
Unix-like systems, where it uses the [soname][] `libglfw.so.3`.

[soname]: https://en.wikipedia.org/wiki/soname

__Old syntax__
```c
#include <GL/glfw.h>
```

__New syntax__
```c
#include <GLFW/glfw3.h>
```


### Removal of threading functions {#moving_threads}

The threading functions have been removed, including the per-thread sleep
function.  They were fairly primitive, under-used, poorly integrated and took
time away from the focus of GLFW (i.e.  context, input and window).  There are
better threading libraries available and native threading support is available
in both [C++11][] and [C11][], both of which are gaining traction.

[C++11]: https://en.cppreference.com/w/cpp/thread
[C11]: https://en.cppreference.com/w/c/thread

If you wish to use the C++11 or C11 facilities but your compiler doesn't yet
support them, see the [TinyThread++][] and [TinyCThread][] projects created by
the original author of GLFW.  These libraries implement a usable subset of the
threading APIs in C++11 and C11, and in fact some GLFW 3 test programs use
TinyCThread.

[TinyThread++]: https://gitorious.org/tinythread/tinythreadpp
[TinyCThread]: https://github.com/tinycthread/tinycthread

However, GLFW 3 has better support for _use from multiple threads_ than GLFW
2 had.  Contexts can be made current on any thread, although only a single
thread at a time, and the documentation explicitly states which functions may be
used from any thread and which must only be used from the main thread.

__Removed functions__
> `glfwSleep`, `glfwCreateThread`, `glfwDestroyThread`, `glfwWaitThread`,
> `glfwGetThreadID`, `glfwCreateMutex`, `glfwDestroyMutex`, `glfwLockMutex`,
> `glfwUnlockMutex`, `glfwCreateCond`, `glfwDestroyCond`, `glfwWaitCond`,
> `glfwSignalCond`, `glfwBroadcastCond` and `glfwGetNumberOfProcessors`.

__Removed types__
> `GLFWthreadfun`


### Removal of image and texture loading {#moving_image}

The image and texture loading functions have been removed.  They only supported
the Targa image format, making them mostly useful for beginner level examples.
To become of sufficiently high quality to warrant keeping them in GLFW 3, they
would need not only to support other formats, but also modern extensions to
OpenGL texturing.  This would either add a number of external
dependencies (libjpeg, libpng, etc.), or force GLFW to ship with inline versions
of these libraries.

As there already are libraries doing this, it is unnecessary both to duplicate
the work and to tie the duplicate to GLFW.  The resulting library would also be
platform-independent, as both OpenGL and stdio are available wherever GLFW is.

__Removed functions__
> `glfwReadImage`, `glfwReadMemoryImage`, `glfwFreeImage`, `glfwLoadTexture2D`,
> `glfwLoadMemoryTexture2D` and `glfwLoadTextureImage2D`.


### Removal of GLFWCALL macro {#moving_stdcall}

The `GLFWCALL` macro, which made callback functions use [\_\_stdcall][stdcall]
on Windows, has been removed.  GLFW is written in C, not Pascal.  Removing this
macro means there's one less thing for application programmers to remember, i.e.
the requirement to mark all callback functions with `GLFWCALL`.  It also
simplifies the creation of DLLs and DLL link libraries, as there's no need to
explicitly disable `@n` entry point suffixes.

[stdcall]: https://msdn.microsoft.com/en-us/library/zxk0tw93.aspx

__Old syntax__
```c
void GLFWCALL callback_function(...);
```

__New syntax__
```c
void callback_function(...);
```


### Window handle parameters {#moving_window_handles}

Because GLFW 3 supports multiple windows, window handle parameters have been
added to all window-related GLFW functions and callbacks.  The handle of
a newly created window is returned by @ref glfwCreateWindow (formerly
`glfwOpenWindow`).  Window handles are pointers to the
[opaque][opaque-type] type @ref GLFWwindow.

[opaque-type]: https://en.wikipedia.org/wiki/Opaque_data_type

__Old syntax__
```c
glfwSetWindowTitle("New Window Title");
```

__New syntax__
```c
glfwSetWindowTitle(window, "New Window Title");
```


### Explicit monitor selection {#moving_monitor}

GLFW 3 provides support for multiple monitors.  To request a full screen mode window,
instead of passing `GLFW_FULLSCREEN` you specify which monitor you wish the
window to use.  The @ref glfwGetPrimaryMonitor function returns the monitor that
GLFW 2 would have selected, but there are many other
[monitor functions](@ref monitor_guide).  Monitor handles are pointers to the
[opaque][opaque-type] type @ref GLFWmonitor.

__Old basic full screen__
```c
glfwOpenWindow(640, 480, 8, 8, 8, 0, 24, 0, GLFW_FULLSCREEN);
```

__New basic full screen__
```c
window = glfwCreateWindow(640, 480, "My Window", glfwGetPrimaryMonitor(), NULL);
```

@note The framebuffer bit depth parameters of `glfwOpenWindow` have been turned
into [window hints](@ref window_hints), but as they have been given
[sane defaults](@ref window_hints_values) you rarely need to set these hints.


### Removal of automatic event polling {#moving_autopoll}

GLFW 3 does not automatically poll for events in @ref glfwSwapBuffers, meaning
you need to call @ref glfwPollEvents or @ref glfwWaitEvents yourself.  Unlike
buffer swap, which acts on a single window, the event processing functions act
on all windows at once.

__Old basic main loop__
```c
while (...)
{
    // Process input
    // Render output
    glfwSwapBuffers();
}
```

__New basic main loop__
```c
while (...)
{
    // Process input
    // Render output
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```


### Explicit context management {#moving_context}

Each GLFW 3 window has its own OpenGL context and only you, the application
programmer, can know which context should be current on which thread at any
given time.  Therefore, GLFW 3 leaves that decision to you.

This means that you need to call @ref glfwMakeContextCurrent after creating
a window before you can call any OpenGL functions.


### Separation of window and framebuffer sizes {#moving_hidpi}

Window positions and sizes now use screen coordinates, which may not be the same
as pixels on machines with high-DPI monitors.  This is important as OpenGL uses
pixels, not screen coordinates.  For example, the rectangle specified with
`glViewport` needs to use pixels.  Therefore, framebuffer size functions have
been added.  You can retrieve the size of the framebuffer of a window with @ref
glfwGetFramebufferSize function.  A framebuffer size callback has also been
added, which can be set with @ref glfwSetFramebufferSizeCallback.

__Old basic viewport setup__
```c
glfwGetWindowSize(&width, &height);
glViewport(0, 0, width, height);
```

__New basic viewport setup__
```c
glfwGetFramebufferSize(window, &width, &height);
glViewport(0, 0, width, height);
```


### Window closing changes {#moving_window_close}

The `GLFW_OPENED` window parameter has been removed.  As long as the window has
not been destroyed, whether through @ref glfwDestroyWindow or @ref
glfwTerminate, the window is "open".

A user attempting to close a window is now just an event like any other.  Unlike
GLFW 2, windows and contexts created with GLFW 3 will never be destroyed unless
you choose them to be.  Each window now has a close flag that is set to
`GLFW_TRUE` when the user attempts to close that window.  By default, nothing else
happens and the window stays visible.  It is then up to you to either destroy
the window, take some other action or ignore the request.

You can query the close flag at any time with @ref glfwWindowShouldClose and set
it at any time with @ref glfwSetWindowShouldClose.

__Old basic main loop__
```c
while (glfwGetWindowParam(GLFW_OPENED))
{
    ...
}
```

__New basic main loop__
```c
while (!glfwWindowShouldClose(window))
{
    ...
}
```

The close callback no longer returns a value.  Instead, it is called after the
close flag has been set, so it can optionally override its value, before
event processing completes.  You may however not call @ref glfwDestroyWindow
from the close callback (or any other window related callback).

__Old syntax__
```c
int GLFWCALL window_close_callback(void);
```

__New syntax__
```c
void window_close_callback(GLFWwindow* window);
```

@note GLFW never clears the close flag to `GLFW_FALSE`, meaning you can use it
for other reasons to close the window as well, for example the user choosing
Quit from an in-game menu.


### Persistent window hints {#moving_hints}

The `glfwOpenWindowHint` function has been renamed to @ref glfwWindowHint.

Window hints are no longer reset to their default values on window creation, but
instead retain their values until modified by @ref glfwWindowHint or @ref
glfwDefaultWindowHints, or until the library is terminated and re-initialized.


### Video mode enumeration {#moving_video_modes}

Video mode enumeration is now per-monitor.  The @ref glfwGetVideoModes function
now returns all available modes for a specific monitor instead of requiring you
to guess how large an array you need.  The `glfwGetDesktopMode` function, which
had poorly defined behavior, has been replaced by @ref glfwGetVideoMode, which
returns the current mode of a monitor.


### Removal of character actions {#moving_char_up}

The action parameter of the [character callback](@ref GLFWcharfun) has been
removed.  This was an artefact of the origin of GLFW, i.e. being developed in
English by a Swede.  However, many keyboard layouts require more than one key to
produce characters with diacritical marks. Even the Swedish keyboard layout
requires this for uncommon cases like ü.

__Old syntax__
```c
void GLFWCALL character_callback(int character, int action);
```

__New syntax__
```c
void character_callback(GLFWwindow* window, int character);
```


### Cursor position changes {#moving_cursorpos}

The `glfwGetMousePos` function has been renamed to @ref glfwGetCursorPos,
`glfwSetMousePos` to @ref glfwSetCursorPos and `glfwSetMousePosCallback` to @ref
glfwSetCursorPosCallback.

The cursor position is now `double` instead of `int`, both for the direct
functions and for the callback.  Some platforms can provide sub-pixel cursor
movement and this data is now passed on to the application where available.  On
platforms where this is not provided, the decimal part is zero.

GLFW 3 only allows you to position the cursor within a window using @ref
glfwSetCursorPos (formerly `glfwSetMousePos`) when that window is active.
Unless the window is active, the function fails silently.


### Wheel position replaced by scroll offsets {#moving_wheel}

The `glfwGetMouseWheel` function has been removed.  Scrolling is the input of
offsets and has no absolute position.  The mouse wheel callback has been
replaced by a [scroll callback](@ref GLFWscrollfun) that receives
two-dimensional floating point scroll offsets.  This allows you to receive
precise scroll data from for example modern touchpads.

__Old syntax__
```c
void GLFWCALL mouse_wheel_callback(int position);
```

__New syntax__
```c
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
```

__Removed functions__
> `glfwGetMouseWheel`


### Key repeat action {#moving_repeat}

The `GLFW_KEY_REPEAT` enable has been removed and key repeat is always enabled
for both keys and characters.  A new key action, `GLFW_REPEAT`, has been added
to allow the [key callback](@ref GLFWkeyfun) to distinguish an initial key press
from a repeat.  Note that @ref glfwGetKey still returns only `GLFW_PRESS` or
`GLFW_RELEASE`.


### Physical key input {#moving_keys}

GLFW 3 key tokens map to physical keys, unlike in GLFW 2 where they mapped to
the values generated by the current keyboard layout.  The tokens are named
according to the values they would have in the standard US layout, but this
is only a convenience, as most programmers are assumed to know that layout.
This means that (for example) `GLFW_KEY_LEFT_BRACKET` is always a single key and
is the same key in the same place regardless of what keyboard layouts the users
of your program have.

The key input facility was never meant for text input, although using it that
way worked slightly better in GLFW 2.  If you were using it to input text, you
should be using the character callback instead, on both GLFW 2 and 3.  This will
give you the characters being input, as opposed to the keys being pressed.

GLFW 3 has key tokens for all keys on a standard 105 key keyboard, so instead of
having to remember whether to check for `a` or `A`, you now check for
@ref GLFW_KEY_A.


### Joystick function changes {#moving_joystick}

The `glfwGetJoystickPos` function has been renamed to @ref glfwGetJoystickAxes.

The `glfwGetJoystickParam` function and the `GLFW_PRESENT`, `GLFW_AXES` and
`GLFW_BUTTONS` tokens have been replaced by the @ref glfwJoystickPresent
function as well as axis and button counts returned by the @ref
glfwGetJoystickAxes and @ref glfwGetJoystickButtons functions.


### Win32 MBCS support {#moving_mbcs}

The Win32 port of GLFW 3 will not compile in [MBCS mode][MBCS].  However,
because the use of the Unicode version of the Win32 API doesn't affect the
process as a whole, but only those windows created using it, it's perfectly
possible to call MBCS functions from other parts of the same application.
Therefore, even if an application using GLFW has MBCS mode code, there's no need
for GLFW itself to support it.

[MBCS]: https://msdn.microsoft.com/en-us/library/5z097dxa.aspx


### Support for versions of Windows older than XP {#moving_windows}

All explicit support for version of Windows older than XP has been removed.
There is no code that actively prevents GLFW 3 from running on these earlier
versions, but it uses Win32 functions that those versions lack.

Windows XP was released in 2001, and by now (January 2015) it has not only
replaced almost all earlier versions of Windows, but is itself rapidly being
replaced by Windows 7 and 8.  The MSDN library doesn't even provide
documentation for version older than Windows 2000, making it difficult to
maintain compatibility with these versions even if it was deemed worth the
effort.

The Win32 API has also not stood still, and GLFW 3 uses many functions only
present on Windows XP or later.  Even supporting an OS as new as XP (new
from the perspective of GLFW 2, which still supports Windows 95) requires
runtime checking for a number of functions that are present only on modern
version of Windows.


### Capture of system-wide hotkeys {#moving_syskeys}

The ability to disable and capture system-wide hotkeys like Alt+Tab has been
removed.  Modern applications, whether they're games, scientific visualisations
or something else, are nowadays expected to be good desktop citizens and allow
these hotkeys to function even when running in full screen mode.


### Automatic termination {#moving_terminate}

GLFW 3 does not register @ref glfwTerminate with `atexit` at initialization,
because `exit` calls registered functions from the calling thread and while it
is permitted to call `exit` from any thread, @ref glfwTerminate must only be
called from the main thread.

To release all resources allocated by GLFW, you should call @ref glfwTerminate
yourself, from the main thread, before the program terminates.  Note that this
destroys all windows not already destroyed with @ref glfwDestroyWindow,
invalidating any window handles you may still have.


### GLU header inclusion {#moving_glu}

GLFW 3 does not by default include the GLU header and GLU itself has been
deprecated by [Khronos][].  __New projects should not use GLU__, but if you need
it for legacy code that has been moved to GLFW 3, you can request that the GLFW
header includes it by defining @ref GLFW_INCLUDE_GLU before the inclusion of the
GLFW header.

[Khronos]: https://en.wikipedia.org/wiki/Khronos_Group

__Old syntax__
```c
#include <GL/glfw.h>
```

__New syntax__
```c
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
```

There are many libraries that offer replacements for the functionality offered
by GLU.  For the matrix helper functions, see math libraries like [GLM][] (for
C++), [linmath.h][] (for C) and others.  For the tessellation functions, see for
example [libtess2][].

[GLM]: https://github.com/g-truc/glm
[linmath.h]: https://github.com/datenwolf/linmath.h
[libtess2]: https://github.com/memononen/libtess2


## Name change tables {#moving_tables}


### Renamed functions {#moving_renamed_functions}

| GLFW 2                      | GLFW 3                        | Notes |
| --------------------------- | ----------------------------- | ----- |
| `glfwOpenWindow`            | @ref glfwCreateWindow         | All channel bit depths are now hints
| `glfwCloseWindow`           | @ref glfwDestroyWindow        |       |
| `glfwOpenWindowHint`        | @ref glfwWindowHint           | Now accepts all `GLFW_*_BITS` tokens |
| `glfwEnable`                | @ref glfwSetInputMode         |       |
| `glfwDisable`               | @ref glfwSetInputMode         |       |
| `glfwGetMousePos`           | @ref glfwGetCursorPos         |       |
| `glfwSetMousePos`           | @ref glfwSetCursorPos         |       |
| `glfwSetMousePosCallback`   | @ref glfwSetCursorPosCallback |       |
| `glfwSetMouseWheelCallback` | @ref glfwSetScrollCallback    | Accepts two-dimensional scroll offsets as doubles |
| `glfwGetJoystickPos`        | @ref glfwGetJoystickAxes      |       |
| `glfwGetWindowParam`        | @ref glfwGetWindowAttrib      |       |
| `glfwGetGLVersion`          | @ref glfwGetWindowAttrib      | Use `GLFW_CONTEXT_VERSION_MAJOR`, `GLFW_CONTEXT_VERSION_MINOR` and `GLFW_CONTEXT_REVISION` |
| `glfwGetDesktopMode`        | @ref glfwGetVideoMode         | Returns the current mode of a monitor |
| `glfwGetJoystickParam`      | @ref glfwJoystickPresent      | The axis and button counts are provided by @ref glfwGetJoystickAxes and @ref glfwGetJoystickButtons |


### Renamed types {#moving_renamed_types}

| GLFW 2              | GLFW 3                | Notes |
| ------------------- | --------------------- |       |
| `GLFWmousewheelfun` | @ref GLFWscrollfun    |       |
| `GLFWmouseposfun`   | @ref GLFWcursorposfun |       |


### Renamed tokens {#moving_renamed_tokens}

| GLFW 2                      | GLFW 3                       | Notes |
| --------------------------- | ---------------------------- | ----- |
| `GLFW_OPENGL_VERSION_MAJOR` | `GLFW_CONTEXT_VERSION_MAJOR` | Renamed as it applies to OpenGL ES as well |
| `GLFW_OPENGL_VERSION_MINOR` | `GLFW_CONTEXT_VERSION_MINOR` | Renamed as it applies to OpenGL ES as well |
| `GLFW_FSAA_SAMPLES`         | `GLFW_SAMPLES`               | Renamed to match the OpenGL API |
| `GLFW_ACTIVE`               | `GLFW_FOCUSED`               | Renamed to match the window focus callback |
| `GLFW_WINDOW_NO_RESIZE`     | `GLFW_RESIZABLE`             | The default has been inverted |
| `GLFW_MOUSE_CURSOR`         | `GLFW_CURSOR`                | Used with @ref glfwSetInputMode |
| `GLFW_KEY_ESC`              | `GLFW_KEY_ESCAPE`            |       |
| `GLFW_KEY_DEL`              | `GLFW_KEY_DELETE`            |       |
| `GLFW_KEY_PAGEUP`           | `GLFW_KEY_PAGE_UP`           |       |
| `GLFW_KEY_PAGEDOWN`         | `GLFW_KEY_PAGE_DOWN`         |       |
| `GLFW_KEY_KP_NUM_LOCK`      | `GLFW_KEY_NUM_LOCK`          |       |
| `GLFW_KEY_LCTRL`            | `GLFW_KEY_LEFT_CONTROL`      |       |
| `GLFW_KEY_LSHIFT`           | `GLFW_KEY_LEFT_SHIFT`        |       |
| `GLFW_KEY_LALT`             | `GLFW_KEY_LEFT_ALT`          |       |
| `GLFW_KEY_LSUPER`           | `GLFW_KEY_LEFT_SUPER`        |       |
| `GLFW_KEY_RCTRL`            | `GLFW_KEY_RIGHT_CONTROL`     |       |
| `GLFW_KEY_RSHIFT`           | `GLFW_KEY_RIGHT_SHIFT`       |       |
| `GLFW_KEY_RALT`             | `GLFW_KEY_RIGHT_ALT`         |       |
| `GLFW_KEY_RSUPER`           | `GLFW_KEY_RIGHT_SUPER`       |       |

