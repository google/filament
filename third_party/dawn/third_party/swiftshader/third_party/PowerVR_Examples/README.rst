.. image:: https://img.shields.io/github/tag-date/powervr-graphics/Native_SDK.svg
   :target: https://github.com/powervr-graphics/Native_SDK/releases
      
.. image:: https://img.shields.io/github/license/powervr-graphics/Native_SDK.svg
    :target: https://github.com/powervr-graphics/Native_SDK/blob/master/LICENSE.md

.. image:: https://travis-ci.org/powervr-graphics/Native_SDK.svg?branch=master
     :target: https://travis-ci.org/powervr-graphics/Native_SDK

.. figure:: ./docs/images/WelcomeGraphic.png

===============
The PowerVR SDK
===============

The PowerVR SDK is an open source codebase to help with the development of graphics applications for PowerVR and other platforms.
It consists of two main parts: the Framework and a set of examples.

The Framework is a collection of libraries that aim to make development of OpenGL ES and Vulkan real-time applications much easier by removing boilerplate code and enhancing productivity. It can be used to easily create the lowest-level parts of a graphics engine - the main loop, platform abstraction, and graphical utilities. There are even a couple of 
examples showing how it is possible to build a pseudo-engine on top of the Vulkan Framework using our PVRPfx library.

The Framework is intended to reduce the amount of boilerplate code in an application. This allows a developer to focus on the key graphics API calls for drawing to screen rather than standard application setup.

The examples themselves are mostly code samples intended to:

* Show the basics of OpenGL ES and Vulkan, with the HelloAPI and IntroducingPVRShell examples.
* Demonstrate optimal implementations for techniques relevant for PowerVR, such as our DeferredShading with Pixel Local Storage/transient attachments, or our Gaussian Blur using a Sliding Average compute shader for reducing bandwidth.
* Demonstrate how to use important extensions that may improve performance, PowerVR specific or otherwise, such as IMGFramebufferDownsample and IMGTextureFilterCubic.

The examples are built on top of the Framework and optimised so that only the relevant code is left. The examples range from setting up a simple application using the Framework to implementing more advanced graphics techniques, such as physically-based rendering.
Examples are available using Vulkan, OpenGL ES, and OpenCL.

Developers can interact with our online Community at `www.imgtec.com/developers <https://www.imgtec.com/developers/>`_.

Graphics Drivers
----------------

The PowerVR SDK does not provide OpenGL ES or Vulkan API system libraries. Please ask the platform provider for these libraries if they are not present.
PVRVFrame is provided for platforms without native support for OpenGL ES but where OpenGL is supported.

Building
--------

The PowerVR SDK uses CMake for building for any platform, and additionally uses Gradle when building for Android.
To build the SDK navigate to the root of the SDK or alternatively to the root folder for any example, create a cmake binary directory, and run CMake from this created folder. 
For Android, navigate to the ``build-android`` folder of the item to build, and run the platform specific Gradle wrapper script provided (gradlew.bat/gradlew.sh).

For detailed instructions for building the SDK, see `here <BUILD.rst>`_.

PVRVFrame
---------

PVRVFrame is included as part of the PowerVR SDK and provides a set of desktop emulation libraries for OpenGL ES wrapping desktop OpenGL allowing deployment of OpenGL ES applications on desktop Windows, macOS, and Linux which can help to speed up development times and improve productivity.
***Note:** As  PVRVFrame provides an OpenGL ES wrapper for desktop OpenGL performance and capabilities depend on the 3D acceleration present in the system.
***Note:** The PVRVFrame libraries are not intended to be a completely accurate replication of the behaviour of PowerVR hardware.

* The PVRVFrame libraries can be found pre-installed in the ``[path-to-sdk]/libs`` folder.
* If the SDK was installed using the PowerVR SDK installer, the PVRVFrame emulation libraries may already be present in the PATH environment variable if this was requested during installation.
* Otherwise, the PVRVFrame libraries can be manually added to the PATH or can be copied to a DLL-accessible directory prior to running the SDK applications - This may be anywhere in the path, next to the executable, system default folders etc. 

***Note:** To install PVRVFrame system-wide on Windows and run both 32 and 64-bit builds using the PVRVFrame libraries, copy the 64-bit version to ``%windir%\System32`` and the 32-bit version to ``%windir%\SysWOW64`` so that they are automatically selected by the corresponding applications. Otherwise, it may be necessary to manually modify the path based on which architecture needs to be run. For instance, if the 32-bit libraries are in the PATH, 64-bit applications cannot be run and vice versa.

The PVRVFrame library filenames are:

* ``libEGL.dll``     (EGL) 
* ``libGLES_CM.dll`` (OpenGL ES 1.x) 
* ``libGLESv2.dll``  (OpenGL ES 2.0 and 3.x)

Dependencies
------------

The PowerVR SDK has a number of third-party dependencies, each of which may have their own license:

- `glm <https://github.com/g-truc/glm>`_: OpenGL Mathematics
- `pugixml <https://github.com/zeux/pugixml>`_: Light-weight, simple and fast XML parser for C++ with XPath support
- `concurrentqueue <https://github.com/cameron314/concurrentqueue>`_: A fast multi-producer, multi-consumer lock-free concurrent queue for C++11
- `glslang <https://github.com/KhronosGroup/glslang>`_: Shader front end and validator
- `tinygltf <https://github.com/syoyo/tinygltf>`_: Header only C++11 tiny glTF 2.0 library
- `vma <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`_: Vulkan Memory Allocator
- `vulkan <https://github.com/KhronosGroup/Vulkan-Docs>`_: Sources for the formal documentation of the Vulkan API

Release notes
-------------

For the latest version of the Release Notes detailing what has changed in this release, please visit `Release Notes <https://www.imgtec.com/developers/powervr-sdk-tools/whats-new/>`_.
