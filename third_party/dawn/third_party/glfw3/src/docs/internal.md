# Internal structure {#internals_guide}

[TOC]

There are several interfaces inside GLFW.  Each interface has its own area of
responsibility and its own naming conventions.


## Public interface {#internals_public}

The most well-known is the public interface, described in the glfw3.h header
file.  This is implemented in source files shared by all platforms and these
files contain no platform-specific code.  This code usually ends up calling the
platform and internal interfaces to do the actual work.

The public interface uses the OpenGL naming conventions except with GLFW and
glfw instead of GL and gl.  For struct members, where OpenGL sets no precedent,
it use headless camel case.

Examples: `glfwCreateWindow`, `GLFWwindow`, `GLFW_RED_BITS`


## Native interface {#internals_native}

The [native interface](@ref native) is a small set of publicly available
but platform-specific functions, described in the glfw3native.h header file and
used to gain access to the underlying window, context and (on some platforms)
display handles used by the platform interface.

The function names of the native interface are similar to those of the public
interface, but embeds the name of the interface that the returned handle is
from.

Examples: `glfwGetX11Window`, `glfwGetWGLContext`


## Internal interface {#internals_internal}

The internal interface consists of utility functions used by all other
interfaces.  It is shared code implemented in the same shared source files as
the public and event interfaces.  The internal interface is described in the
internal.h header file.

The internal interface is in charge of GLFW's global data, which it stores in
a `_GLFWlibrary` struct named `_glfw`.

The internal interface uses the same style as the public interface, except all
global names have a leading underscore.

Examples: `_glfwIsValidContextConfig`, `_GLFWwindow`, `_glfw.monitorCount`


## Platform interface {#internals_platform}

The platform interface implements all platform-specific operations as a service
to the public interface.  This includes event processing.  The platform
interface is never directly called by application code and never directly calls
application-provided callbacks.  It is also prohibited from modifying the
platform-independent part of the internal structs.  Instead, it calls the event
interface when events interesting to GLFW are received.

The platform interface mostly mirrors those parts of the public interface that needs to
perform platform-specific operations on some or all platforms.

The window system bits of the platform API is called through the `_GLFWplatform` struct of
function pointers, to allow runtime selection of platform.  This includes the window and
context creation, input and event processing, monitor and Vulkan surface creation parts of
GLFW.  This is located in the global `_glfw` struct.

Examples: `_glfw.platform.createWindow`

The timer, threading and module loading bits of the platform API are plain functions with
a `_glfwPlatform` prefix, as these things are independent of what window system is being
used.

Examples: `_glfwPlatformGetTimerValue`

The platform interface also defines structs that contain platform-specific
global and per-object state.  Their names mirror those of the internal
interface, except that an interface-specific suffix is added.

Examples: `_GLFWwindowX11`, `_GLFWcontextWGL`

These structs are incorporated as members into the internal interface structs
using special macros that name them after the specific interface used.  This
prevents shared code from accidentally using these members.

Examples: `window->win32.handle`, `_glfw.x11.display`


## Event interface {#internals_event}

The event interface is implemented in the same shared source files as the public
interface and is responsible for delivering the events it receives to the
application, either via callbacks, via window state changes or both.

The function names of the event interface use a `_glfwInput` prefix and the
ObjectEvent pattern.

Examples: `_glfwInputWindowFocus`, `_glfwInputCursorPos`


## Static functions {#internals_static}

Static functions may be used by any interface and have no prefixes or suffixes.
These use headless camel case.

Examples: `isValidElementForJoystick`


## Configuration macros {#internals_config}

GLFW uses a number of configuration macros to select at compile time which
interfaces and code paths to use.  They are defined in the GLFW CMake target.

Configuration macros the same style as tokens in the public interface, except
with a leading underscore.

Examples: `_GLFW_WIN32`, `_GLFW_BUILD_DLL`

