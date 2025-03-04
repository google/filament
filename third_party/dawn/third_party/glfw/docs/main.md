# Introduction {#mainpage}

GLFW is a free, Open Source, multi-platform library for OpenGL, OpenGL ES and
Vulkan application development.  It provides a simple, platform-independent API
for creating windows, contexts and surfaces, reading input, handling events, etc.

@ref news list new features, caveats and deprecations.

@ref quick_guide is a guide for users new to GLFW.  It takes you through how to
write a small but complete program.

There are guides for each section of the API:

 - @ref intro_guide – initialization, error handling and high-level design
 - @ref window_guide – creating and working with windows and framebuffers
 - @ref context_guide – working with OpenGL and OpenGL ES contexts
 - @ref vulkan_guide - working with Vulkan objects and extensions
 - @ref monitor_guide – enumerating and working with monitors and video modes
 - @ref input_guide – receiving events, polling and processing input

Once you have written a program, see @ref compile_guide and @ref build_guide.

The [reference documentation](modules.html) provides more detailed information
about specific functions.

@ref moving_guide explains what has changed and how to update existing code to
use the new API.

There is a section on @ref guarantees_limitations for pointer lifetimes,
reentrancy, thread safety, event order and backward and forward compatibility.

Finally, @ref compat_guide explains what APIs, standards and protocols GLFW uses
and what happens when they are not present on a given machine.

This documentation was generated with Doxygen.  The sources for it are available
in both the [source distribution](https://www.glfw.org/download.html) and
[GitHub repository](https://github.com/glfw/glfw).

