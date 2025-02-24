# Vulkan guide {#vulkan_guide}

[TOC]

This guide is intended to fill the gaps between the official [Vulkan
resources](https://www.khronos.org/vulkan/) and the rest of the GLFW
documentation and is not a replacement for either.  It assumes some familiarity
with Vulkan concepts like loaders, devices, queues and surfaces and leaves it to
the Vulkan documentation to explain the details of Vulkan functions.

To develop for Vulkan you should download the [LunarG Vulkan
SDK](https://vulkan.lunarg.com/) for your platform.  Apart from headers and link
libraries, they also provide the validation layers necessary for development.

The [Vulkan Tutorial](https://vulkan-tutorial.com/) has more information on how
to use GLFW and Vulkan.  The [Khronos Vulkan
Samples](https://github.com/KhronosGroup/Vulkan-Samples) also use GLFW, although
with a small framework in between.

For details on a specific Vulkan support function, see the @ref vulkan.  There
are also guides for the other areas of the GLFW API.

 - @ref intro_guide
 - @ref window_guide
 - @ref context_guide
 - @ref monitor_guide
 - @ref input_guide


## Finding the Vulkan loader {#vulkan_loader}

GLFW itself does not ever need to be linked against the Vulkan loader.

By default, GLFW will load the Vulkan loader dynamically at runtime via its standard name:
`vulkan-1.dll` on Windows, `libvulkan.so.1` on Linux and other Unix-like systems and
`libvulkan.1.dylib` on macOS.

@macos GLFW will also look up and search the `Frameworks` subdirectory of your
application bundle.

If your code is using a Vulkan loader with a different name or in a non-standard location
you will need to direct GLFW to it.  Pass your version of `vkGetInstanceProcAddr` to @ref
glfwInitVulkanLoader before initializing GLFW and it will use that function for all Vulkan
entry point retrieval.  This prevents GLFW from dynamically loading the Vulkan loader.

```c
glfwInitVulkanLoader(vkGetInstanceProcAddr);
```

@macos To make your application be redistributable you will need to set up the application
bundle according to the LunarG SDK documentation.  This is explained in more detail in the
[SDK documentation for macOS](https://vulkan.lunarg.com/doc/sdk/latest/mac/getting_started.html).


## Including the Vulkan header file {#vulkan_include}

To have GLFW include the Vulkan header, define @ref GLFW_INCLUDE_VULKAN before including
the GLFW header.

```c
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

If you instead want to include the Vulkan header from a custom location or use
your own custom Vulkan header then do this before the GLFW header.

```c
#include <path/to/vulkan.h>
#include <GLFW/glfw3.h>
```

Unless a Vulkan header is included, either by the GLFW header or above it, the following
GLFW functions will not be declared, as depend on Vulkan types.

 - @ref glfwInitVulkanLoader
 - @ref glfwGetInstanceProcAddress
 - @ref glfwGetPhysicalDevicePresentationSupport
 - @ref glfwCreateWindowSurface

The `VK_USE_PLATFORM_*_KHR` macros do not need to be defined for the Vulkan part
of GLFW to work.  Define them only if you are using these extensions directly.


## Querying for Vulkan support {#vulkan_support}

If you are linking directly against the Vulkan loader then you can skip this
section.  The canonical desktop loader library exports all Vulkan core and
Khronos extension functions, allowing them to be called directly.

If you are loading the Vulkan loader dynamically instead of linking directly
against it, you can check for the availability of a loader and ICD with @ref
glfwVulkanSupported.

```c
if (glfwVulkanSupported())
{
    // Vulkan is available, at least for compute
}
```

This function returns `GLFW_TRUE` if the Vulkan loader and any minimally
functional ICD was found.

If one or both were not found, calling any other Vulkan related GLFW function
will generate a @ref GLFW_API_UNAVAILABLE error.


### Querying Vulkan function pointers {#vulkan_proc}

To load any Vulkan core or extension function from the found loader, call @ref
glfwGetInstanceProcAddress.  To load functions needed for instance creation,
pass `NULL` as the instance.

```c
PFN_vkCreateInstance pfnCreateInstance = (PFN_vkCreateInstance)
    glfwGetInstanceProcAddress(NULL, "vkCreateInstance");
```

Once you have created an instance, you can load from it all other Vulkan core
functions and functions from any instance extensions you enabled.

```c
PFN_vkCreateDevice pfnCreateDevice = (PFN_vkCreateDevice)
    glfwGetInstanceProcAddress(instance, "vkCreateDevice");
```

This function in turn calls `vkGetInstanceProcAddr`.  If that fails, the
function falls back to a platform-specific query of the Vulkan loader (i.e.
`dlsym` or `GetProcAddress`).  If that also fails, the function returns `NULL`.
For more information about `vkGetInstanceProcAddr`, see the Vulkan
documentation.

Vulkan also provides `vkGetDeviceProcAddr` for loading device-specific versions
of Vulkan function.  This function can be retrieved from an instance with @ref
glfwGetInstanceProcAddress.

```c
PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)
    glfwGetInstanceProcAddress(instance, "vkGetDeviceProcAddr");
```

Device-specific functions may execute a little faster, due to not having to
dispatch internally based on the device passed to them.  For more information
about `vkGetDeviceProcAddr`, see the Vulkan documentation.


## Querying required Vulkan extensions {#vulkan_ext}

To do anything useful with Vulkan you need to create an instance.  If you want
to use Vulkan to render to a window, you must enable the instance extensions
GLFW requires to create Vulkan surfaces.

To query the instance extensions required, call @ref
glfwGetRequiredInstanceExtensions.

```c
uint32_t count;
const char** extensions = glfwGetRequiredInstanceExtensions(&count);
```

These extensions must all be enabled when creating instances that are going to
be passed to @ref glfwGetPhysicalDevicePresentationSupport and @ref
glfwCreateWindowSurface.  The set of extensions will vary depending on platform
and may also vary depending on graphics drivers and other factors.

If it fails it will return `NULL` and GLFW will not be able to create Vulkan
window surfaces.  You can still use Vulkan for off-screen rendering and compute
work.

If successful the returned array will always include `VK_KHR_surface`, so if
you don't require any additional extensions you can pass this list directly to
the `VkInstanceCreateInfo` struct.

```c
VkInstanceCreateInfo ici;

memset(&ici, 0, sizeof(ici));
ici.enabledExtensionCount = count;
ici.ppEnabledExtensionNames = extensions;
...
```

Additional extensions may be required by future versions of GLFW.  You should
check whether any extensions you wish to enable are already in the returned
array, as it is an error to specify an extension more than once in the
`VkInstanceCreateInfo` struct.

@macos MoltenVK is (as of July 2022) not yet a fully conformant implementation
of Vulkan.  As of Vulkan SDK 1.3.216.0, this means you must also enable the
`VK_KHR_portability_enumeration` instance extension and set the
`VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR` bit in the instance creation
info flags for MoltenVK to show up in the list of physical devices.  For more
information, see the Vulkan and MoltenVK documentation.


## Querying for Vulkan presentation support {#vulkan_present}

Not every queue family of every Vulkan device can present images to surfaces.
To check whether a specific queue family of a physical device supports image
presentation without first having to create a window and surface, call @ref
glfwGetPhysicalDevicePresentationSupport.

```c
if (glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_family_index))
{
    // Queue family supports image presentation
}
```

The `VK_KHR_surface` extension additionally provides the
`vkGetPhysicalDeviceSurfaceSupportKHR` function, which performs the same test on
an existing Vulkan surface.


## Creating the window {#vulkan_window}

Unless you will be using OpenGL or OpenGL ES with the same window as Vulkan,
there is no need to create a context.  You can disable context creation with the
[GLFW_CLIENT_API](@ref GLFW_CLIENT_API_hint) hint.

```c
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
GLFWwindow* window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);
```

See @ref context_less for more information.


## Creating a Vulkan window surface {#vulkan_surface}

You can create a Vulkan surface (as defined by the `VK_KHR_surface` extension)
for a GLFW window with @ref glfwCreateWindowSurface.

```c
VkSurfaceKHR surface;
VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
if (err)
{
    // Window surface creation failed
}
```

If an OpenGL or OpenGL ES context was created on the window, the context has
ownership of the presentation on the window and a Vulkan surface cannot be
created.

It is your responsibility to destroy the surface.  GLFW does not destroy it for
you.  Call `vkDestroySurfaceKHR` function from the same extension to destroy it.

