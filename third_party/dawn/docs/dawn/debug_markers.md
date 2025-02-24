# Debug Markers

Dawn provides debug tooling integration for each backend.

Debugging markers are exposed through this API:
```
partial GPUProgrammablePassEncoder {
    void pushDebugGroup(const char * markerLabel);
    void popDebugGroup();
    void insertDebugMarker(const char * markerLabel);
};
```

These APIs will result in silent no-ops if they are used without setting up
the execution environment properly. Each backend has a specific process
for setting up this environment.

## D3D12

Debug markers on D3D12 are implemented with the [PIX Event Runtime](https://blogs.msdn.microsoft.com/pix/winpixeventruntime/).

To enable marker functionality, you must:
1. Click the download link on https://www.nuget.org/packages/WinPixEventRuntime
2. Rename the .nupkg file to a .zip extension, then extract its contents.
3. Copy `bin\WinPixEventRuntime.dll` into the same directory as `libdawn_native.dll`.
4. Launch your application.

You may now call the debug marker APIs mentioned above and see them from your GPU debugging tool. When using your tool, it is supported to both launch your application with the debugger attached, or attach the debugger while your application is running.

D3D12 debug markers have been tested with [Microsoft PIX](https://devblogs.microsoft.com/pix/) and [Intel Graphics Frame Analyzer](https://software.intel.com/en-us/gpa/graphics-frame-analyzer).

Unfortunately, PIX's UI does does not lend itself to capturing single frame applications like tests. You must enable capture from within your application. To do this in Dawn tests, pass the --begin-capture-on-startup flag to dawn_end2end_tests.exe.

## Vulkan

Debug markers on Vulkan are implemented with [VK_EXT_debug_utils](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_debug_utils.html).

To enable marker functionality, you must launch your application from your debugging tool. Attaching to an already running application is not supported.

Vulkan markers have been tested with [RenderDoc](https://renderdoc.org/).

## Metal

Debug markers on Metal are used with the XCode debugger.

To enable marker functionality, you must launch your application from XCode and use [GPU Frame Capture](https://developer.apple.com/documentation/metal/tools_profiling_and_debugging/metal_gpu_capture).

## OpenGL

Debug markers on OpenGL are not implemented and will result in a silent no-op. This is due to low adoption of the GL_EXT_debug_marker extension in Linux device drivers.
