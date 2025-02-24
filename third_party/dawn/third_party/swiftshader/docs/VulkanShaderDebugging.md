# Vulkan Shader Debugging

SwiftShader implements a Vulkan shader debugger that uses the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol).

This debugger is still actively being developed. Please see the [Known Issues](#Known-Issues).

# Enabling

To enable the debugger functionality, SwiftShader needs to be built using the CMake `SWIFTSHADER_ENABLE_VULKAN_DEBUGGER` flag (`-DSWIFTSHADER_ENABLE_VULKAN_DEBUGGER=1`):

Once SwiftShader is built with the debugger functionality, there are two environment flags that control the runtime behavior:

* `VK_DEBUGGER_PORT` - set to an unused port number that will be used to create the DAP localhost socket. If this environment variable is not set, then the debugger functionality will not be enabled.
* `VK_WAIT_FOR_DEBUGGER` - if defined, the debugger will block on `vkCreateDevice()` until a debugger connection is established, before allowing `vkCreateDevice()` to return. This allows breakpoints to be set before execution continues.

# Connecting using Visual Studio Code

Once you have built SwiftShader with the debugger functionality enabled, and the `VK_DEBUGGER_PORT` environment variable set, you can connect to the debugger using the following Visual Studio Code `"debugServer"` [Launch Configuration](https://code.visualstudio.com/docs/editor/debugging#_launch-configurations):

```json
    {
        "name": "Vulkan Shader Debugger",
        "type": "node",
        "request": "launch",
        "debugServer": 19020,
    }
```

Note that the `"type": "node"` field is unused, but is required.

[TODO](https://issuetracker.google.com/issues/148373102): Create a Visual Studio Code extension that provides a pre-built SwiftShader driver and debugger type.

# Shader entry breakpoints

You can use the following function breakpoint names to set a breakpoint on the entry to all shaders of the corresponding shader type:
* `"VertexShader"`
* `"FragmentShader"`
* `"ComputeShader"`

# High-level Shader debugging

The debugger, will by default, automatically disassemble the SPIR-V shader code, and provide this as the source for the shader program.

However, if the shader program contains [`OpenCL.DebugInfo.100`](https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.mobile.html) debug info instructions, then the debugger will allow you to debug the high-level shader source (please see [Known Issues](#Known-Issues)).


# Known Issues

* Currently enabling the debugger dramatically affects performance for all shader invocations. We may want to just-in-time recompile shaders that are actively being debugged to keep the invocations of non-debugged shaders performant. [Tracker bug](https://issuetracker.google.com/issues/148372410)
* Support for [`OpenCL.DebugInfo.100`](https://www.khronos.org/registry/spir-v/specs/unified1/OpenCL.DebugInfo.100.mobile.html) is still in early, but active development. Many features are still incomplete.
* Shader subgroup invocations are currently presented as a single thread, with each invocation presented as `Lane N` groups in the watch window(s). This approach is still being evaluated, and may be reworked.