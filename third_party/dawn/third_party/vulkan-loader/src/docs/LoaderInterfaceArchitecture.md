<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Architecture of the Vulkan Loader Interfaces <!-- omit from toc -->
[![Creative Commons][3]][4]

<!-- Copyright &copy; 2015-2023 LunarG, Inc. -->

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/
## Table of Contents <!-- omit from toc -->

- [Overview](#overview)
  - [Who Should Read This Document](#who-should-read-this-document)
  - [The Loader](#the-loader)
    - [Goals of the Loader](#goals-of-the-loader)
  - [Layers](#layers)
  - [Drivers](#drivers)
    - [Installable Client Drivers](#installable-client-drivers)
  - [VkConfig](#vkconfig)
- [Important Vulkan Concepts](#important-vulkan-concepts)
  - [Instance Versus Device](#instance-versus-device)
    - [Instance-Specific](#instance-specific)
      - [Instance Objects](#instance-objects)
      - [Instance Functions](#instance-functions)
      - [Instance Extensions](#instance-extensions)
    - [Device-Specific](#device-specific)
      - [Device Objects](#device-objects)
      - [Device Functions](#device-functions)
      - [Device Extensions](#device-extensions)
  - [Dispatch Tables and Call Chains](#dispatch-tables-and-call-chains)
    - [Instance Call Chain Example](#instance-call-chain-example)
    - [Device Call Chain Example](#device-call-chain-example)
- [Elevated Privilege Caveats](#elevated-privilege-caveats)
- [Application Interface to the Loader](#application-interface-to-the-loader)
- [Layer Interface with the Loader](#layer-interface-with-the-loader)
- [Driver Interface With the Loader](#driver-interface-with-the-loader)
- [Debugging Issues](#debugging-issues)
- [Loader Policies](#loader-policies)
- [Filter Environment Variable Behaviors](#filter-environment-variable-behaviors)
  - [Comparison Strings](#comparison-strings)
  - [Comma-Delimited Lists](#comma-delimited-lists)
  - [Globs](#globs)
  - [Case-Insensitive](#case-insensitive)
  - [Environment Variable Priority](#environment-variable-priority)
- [Table of Debug Environment Variables](#table-of-debug-environment-variables)
  - [Active Environment Variables](#active-environment-variables)
  - [Deprecated Environment Variables](#deprecated-environment-variables)
- [Glossary of Terms](#glossary-of-terms)

## Overview

Vulkan is a layered architecture, made up of the following elements:
  * The Vulkan Application
  * [The Vulkan Loader](#the-loader)
  * [Vulkan Layers](#layers)
  * [Drivers](#drivers)
  * [VkConfig](#vkconfig)

![High Level View of Loader](./images/high_level_loader.png)

The general concepts in this document are applicable to the loaders available
for Windows, Linux, Android, and macOS systems.


### Who Should Read This Document

While this document is primarily targeted at developers of Vulkan applications,
drivers and layers, the information contained in it could be useful to anyone
wanting a better understanding of the Vulkan runtime.


### The Loader

The application sits at the top and interfaces directly with the Vulkan
loader.
At the bottom of the stack sits the drivers.
A driver can control one or more physical devices capable of rendering Vulkan,
implement a conversion from Vulkan into a native graphics API (like
[MoltenVk](https://github.com/KhronosGroup/MoltenVK), or implement a fully
software path that can be executed on a CPU to simulate a Vulkan device (like
[SwiftShader](https://github.com/google/swiftshader) or LavaPipe).
Remember, Vulkan-capable hardware may be graphics-based, compute-based, or
both.
Between the application and the drivers, the loader can inject any number of
optional [layers](#layers) that provide special functionality.
The loader is critical to managing the proper dispatching of Vulkan
functions to the appropriate set of layers and drivers.
The Vulkan object model allows the loader to insert layers into a call-chain
so that the layers can process Vulkan functions prior to the driver being
called.

This document is intended to provide an overview of the necessary interfaces
between each of these.


#### Goals of the Loader

The loader was designed with the following goals in mind:
 1. Support one or more Vulkan-capable drivers on a user's system without them
 interfering with one another.
 2. Support Vulkan Layers which are optional modules that can be enabled by an
application, developer, or standard system settings.
 3. Keep the overall overhead of the loader to the minimum possible.


### Layers

Layers are optional components that augment the Vulkan development environment.
They can intercept, evaluate, and modify existing Vulkan functions on their
way from the application down to the drivers and back up.
Layers are implemented as libraries that can be enabled in different ways
and are loaded during CreateInstance.
Each layer can choose to hook, or intercept, Vulkan functions which in
turn can be ignored, inspected, or augmented.
Any function a layer does not hook is simply skipped for that layer and the
control flow will simply continue on to the next supporting layer or
driver.
Because of this, a layer can choose whether to intercept all known Vulkan
functions or only a subset it is interested in.

Some examples of features that layers may expose include:
 * Validating API usage
 * Tracing API calls
 * Debugging aids
 * Profiling
 * Overlay

Because layers are optional and dynamically loaded, they can be enabled
and disabled as desired.
For example, while developing and debugging an application, enabling
certain layers can assist in making sure it properly uses the Vulkan API.
But when releasing the application, those layers are unnecessary
and thus won't be enabled, increasing the speed of the application.


### Drivers

The library that implements Vulkan, either through supporting a physical
hardware device directly, converting Vulkan commands into native graphics
commands, or simulating Vulkan through software, is considered "a driver".
The most common type of driver is still the Installable Client Driver (or ICD).
The loader is responsible for discovering available Vulkan drivers on the
system.
Given a list of available drivers, the loader can enumerate all the available
physical devices and provide this information for an application.


#### Installable Client Drivers

Vulkan allows multiple ICDs each supporting one or more devices.
Each of these devices is represented by a Vulkan `VkPhysicalDevice` object.
The loader is responsible for discovering available Vulkan ICDs via the standard
driver search on the system.


### VkConfig

VkConfig is a tool LunarG has developed to assist with modifying the Vulkan
environment on the local system.
It can be used to find layers, enable them, change layer settings, and other
useful features.
VkConfig can be found by either installing the
[Vulkan SDK](https://vulkan.lunarg.com/) or by building the source out of the
[LunarG VulkanTools GitHub Repo](https://github.com/LunarG/VulkanTools).

VkConfig generates three outputs, two of which work with the Vulkan loader and
layers.
These outputs are:
 * The Vulkan Override Layer
 * The Vulkan Layer Settings File
 * VkConfig Configuration Settings

These files are found in different locations based on your platform:

<table style="width:100%">
  <tr>
    <th>Platform</th>
    <th>Output</th>
    <th>Location</th>
  </tr>
  <tr>
    <th rowspan="3">Linux</th>
    <td>Vulkan Override Layer</td>
    <td>$USER/.local/share/vulkan/implicit_layer.d/VkLayer_override.json</td>
  </tr>
  <tr>
    <td>Vulkan Layer Settings</td>
    <td>$USER/.local/share/vulkan/settings.d/vk_layer_settings.txt</td>
  </tr>
  <tr>
    <td>VkConfig Configuration Settings</td>
    <td>$USER/.local/share/vulkan/settings.d/vk_layer_settings.txt</td>
  </tr>
  <tr>
    <th rowspan="3">Windows</th>
    <td>Vulkan Override Layer</td>
    <td>%HOME%\AppData\Local\LunarG\vkconfig\override\VkLayerOverride.json</td>
  </tr>
  <tr>
    <td>Vulkan Layer Settings</td>
    <td>(registry) HKEY_CURRENT_USER\Software\Khronos\Vulkan\LoaderSettings</td>
  </tr>
  <tr>
    <td>VkConfig Configuration Settings</td>
    <td>(registry) HKEY_CURRENT_USER\Software\LunarG\vkconfig </td>
  </tr>
</table>

The [Override Meta-Layer](./LoaderLayerInterface.md#override-meta-layer) is
an important part of how VkConfig works.
This layer, when found by the loader, forces the loading of the desired layers
that were enabled inside of VkConfig as well as disables those layers that
were intentionally disabled (including implicit layers).

The Vulkan Layer Settings file can be used to specify certain behaviors and
actions each enabled layer is expected to perform.
These settings can also be controlled by VkConfig, or they can be manually
enabled.
For details on what settings can be used, refer to the individual layers.

In the future, VkConfig may have additional interactions with the Vulkan
loader.

More details on VkConfig can be found in its
[GitHub documentation](https://github.com/LunarG/VulkanTools/blob/main/vkconfig/README.md).
<br/>
<br/>


## Important Vulkan Concepts

Vulkan has a few concepts that provide a fundamental basis for its organization.
These concepts should be understood by any one attempting to use Vulkan or
develop any of its components.


### Instance Versus Device

An important concept to understand, which is brought up repeatedly throughout this
document, is how the Vulkan API is organized.
Many objects, functions, extensions, and other behavior in Vulkan can be
separated into two groups:
 * [Instance-specific](#instance-specific)
 * [Device-specific](#device-specific)


#### Instance-Specific

A "Vulkan instance" (`VkInstance`) is a high-level construct used to provide
Vulkan system-level information and functionality.

##### Instance Objects

A few Vulkan objects associated directly with an instance are:
 * `VkInstance`
 * `VkPhysicalDevice`
 * `VkPhysicalDeviceGroup`

##### Instance Functions

An "instance function" is any Vulkan function where the first parameter is an
[instance object](#instance-objects) or no object at all.

Some Vulkan instance functions are:
 * `vkEnumerateInstanceExtensionProperties`
 * `vkEnumeratePhysicalDevices`
 * `vkCreateInstance`
 * `vkDestroyInstance`

An application can link directly to all core instance functions through the
Vulkan loader's headers.
Alternatively, an application can query function pointers using
`vkGetInstanceProcAddr`.
`vkGetInstanceProcAddr` can be used to query any instance or device entry-points
in addition to all core entry-points.

If `vkGetInstanceProcAddr` is called using a `VkInstance`, then any function
pointer returned is specific to that `VkInstance` and any additional objects
that are created from it.

##### Instance Extensions

Extensions to Vulkan are similarly associated based on what type of
functions they provide.
Because of this, extensions are broken up into instance or device extensions
where most, if not all of the functions, in the extension are of the
corresponding type.
For example, an "instance extension" is composed primarily of "instance
functions" which primarily take instance objects.
These will be discussed in more detail later.


#### Device-Specific

A Vulkan device (`VkDevice`), on the other-hand, is a logical identifier used
to associate functions with a particular Vulkan physical device
(`VkPhysicalDevice`) through a particular driver on a user's system.

##### Device Objects

A few of the Vulkan constructs associated directly with a device include:
 * `VkDevice`
 * `VkQueue`
 * `VkCommandBuffer`

##### Device Functions

A "device function" is any Vulkan function which takes any device object as its
first parameter or a child object of the device.
The vast majority of Vulkan functions are device functions.
Some Vulkan device functions are:
 * `vkQueueSubmit`
 * `vkBeginCommandBuffer`
 * `vkCreateEvent`

Vulkan devices functions may be queried using either `vkGetInstanceProcAddr` or
`vkGetDeviceProcAddr`.
If an application chooses to use `vkGetInstanceProcAddr`, each call will have
additional function calls built into the call chain, which will reduce
performance slightly.
If, instead, the application uses `vkGetDeviceProcAddr`, the call chain will be
more optimized to the specific device, but the returned function pointers will
**only** work for the device used when querying them.
Unlike `vkGetInstanceProcAddr`, `vkGetDeviceProcAddr` can only be used on
Vulkan device functions.

The best solution is to query instance extension functions using
`vkGetInstanceProcAddr`, and to query device extension functions using
`vkGetDeviceProcAddr`.
See
[Best Application Performance Setup](LoaderApplicationInterface.md#best-application-performance-setup)
section in the
[LoaderApplicationInterface.md](LoaderApplicationInterface.md) document for more
information on this.

##### Device Extensions

As with instance extensions, a device extension is a set of Vulkan device
functions extending the Vulkan language.
More information about device extensions can be found later in this document.


### Dispatch Tables and Call Chains

Vulkan uses an object model to control the scope of a particular action or
operation.
The object to be acted on is always the first parameter of a Vulkan call and is
a dispatchable object (see Vulkan specification section 3.3 Object Model).
Under the covers, the dispatchable object handle is a pointer to a structure,
which in turn, contains a pointer to a dispatch table maintained by the loader.
This dispatch table contains pointers to the Vulkan functions appropriate to
that object.

There are two types of dispatch tables the loader maintains:
 - Instance Dispatch Table
   - Created in the loader during the call to `vkCreateInstance`
 - Device Dispatch Table
   - Created in the loader during the call to `vkCreateDevice`

At that time the application and the system can each specify optional layers to
be included.
The loader will initialize the specified layers to create a call chain for each
Vulkan function and each entry of the dispatch table will point to the first
element of that chain.
Thus, the loader builds an instance call chain for each `VkInstance` that is
created and a device call chain for each `VkDevice` that is created.

When an application calls a Vulkan function, this typically will first hit a
*trampoline* function in the loader.
These *trampoline* functions are small, simple functions that jump to the
appropriate dispatch table entry for the object they are given.
Additionally, for functions in the instance call chain, the loader has an
additional function, called a *terminator*, which is called after all enabled
layers to marshall the appropriate information to all available drivers.


#### Instance Call Chain Example

For example, the diagram below represents what happens in the call chain for
`vkCreateInstance`.
After initializing the chain, the loader calls into the first layer's
`vkCreateInstance`, which will call the next layer's `vkCreateInstance`
before finally terminating in the loader again where it will call
every driver's `vkCreateInstance`.
This allows every enabled layer in the chain to set up what it needs based on
the `VkInstanceCreateInfo` structure from the application.

![Instance Call Chain](./images/loader_instance_chain.png)

This also highlights some of the complexity the loader must manage when using
instance call chains.
As shown here, the loader's *terminator* must aggregate information to and from
multiple drivers when they are present.
This implies that the loader has to be aware of any instance-level extensions
which work on a `VkInstance` to aggregate them correctly.


#### Device Call Chain Example

Device call chains are created in `vkCreateDevice` and are generally simpler
because they deal with only a single device.
This allows for the specific driver exposing this device to always be the
*terminator* of the chain.

![Loader Device Call Chain](./images/loader_device_chain_loader.png)
<br/>


## Elevated Privilege Caveats

To ensure that the system is safe from exploitation, Vulkan applications which
are run with elevated privileges are restricted from certain operations, such
as reading environment variables from unsecure locations or searching for
files in user controlled paths.
This is done to ensure that an application running with elevated privileges does
not run using components that were not installed in the proper approved
locations.

The loader uses platform-specific mechanisms (such as `secure_getenv` and its
equivalents) for querying sensitive environment variables to avoid accidentally
using untrusted results.

These behaviors also result in ignoring certain environment variables, such as:

  * `VK_DRIVER_FILES` / `VK_ICD_FILENAMES`
  * `VK_ADD_DRIVER_FILES`
  * `VK_LAYER_PATH`
  * `VK_ADD_LAYER_PATH`
  * `VK_IMPLICIT_LAYER_PATH`
  * `VK_ADD_IMPLICIT_LAYER_PATH`
  * `XDG_CONFIG_HOME` (Linux/Mac-specific)
  * `XDG_DATA_HOME` (Linux/Mac-specific)

For more information on the affected search paths, refer to
[Layer Discovery](LoaderLayerInterface.md#layer-discovery) and
[Driver Discovery](LoaderDriverInterface.md#driver-discovery).
<br/>
<br/>


## Application Interface to the Loader

The Application interface to the Vulkan loader is now detailed in the
[LoaderApplicationInterface.md](LoaderApplicationInterface.md) document found in
the same directory as this file.
<br/>
<br/>


## Layer Interface with the Loader

The Layer interface to the Vulkan loader is detailed in the
[LoaderLayerInterface.md](LoaderLayerInterface.md) document found in the same
directory as this file.
<br/>
<br/>


## Driver Interface With the Loader

The Driver interface to the Vulkan loader is detailed in the
[LoaderDriverInterface.md](LoaderDriverInterface.md) document found in the same
directory as this file.
<br/>
<br/>


## Debugging Issues


If your application is crashing or behaving weirdly, the loader provides
several mechanisms for you to debug the issues.
These are detailed in the [LoaderDebugging.md](LoaderDebugging.md) document
found in the same directory as this file.
<br/>
<br/>


## Loader Policies

Loader policies with regards to the loader interaction with drivers and layers
 are now documented in the appropriate sections.
The intention of these sections is to clearly define expected behavior of the
loader with regards to its interactions with those components.
This could be especially useful in cases where a new or specialized loader may
be required that conforms to the behavior of the existing loader.
Because of this, the primary focus of those sections is on expected behaviors
for all relevant components to create a consistent experience across platforms.
In the long-run, this could also be used as validation requirements for any
existing Vulkan loaders.

To review the particular policy sections, please refer to one or both of the
sections listed below:
 * [Loader And Driver Policy](LoaderDriverInterface.md#loader-and-driver-policy)
 * [Loader And Layer Policy](LoaderLayerInterface.md#loader-and-layer-policy)
<br/>
<br/>

## Filter Environment Variable Behaviors

The filter environment variables provided in certain areas have some common
restrictions and behaviors that should be listed.

### Comparison Strings

The filter variables will be compared against the appropriate strings for either
drivers or layers.
The appropriate string for layers is the layer name provided in the layer's
manifest file.
Since drivers don’t have a name like layers, this substring is used to compare
against the driver manifest's filename.

### Comma-Delimited Lists

All of the filter environment variables accept comma-delimited input.
Therefore, you can chain multiple strings together and it will use the strings
to individually enable or disable the appropriate item in the current list of
available items.

### Globs

To provide enough flexibility to limit name searches to only those wanted by the
developer, the loader uses a limited glob format for strings.
Acceptable globs are:
 - Prefixes:   `"string*"`
 - Suffixes:   `"*string"`
 - Substrings:  `"*string*"`
 - Whole strings: `"string"`
   - In the case of whole strings, the string will be compared against each
     layer or driver file name in its entirety.
   - Because of this, it will only match the specific target such as:
     `VK_LAYER_KHRONOS_validation` will match the layer name
     `VK_LAYER_KHRONOS_validation`, but **not** a layer named
     `VK_LAYER_KHRONOS_validation2` (not that there is such a layer).

This is especially useful because it is difficult sometimes to determine the
full name of a driver manifest file or even some commonly used layers
such as `VK_LAYER_KHRONOS_validation`.

### Case-Insensitive

All of the filter environment variables assume the strings inside of the glob
are not case-sensitive.
Therefore, “Bob”, “bob”, and “BOB” all amount to the same thing.

### Environment Variable Priority

The values from the *disable* environment variable will be considered
**before** the *enable* or *select* environment variable.
Because of this, it is possible to disable a layer/driver using the *disable*
environment variable, only to have it be re-enabled by the *enable*/*select*
environment variable.
This is useful if you disable all layers/drivers with the intent of only
enabling a smaller subset of specific layers/drivers for issue triaging.

## Table of Debug Environment Variables

The following are all the Debug Environment Variables available for use with the
Loader.
These are referenced throughout the text, but collected here for ease of
discovery.

### Active Environment Variables

<table style="width:100%">
  <tr>
    <th>Environment Variable</th>
    <th>Behavior</th>
    <th>Restrictions</th>
    <th>Example Format</th>
  </tr>
  <tr>
    <td><small>
        <i>VK_ADD_DRIVER_FILES</i>
    </small></td>
    <td><small>
        Provide a list of additional driver JSON files that the loader will use
        in addition to the drivers that the loader would find normally.
        The list of drivers will be added first, prior to the list of drivers
        that would be found normally.
        The value contains a list of delimited full path listings to
        driver JSON Manifest files.<br/>
    </small></td>
    <td><small>
        If a global path to the JSON file is not used, issues may be encountered.
        <br/> <br/>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_ADD_DRIVER_FILES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>/intel.json:<folder_b>/amd.json
        <br/> <br/>
        set<br/>
        &nbsp;&nbsp;VK_ADD_DRIVER_FILES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>\nvidia.json;<folder_b>\mesa.json
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_ADD_LAYER_PATH</i>
    </small></td>
    <td><small>
        Provide a list of additional paths that the loader will use to search
        for explicit layers in addition to the loader's standard layer library
        search paths when looking for layer manifest files.
        The paths will be added first, prior to the list of folders that would
        be searched normally.
    </small></td>
    <td><small>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_ADD_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;:&lt;path_b&gt;<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_ADD_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;;&lt;path_b&gt;</small>
    </td>
  </tr>
    <tr>
    <td><small>
        <i>VK_ADD_IMPLICIT_LAYER_PATH</i>
    </small></td>
    <td><small>
        Provide a list of additional paths that the loader will use to search
        for implicit layers in addition to the loader's standard layer library
        search paths when looking for layer manifest files.
        The paths will be added first, prior to the list of folders that would
        be searched normally.
    </small></td>
    <td><small>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_ADD_IMPLICIT_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;:&lt;path_b&gt;<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_ADD_IMPLICIT_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;;&lt;path_b&gt;</small>
    </td>
  </tr>
  <tr>
    <td><small>
        <i>VK_DRIVER_FILES</i>
    </small></td>
    <td><small>
        Force the loader to use the specific driver JSON files.
        The value contains a list of delimited full path listings to
        driver JSON Manifest files and/or
        paths to folders containing driver JSON files.<br/>
        <br/>
        This has replaced the older deprecated environment variable
        <i>VK_ICD_FILENAMES</i>, however the older environment variable will
        continue to work.
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.207 of the Vulkan headers and later.<br/>
        It is recommended to use absolute paths to JSON files.
        Relative paths may have issues due to how the loader transforms relative library
        paths into absolute ones.
        <br/> <br/>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_DRIVER_FILES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>/intel.json:<folder_b>/amd.json
        <br/> <br/>
        set<br/>
        &nbsp;&nbsp;VK_DRIVER_FILES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>\nvidia.json;<folder_b>\mesa.json
        </small>
    </td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LAYER_PATH</i></small></td>
    <td><small>
        Override the loader's standard explicit layer search paths and use the
        provided delimited files and/or folders to locate layer manifest files.
    </small></td>
    <td><small>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;:&lt;path_b&gt;<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;;&lt;path_b&gt;
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_IMPLICIT_LAYER_PATH</i></small></td>
    <td><small>
        Override the loader's standard implicit layer search paths and use the
        provided delimited files and/or folders to locate layer manifest files.
    </small></td>
    <td><small>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_IMPLICIT_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;:&lt;path_b&gt;<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_IMPLICIT_LAYER_PATH=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;path_a&gt;;&lt;path_b&gt;
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DEBUG</i>
    </small></td>
    <td><small>
        Enable loader debug messages using a comma-delimited list of level
        options.  These options are:<br/>
        &nbsp;&nbsp;* error (only errors)<br/>
        &nbsp;&nbsp;* warn (only warnings)<br/>
        &nbsp;&nbsp;* info (only info)<br/>
        &nbsp;&nbsp;* debug (only debug)<br/>
        &nbsp;&nbsp;* layer (layer-specific output)<br/>
        &nbsp;&nbsp;* driver (driver-specific output)<br/>
        &nbsp;&nbsp;* all (report out all messages)<br/><br/>
        To enable multiple options (outside of "all") like info, warning and
        error messages, set the value to "error,warn,info".
    </small></td>
    <td><small>
        None
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_DEBUG=all<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_DEBUG=warn
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DEVICE_SELECT</i>
    </small></td>
    <td><small>
        Allows the user to force a particular device to be prioritized above all
        other devices in the return order of <i>vkGetPhysicalDevices<i> and
        <i>vkGetPhysicalDeviceGroups<i> functions.<br/>
        The value should be "<hex vendor id>:<hex device id>".<br/>
        <b>NOTE:</b> This DOES NOT REMOVE devices from the list on reorders them.
    </small></td>
    <td><small>
        <b>Linux Only</b>
    </small></td>
    <td><small>
        set VK_LOADER_DEVICE_SELECT=0x10de:0x1f91
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DISABLE_SELECT</i>
    </small></td>
    <td><small>
        Allows the user to disable the consistent sorting algorithm run in the
        loader before returning the set of physical devices to layers.<br/>
    </small></td>
    <td><small>
        <b>Linux Only</b>
    </small></td>
    <td><small>
        set VK_LOADER_DISABLE_SELECT=1
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DISABLE_INST_EXT_FILTER</i>
    </small></td>
    <td><small>
        Disable the filtering out of instance extensions that the loader doesn't
        know about.
        This will allow applications to enable instance extensions exposed by
        drivers but that the loader has no support for.<br/>
    </small></td>
    <td><small>
        <b>Use Wisely!</b> This may cause the loader or application to crash.
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_DISABLE_INST_EXT_FILTER=1<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_DISABLE_INST_EXT_FILTER=1
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DRIVERS_SELECT</i>
    </small></td>
    <td><small>
        A comma-delimited list of globs to search for in known drivers and
        used to select only the drivers whose manifest file names match one or
        more of the provided globs.<br/>
        Since drivers don’t have a name like layers, this glob is used to
        compare against the manifest filename.
        Known driver manifests being those files that are already found by the
        loader taking into account default search paths and other environment
        variables (like <i>VK_ICD_FILENAMES</i> or <i>VK_ADD_DRIVER_FILES</i>).
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.234 of the Vulkan headers and later.<br/>
        If no drivers are found with a manifest filename that matches any of the
        provided globs, then no driver is enabled and it <b>may</b> result
        in Vulkan applications failing to run properly.
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_DRIVERS_SELECT=nvidia*<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_DRIVERS_SELECT=nvidia*<br/><br/>
        The above would select only the Nvidia driver if it was present on the
        system and already visible to the loader.
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DRIVERS_DISABLE</i>
    </small></td>
    <td><small>
        A comma-delimited list of globs to search for in known drivers and
        used to disable only the drivers whose manifest file names match one or
        more of the provided globs.<br/>
        Since drivers don’t have a name like layers, this glob is used to
        compare against the manifest filename.
        Known driver manifests being those files that are already found by the
        loader taking into account default search paths and other environment
        variables (like <i>VK_ICD_FILENAMES</i> or <i>VK_ADD_DRIVER_FILES</i>).
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.234 of the Vulkan headers and later.<br/>
        If all available drivers are disabled using this environment variable,
        then no drivers will be found by the loader and <b>will</b> result
        in Vulkan applications failing to run properly.<br/>
        This is also checked before other driver environment variables (such as
        <i>VK_LOADER_DRIVERS_SELECT</i>) so that a user may easily disable all
        drivers and then selectively re-enable individual drivers using the
        enable environment variable.
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_DRIVERS_DISABLE=*amd*,*intel*<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_DRIVERS_DISABLE=*amd*,*intel*<br/><br/>
        The above would disable both Intel and AMD drivers if both were present
        on the system and already visible to the loader.
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_LAYERS_ENABLE</i>
    </small></td>
    <td><small>
        A comma-delimited list of globs to search for in known layers and
        used to select only the layers whose layer name matches one or more of
        the provided globs.<br/>
        Known layers are those which are found by the loader taking into account
        default search paths and other environment variables
        (like <i>VK_LAYER_PATH</i>).
        <br/>
        This has replaced the older deprecated environment variable
        <i>VK_INSTANCE_LAYERS</i>
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.234 of the Vulkan headers and later.
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_ENABLE=*validation,*recon*<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_ENABLE=*validation,*recon*<br/><br/>
        The above would enable the Khronos validation layer and the
        GfxReconstruct layer, if both were present on the system and already
        visible to the loader.
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_LAYERS_DISABLE</i>
    </small></td>
    <td><small>
        A comma-delimited list of globs to search for in known layers and
        used to disable only the layers whose layer name matches one or more of
        the provided globs.<br/>
        Known layers are those which are found by the loader taking into account
        default search paths and other environment variables
        (like <i>VK_LAYER_PATH</i>).
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.234 of the Vulkan headers and later.<br/>
        Disabling a layer that an application intentionally enables as an
        explicit layer <b>may</b> cause the application to not function
        properly.<br/>
        This is also checked before other layer environment variables (such as
        <i>VK_LOADER_LAYERS_ENABLE</i>) so that a user may easily disable all
        layers and then selectively re-enable individual layers using the
        enable environment variable.
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_DISABLE=*MESA*,~implicit~<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_DISABLE=*MESA*,~implicit~<br/><br/>
        The above would disable any Mesa layer and all other implicit layers
        that would normally be enabled on the system.
    </small></td>
  </tr>
  <tr>
  <td><small>
    <i>VK_LOADER_LAYERS_ALLOW</i>
    </small></td>
    <td><small>
        A comma-delimited list of globs to search for in known layers and
        used to prevent layers whose layer name matches one or more of
        the provided globs from being disabled by <i>VK_LOADER_LAYERS_DISABLE</i>.<br/>
        Known layers are those which are found by the loader taking into account
        default search paths and other environment variables
        (like <i>VK_LAYER_PATH</i>).
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.262 of the Vulkan headers and later.<br/>
        This will not cause layers to be enabled if the normal mechanism to
        enable them
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_ALLOW=*validation*,*recon*<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_LAYERS_ALLOW=*validation*,*recon*<br/><br/>
        The above would allow any layer whose name is validation or recon to be
        enabled regardless of the value of <i>VK_LOADER_LAYERS_DISABLE</i>.
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING</i>
    </small></td>
    <td><small>
        If set to "1", causes the loader to not unload dynamic libraries during vkDestroyInstance.
        This option allows leak sanitizers to have full stack traces.
    </small></td>
    <td><small>
        This functionality is only available with Loaders built with version
        1.3.259 of the Vulkan headers and later.<br/>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=1<br/>
        <br/>
        set<br/>
        &nbsp;&nbsp;VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=1<br/><br/>
    </small></td>
  </tr>
</table>

<br/>

### Deprecated Environment Variables

These environment variables are still active and supported, however support
may be removed in a future loader release.

<table style="width:100%">
  <tr>
    <th>Environment Variable</th>
    <th>Behavior</th>
    <th>Replaced By</th>
    <th>Restrictions</th>
    <th>Example Format</th>
  </tr>
  <tr>
    <td><small><i>VK_ICD_FILENAMES</i></small></td>
    <td><small>
            Force the loader to use the specific driver JSON files.
            The value contains a list of delimited full path listings to
            driver JSON Manifest files.<br/>
            <br/>
            <b>NOTE:</b> If a global path to the JSON file is not used, issues
            may be encountered.<br/>
    </small></td>
    <td><small>
        This has been replaced by <i>VK_DRIVER_FILES</i>.
    </small></td>
    <td><small>
        <a href="#elevated-privilege-caveats">
            Ignored when running Vulkan application with elevated privileges.
        </a>
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_ICD_FILENAMES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>/intel.json:<folder_b>/amd.json
        <br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_ICD_FILENAMES=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<folder_a>\nvidia.json;<folder_b>\mesa.json
    </small></td>
  </tr>
  <tr>
    <td><small>
        <i>VK_INSTANCE_LAYERS</i>
    </small></td>
    <td><small>
        Force the loader to add the given layers to the list of Enabled layers
        normally passed into <b>vkCreateInstance</b>.
        These layers are added first, and the loader will remove any duplicate
        layers that appear in both this list as well as that passed into
        <i>ppEnabledLayerNames</i>.
    </small></td>
    <td><small>
        This has been deprecated by <i>VK_LOADER_LAYERS_ENABLE</i>.
        It also overrides any layers disabled with
        <i>VK_LOADER_LAYERS_DISABLE</i>.
    </small></td>
    <td><small>
        None
    </small></td>
    <td><small>
        export<br/>
        &nbsp;&nbsp;VK_INSTANCE_LAYERS=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;layer_a&gt;;&lt;layer_b&gt;<br/><br/>
        set<br/>
        &nbsp;&nbsp;VK_INSTANCE_LAYERS=<br/>
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;layer_a&gt;;&lt;layer_b&gt;
    </small></td>
  </tr>
</table>
<br/>
<br/>

## Glossary of Terms

<table style="width:100%">
  <tr>
    <th>Field Name</th>
    <th>Field Value</th>
  </tr>
  <tr>
    <td>Android Loader</td>
    <td>The loader designed to work primarily for the Android OS.
        This is generated from a different code base than the Khronos loader.
        But, in all important aspects, it should be functionally equivalent.
    </td>
  </tr>
  <tr>
    <td>Khronos Loader</td>
    <td>The loader released by Khronos and currently designed to work primarily
        on Windows, Linux, macOS, Stadia, and Fuchsia.
        This is generated from a different
        <a href="https://github.com/KhronosGroup/Vulkan-Loader">code base</a>
        than the Android loader.
        But in all important aspects, it should be functionally equivalent.
    </td>
  </tr>
  <tr>
    <td>Core Function</td>
    <td>A function that is already part of the Vulkan core specification and not
        an extension. <br/>
        For example, <b>vkCreateDevice()</b>.
    </td>
  </tr>
  <tr>
    <td>Device Call Chain</td>
    <td>The call chain of functions followed for device functions.
        This call chain for a device function is usually as follows: first the
        application calls into a loader trampoline, then the loader trampoline
        calls enabled layers, and the final layer calls into the driver specific
        to the device. <br/>
        See the
        <a href="#dispatch-tables-and-call-chains">Dispatch Tables and Call
        Chains</a> section for more information.
    </td>
  </tr>
  <tr>
    <td>Device Function</td>
    <td>A device function is any Vulkan function which takes a <i>VkDevice</i>,
        <i>VkQueue</i>, <i>VkCommandBuffer</i>, or any child of these, as its
        first parameter. <br/><br/>
        Some Vulkan device functions are: <br/>
        &nbsp;&nbsp;<b>vkQueueSubmit</b>, <br/>
        &nbsp;&nbsp;<b>vkBeginCommandBuffer</b>, <br/>
        &nbsp;&nbsp;<b>vkCreateEvent</b>. <br/><br/>
        See the <a href="#instance-versus-device">Instance Versus Device</a>
        section for more information.
    </td>
  </tr>
  <tr>
    <td>Discovery</td>
    <td>The process of the loader searching for driver and layer files to set up
        the internal list of Vulkan objects available.<br/>
        On <i>Windows/Linux/macOS</i>, the discovery process typically focuses
        on searching for Manifest files.<br/>
        On <i>Android</i>, the process focuses on searching for library files.
    </td>
  </tr>
  <tr>
    <td>Dispatch Table</td>
    <td>An array of function pointers (including core and possibly extension
        functions) used to step to the next entity in a call chain.
        The entity could be the loader, a layer or a driver.<br/>
        See <a href="#dispatch-tables-and-call-chains">Dispatch Tables and Call
        Chains</a> for more information.
    </td>
  </tr>
  <tr>
    <td>Driver</td>
    <td>The underlying library which provides support for the Vulkan API.
        This support can be implemented as either an ICD, API translation
        library, or pure software.<br/>
        See <a href="#drivers">Drivers</a> section for more information.
    </td>
  </tr>
  <tr>
    <td>Extension</td>
    <td>A concept of Vulkan used to expand the core Vulkan functionality.
        Extensions may be IHV-specific, platform-specific, or more broadly
        available. <br/>
        Always first query if an extension exists, and enable it during
        <b>vkCreateInstance</b> (if it is an instance extension) or during
        <b>vkCreateDevice</b> (if it is a device extension) before attempting
        to use it. <br/>
        Extensions will always have an author prefix or suffix modifier to every
        structure, enumeration entry, command entry-point, or define that is
        associated with it.
        For example, `KHR` is the prefix for Khronos authored extensions and
        will also be found on structures, enumeration entries, and commands
        associated with those extensions.
    </td>
  </tr>
  <tr>
    <td>Extension Function</td>
    <td>A function that is defined as part of an extension and not part of the
        Vulkan core specification. <br/>
        As with the extension the function is defined as part of, it will have a
        suffix modifier indicating the author of the extension.<br/>
        Some example extension suffixes include:<br/>
        &nbsp;&nbsp;<b>KHR</b>  - For Khronos authored extensions, <br/>
        &nbsp;&nbsp;<b>EXT</b>  - For multi-company authored extensions, <br/>
        &nbsp;&nbsp;<b>AMD</b>  - For AMD authored extensions, <br/>
        &nbsp;&nbsp;<b>ARM</b>  - For ARM authored extensions, <br/>
        &nbsp;&nbsp;<b>NV</b>   - For Nvidia authored extensions.<br/>
    </td>
  </tr>
  <tr>
    <td>ICD</td>
    <td>Acronym for "Installable Client Driver".
        These are drivers that are provided by IHVs to interact with the
        hardware they provide. <br/>
        These are the most common type of Vulkan drivers. <br/>
        See <a href="#installable-client-drivers">Installable Client Drivers</a>
        section for more information.
    </td>
  </tr>
  <tr>
    <td>IHV</td>
    <td>Acronym for an "Independent Hardware Vendor".
        Typically the company that built the underlying hardware technology
        that is being used. <br/>
        A typical examples for a Graphics IHV include (but not limited to):
        AMD, ARM, Imagination, Intel, Nvidia, Qualcomm
    </td>
  </tr>
  <tr>
    <td>Instance Call Chain</td>
    <td>The call chain of functions followed for instance functions.
        This call chain for an instance function is usually as follows: first
        the application calls into a loader trampoline, then the loader
        trampoline calls enabled layers, the final layer calls a loader
        terminator, and the loader terminator calls all available
        drivers. <br/>
        See the <a href="#dispatch-tables-and-call-chains">Dispatch Tables and
        Call Chains</a> section for more information.
    </td>
  </tr>
  <tr>
    <td>Instance Function</td>
    <td>An instance function is any Vulkan function which takes as its first
        parameter either a <i>VkInstance</i> or a <i>VkPhysicalDevice</i> or
        nothing at all. <br/><br/>
        Some Vulkan instance functions are:<br/>
        &nbsp;&nbsp;<b>vkEnumerateInstanceExtensionProperties</b>, <br/>
        &nbsp;&nbsp;<b>vkEnumeratePhysicalDevices</b>, <br/>
        &nbsp;&nbsp;<b>vkCreateInstance</b>, <br/>
        &nbsp;&nbsp;<b>vkDestroyInstance</b>. <br/><br/>
        See the <a href="#instance-versus-device">Instance Versus Device</a>
        section for more information.
    </td>
  </tr>
  <tr>
    <td>Layer</td>
    <td>Layers are optional components that augment the Vulkan system.
        They can intercept, evaluate, and modify existing Vulkan functions on
        their way from the application down to the driver.<br/>
        See the <a href="#layers">Layers</a> section for more information.
    </td>
  </tr>
  <tr>
    <td>Layer Library</td>
    <td>The <b>Layer Library</b> is the group of all layers the loader is able
        to discover.
        These may include both implicit and explicit layers.
        These layers are available for use by applications unless disabled in
        some way.
        For more info, see
        <a href="LoaderLayerInterface.md#layer-layer-discovery">Layer Discovery
        </a>.
    </td>
  </tr>
  <tr>
    <td>Loader</td>
    <td>The middleware program which acts as the mediator between Vulkan
        applications, Vulkan layers, and Vulkan drivers.<br/>
        See <a href="#the-loader">The Loader</a> section for more information.
    </td>
  </tr>
  <tr>
    <td>Manifest Files</td>
    <td>Data files in JSON format used by the Khronos loader.
        These files contain specific information for either a
        <a href="LoaderLayerInterface.md#layer-manifest-file-format">Layer</a>
        or a
        <a href="LoaderDriverInterface.md#driver-manifest-file-format">Driver</a>
        and define necessary information such as where to find files and default
        settings.
    </td>
  </tr>
  <tr>
    <td>Terminator Function</td>
    <td>The last function in the instance call chain above the driver and owned
        by the loader.
        This function is required in the instance call chain because all
        instance functionality must be communicated to all drivers capable of
        receiving the call. <br/>
        See <a href="#dispatch-tables-and-call-chains">Dispatch Tables and Call
        Chains</a> for more information.
    </td>
  </tr>
  <tr>
    <td>Trampoline Function</td>
    <td>The first function in an instance or device call chain owned by the
        loader which handles the set up and proper call chain walk using the
        appropriate dispatch table.
        On device functions (in the device call chain) this function can
        actually be skipped.<br/>
        See <a href="#dispatch-tables-and-call-chains">Dispatch Tables and Call
        Chains</a> for more information.
    </td>
  </tr>
  <tr>
    <td>WSI Extension</td>
    <td>Acronym for Windowing System Integration.
        A Vulkan extension targeting a particular Windowing system and designed
        to interface between the Windowing system and Vulkan.<br/>
        See
        <a href="LoaderApplicationInterface.md#wsi-extensions">WSI Extensions</a>
        for more information.
    </td>
  </tr>
  <tr>
    <td>Exported Function</td>
    <td>A function which is intended to be obtained through the platform specific
        dynamic linker, specifically from a Driver or a Layer library.
        Functions that are required to be exported are primarily the very first
        functions the Loader calls on a Layer or Driver library. <br/>
    </td>
  </tr>
  <tr>
    <td>Exposed Function</td>
    <td>A function which is intended to be obtained through a Querying Function, such as
        `vkGetInstanceProcAddr`.
        The exact Querying Function required for a specific exposed function varies
        between Layers and Drivers, as well as between interface versions. <br/>
    </td>
  </tr>
  <tr>
    <td>Querying Functions</td>
    <td>These are functions which allow the Loader to query other functions from
        drivers and layers. These functions may be in the Vulkan API but also may be
        from the private Loader and Driver Interface or the Loader and Layer Interface. <br/>
        These functions are:
        `vkGetInstanceProcAddr`, `vkGetDeviceProcAddr`,
        `vk_icdGetInstanceProcAddr`, `vk_icdGetPhysicalDeviceProcAddr`, and
        `vk_layerGetPhysicalDeviceProcAddr`.
    </td>
  </tr>
</table>
