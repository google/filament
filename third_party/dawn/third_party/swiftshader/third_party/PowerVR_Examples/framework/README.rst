PowerVR SDK Framework
=====================

Overview
--------
The PowerVR Framework is a complete framework source code. It targets Windows, Linux, and macOS desktop platforms, and Android and iOS mobile platforms. A key strength of the PowerVR Framework is that it is platform-agnostic, meaning that with the same code, it is possible to compile for different platforms without changing the source code.

The majority of the code is written in C++ and tested across different compilers (Visual Studio 2013-2017, GNU Compiler Collection, and Clang) using modern C++ style. It provides a complete framework for application development. There is also supporting per-platform code such as Objective-C code for iOS and macOS, and Java code for Android. CMake files are also supplied.

The Framework consists of libraries divided by functionality, as shown in the figure below. These modules are provided to be configured as static libraries, but they can be used differently if required.

.. figure:: ./docs/images/PowerVRFrameworkComponents.png

PVRCore
~~~~~~~
`Source code documentation <PVRCore/docs/Index.html>`__

This is the supporting code of the library for developers to use as they wish. PVRCore is also used by the rest of the Framework, so therefore all examples using any other part of the Framework should always link with PVRCore.

PVRAssets
~~~~~~~~~
`Source code documentation <PVRAssets/docs/Index.html>`__

This is the Framework’s asset code. It includes classes and helpers for scene, mesh, light, camera, animations, and asset loading code.  PVRAssets supports the loading of POD and glTF 2.0 files, PVR and PFX materials format, as well as support for a number of texture formats.

PVRShell
~~~~~~~~
`Source code documentation <PVRShell/docs/Index.html>`__

This is the native system abstraction (such as event loops, surfaces, and windows) which simplifies cross-platform compatibility. PVRShell provides useful scaffolding for cross-platform development.

PVRVk
~~~~~
`Source code documentation <PVRVk/docs/Index.html>`__

This is a Vulkan C++ wrapper providing reference-counted objects with lifetime management, strongly typed enums, and other useful functionality.

PVRUtils
~~~~~~~~
`Source code documentation <PVRUtils/docs/Index.html>`__

This contains two libraries (OpenGL ES and Vulkan) providing very convenient helpers and wrappers. These simplify common Vulkan and OpenGL tasks such as Instance-Device-Surface/Context creation, texture loading and VBO/IBO generation. The Vulkan version is written against PVRVk. Both versions each contain a version of the UIRenderer, a 2D/3D printing library that can be used for text or sprite renderering. There are similarities between the OpenGL ES and Vulkan interfaces, taking into account the core differences between the two APIs.

PVRCamera
~~~~~~~~~
`Source code documentation <PVRCamera/docs/Index.html>`__

This is the code for interfacing with the camera on mobile platforms. A dummy desktop version is provided to ease development. OpenGL ES only at present.

Building
--------
All PowerVR examples for all platforms will build the PowerVR Framework libraries they require. If they are used, or code is based upon them, then the Framework will not need to be built separately. Instead, include the relevant CMakeFiles or Gradle projects as dependencies. Any of the SDK examples can be used for this purpose.

The PowerVR SDK ships with pre-built versions of the libraries in the folder ``[SDK]/framework/bin/[Platform]``, where ``[SDK]`` is the SDK root and ``[Platform]`` is the name of the platform of interest. This is the location normally linked to.

All modules can be built separately, by navigating to ``[SDK]/framework/[ModuleName]``, where ``[ModuleName]`` is the name of the specific module of the PowerVR Framework. CMake or Gradle can be used as normal, as building the examples automatically builds the Framework.

Creating an application using the Framework
-------------------------------------------
To create a typical application, please follow these steps:

#. Create a ``CMakeLists.txt`` for the platform. This could be your own, or one of the SDK example's ``CMakeLists.txt`` to use as a base. For example: ``examples/Vulkan/Intermediate/Bumpmap/CMakeLists.txt``.

   In more detail:

   * Add include directories:
      + ``[SDK]/framework``
      + ``[SDK]/include``
   * Add CMake dependencies using ``add_subdirectory``:
      + (If Vulkan) ``[SDK]/framework/PVRVk``
      + (If Vulkan) ``[SDK]/framework/PVRUtils/Vulkan``
      + (If OpenGL ES) ``[SDK]/framework/PVRUtils/OpenGLES``
      + ``[SDK]/framework/PVRShell``
      + ``[SDK]/framework/PVRAssets``
      + ``[SDK]/framework/PVRCore``
   * Alternatively, build the Framework modules as described, and add depependencies to them with ``target_link_libraries``
   * Link against other libraries:
      + (Optional) ``[SDK]/libs/[Platform]/[Other libraries, e.g. PVRScope]``

#. (**Android only**) Create the Gradle scripts. Explaining the Gradle language is beyond the scope of this guide. It is recommended to copy the ``build-android`` folder of one of the SDK examples and working from there. The Framework modules have their own Gradle scripts so ensuring that they are built means declaring them in the ``settings.gradle`` of the example and adding dependencies in ``build.gradle``.

#. Create the application files. For a single CPP file, the includes will usually be:
     +  ``PVRShell/PVRShell.h``
     +  ``PVRUtils/PVRUtilsGles.h`` or ``PVRUtils/PVRUtilsVk.h``

#. Write the skeleton of the application. See description of PVRShell.

Guidelines and Recommendations
------------------------------
Below are a set of guidelines and recommendations to consider when using the PowerVR Framework:

* Use PVRUtils to simplify common, surprisingly complex tasks. This makes them easy, concise and understandable. These tasks include context creation, backbuffer/swapchain setup, and texture uploading. Then step through the code to understand the actual mechanics implemented. This is particularly important for Vulkan tasks that are surprisingly involved, such as texture uploading.
 
* The ``pvr::assets::Model`` class contains all the information needed for drawing, including cameras, lights, and effects. Follow a typical PowerVR SDK example such as IntroducingPVRUtils to understand its basic use, including getting information about the data layout of meshes from a model.
 
* There are many utility functions that simplify complex tasks between Assets and the underlying API. For example, the ``pvr::utils::createInputAssemblyFromXXXXXX`` functions will populate a Vulkan pipeline's vertex configuration with the correct vertex configuration of a mesh. Similarly, the ``createXXXXBufferFromXXXX`` functions will auto-generate and upload VBOs for a mesh. Browse the ``pvr::utils`` namespace for such helpers.

* Use and understand what ``pvr::utils::StructuredBufferView`` can do. It is a class that allows precise description of a Shader Interface Block, which is a UBO/SSBO definition in the shader. It then automatically calculates all the sizes, offsets, and required paddings of every single one of its members, and CPU-side layout. It aligns everything based on STD140, takes into account dynamic offsets, provides helpers for directly setting values to mapped memory, and essentially makes working with buffers much easier. All examples that use UBOs or SSBOs use the ``StructuredMemoryView`` to define and set values.
