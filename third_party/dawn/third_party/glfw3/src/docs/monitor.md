# Monitor guide {#monitor_guide}

[TOC]

This guide introduces the monitor related functions of GLFW.  For details on
a specific function in this category, see the @ref monitor.  There are also
guides for the other areas of GLFW.

 - @ref intro_guide
 - @ref window_guide
 - @ref context_guide
 - @ref vulkan_guide
 - @ref input_guide


## Monitor objects {#monitor_object}

A monitor object represents a currently connected monitor and is represented as
a pointer to the [opaque](https://en.wikipedia.org/wiki/Opaque_data_type) type
@ref GLFWmonitor.  Monitor objects cannot be created or destroyed by the
application and retain their addresses until the monitors they represent are
disconnected or until the library is [terminated](@ref intro_init_terminate).

Each monitor has a current video mode, a list of supported video modes,
a virtual position, a human-readable name, an estimated physical size and
a gamma ramp.  One of the monitors is the primary monitor.

The virtual position of a monitor is in
[screen coordinates](@ref coordinate_systems) and, together with the current
video mode, describes the viewports that the connected monitors provide into the
virtual desktop that spans them.

To see how GLFW views your monitor setup and its available video modes, run the
`monitors` test program.


### Retrieving monitors {#monitor_monitors}

The primary monitor is returned by @ref glfwGetPrimaryMonitor.  It is the user's
preferred monitor and is usually the one with global UI elements like task bar
or menu bar.

```c
GLFWmonitor* primary = glfwGetPrimaryMonitor();
```

You can retrieve all currently connected monitors with @ref glfwGetMonitors.
See the reference documentation for the lifetime of the returned array.

```c
int count;
GLFWmonitor** monitors = glfwGetMonitors(&count);
```

The primary monitor is always the first monitor in the returned array, but other
monitors may be moved to a different index when a monitor is connected or
disconnected.


### Monitor configuration changes {#monitor_event}

If you wish to be notified when a monitor is connected or disconnected, set
a monitor callback.

```c
glfwSetMonitorCallback(monitor_callback);
```

The callback function receives the handle for the monitor that has been
connected or disconnected and the event that occurred.

```c
void monitor_callback(GLFWmonitor* monitor, int event)
{
    if (event == GLFW_CONNECTED)
    {
        // The monitor was connected
    }
    else if (event == GLFW_DISCONNECTED)
    {
        // The monitor was disconnected
    }
}
```

If a monitor is disconnected, all windows that are full screen on it will be
switched to windowed mode before the callback is called.  Only @ref
glfwGetMonitorName and @ref glfwGetMonitorUserPointer will return useful values
for a disconnected monitor and only before the monitor callback returns.


## Monitor properties {#monitor_properties}

Each monitor has a current video mode, a list of supported video modes,
a virtual position, a content scale, a human-readable name, a user pointer, an
estimated physical size and a gamma ramp.


### Video modes {#monitor_modes}

GLFW generally does a good job selecting a suitable video mode when you create
a full screen window, change its video mode or make a windowed one full
screen, but it is sometimes useful to know exactly which video modes are
supported.

Video modes are represented as @ref GLFWvidmode structures.  You can get an
array of the video modes supported by a monitor with @ref glfwGetVideoModes.
See the reference documentation for the lifetime of the returned array.

```c
int count;
GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);
```

To get the current video mode of a monitor call @ref glfwGetVideoMode.  See the
reference documentation for the lifetime of the returned pointer.

```c
const GLFWvidmode* mode = glfwGetVideoMode(monitor);
```

The resolution of a video mode is specified in
[screen coordinates](@ref coordinate_systems), not pixels.


### Physical size {#monitor_size}

The physical size of a monitor in millimetres, or an estimation of it, can be
retrieved with @ref glfwGetMonitorPhysicalSize.  This has no relation to its
current _resolution_, i.e. the width and height of its current
[video mode](@ref monitor_modes).

```c
int width_mm, height_mm;
glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
```

While this can be used to calculate the raw DPI of a monitor, this is often not
useful.  Instead, use the [monitor content scale](@ref monitor_scale) and
[window content scale](@ref window_scale) to scale your content.


### Content scale {#monitor_scale}

The content scale for a monitor can be retrieved with @ref
glfwGetMonitorContentScale.

```c
float xscale, yscale;
glfwGetMonitorContentScale(monitor, &xscale, &yscale);
```

For more information on what the content scale is and how to use it, see
[window content scale](@ref window_scale).


### Virtual position {#monitor_pos}

The position of the monitor on the virtual desktop, in
[screen coordinates](@ref coordinate_systems), can be retrieved with @ref
glfwGetMonitorPos.

```c
int xpos, ypos;
glfwGetMonitorPos(monitor, &xpos, &ypos);
```


### Work area {#monitor_workarea}

The area of a monitor not occupied by global task bars or menu bars is the work
area.  This is specified in [screen coordinates](@ref coordinate_systems) and
can be retrieved with @ref glfwGetMonitorWorkarea.

```c
int xpos, ypos, width, height;
glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
```


### Human-readable name {#monitor_name}

The human-readable, UTF-8 encoded name of a monitor is returned by @ref
glfwGetMonitorName.  See the reference documentation for the lifetime of the
returned string.

```c
const char* name = glfwGetMonitorName(monitor);
```

Monitor names are not guaranteed to be unique.  Two monitors of the same model
and make may have the same name.  Only the monitor handle is guaranteed to be
unique, and only until that monitor is disconnected.


### User pointer {#monitor_userptr}

Each monitor has a user pointer that can be set with @ref
glfwSetMonitorUserPointer and queried with @ref glfwGetMonitorUserPointer.  This
can be used for any purpose you need and will not be modified by GLFW.  The
value will be kept until the monitor is disconnected or until the library is
terminated.

The initial value of the pointer is `NULL`.


### Gamma ramp {#monitor_gamma}

The gamma ramp of a monitor can be set with @ref glfwSetGammaRamp, which accepts
a monitor handle and a pointer to a @ref GLFWgammaramp structure.

```c
GLFWgammaramp ramp;
unsigned short red[256], green[256], blue[256];

ramp.size = 256;
ramp.red = red;
ramp.green = green;
ramp.blue = blue;

for (i = 0;  i < ramp.size;  i++)
{
    // Fill out gamma ramp arrays as desired
}

glfwSetGammaRamp(monitor, &ramp);
```

The gamma ramp data is copied before the function returns, so there is no need
to keep it around once the ramp has been set.

It is recommended that your gamma ramp have the same size as the current gamma
ramp for that monitor.

The current gamma ramp for a monitor is returned by @ref glfwGetGammaRamp.  See
the reference documentation for the lifetime of the returned structure.

```c
const GLFWgammaramp* ramp = glfwGetGammaRamp(monitor);
```

If you wish to set a regular gamma ramp, you can have GLFW calculate it for you
from the desired exponent with @ref glfwSetGamma, which in turn calls @ref
glfwSetGammaRamp with the resulting ramp.

```c
glfwSetGamma(monitor, 1.0);
```

To experiment with gamma correction via the @ref glfwSetGamma function, run the
`gamma` test program.

@note The software controlled gamma ramp is applied _in addition_ to the
hardware gamma correction, which today is typically an approximation of sRGB
gamma.  This means that setting a perfectly linear ramp, or gamma 1.0, will
produce the default (usually sRGB-like) behavior.

@note __Wayland:__ An application cannot read or modify the monitor gamma ramp.
The @ref glfwGetGammaRamp, @ref glfwSetGammaRamp and @ref glfwSetGamma functions
emit @ref GLFW_FEATURE_UNAVAILABLE.

