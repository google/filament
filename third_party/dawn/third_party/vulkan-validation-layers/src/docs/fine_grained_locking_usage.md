<!-- markdownlint-disable MD041 -->
<!-- Copyright 2021-2022 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Fine Grained Locking

Fine grained locking is a performance improvement for multithreaded workloads. It allows Vulkan calls from different threads to be validated in parallel, instead of being serialized by a global lock. Waiting on this lock causes performance problems for multi-threaded applications, and most Vulkan games are heavily multi-threaded.  This feature has been tested with 15+ released games and improves performance in almost all of them, and many improve by about 150%.

However, changes to locking strategy in a multi threaded program are a frequent cause of crashes, incorrect results, or deadlock. For debugging it can be disabled with the instructions below.

Currently this optimization is available for Core Validation, Best Practices,  GPU Assisted Validation and Debug Printf.  Synchronization Validation always runs with global locking. Support for it will be added in a future release.

Thread Safety, Object Lifetime, Handle Wrapping and Stateless validation have always avoided global locking and they are thus unaffected by this feature.

### Configuring Fine Grained Locking

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

Fine Grained Locking settings are managed by configuring the Validation Layer. These settings are described in the
[VK_LAYER_KHRONOS_validation](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details) document.

Fine grained locking can also be enabled and configured using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK.

### Known Limitations

Currently there is not a way to disable this setting via `VK_EXT_validation_features` or other programmatic interface. This will be addressed in a future release.

The locking in Vulkan-ValidationLayer is not intended to provide correct results for programs that violate the thread safety guidelines described in *section 3.6 Threading Behavior* of the Vulkan specification. We will attempt to fix crash bugs in the layer resulting from insufficient external synchronization, but incorrect or inconsistent error messages will be likely. Please run Thread Safety violation to find problems like this.


### Debugging Tips

As mentioned above, solving all problems found by Thread Safety validation is highly recommended before trying to use Core Validation with Fine Grained Locking. 

If you encounter a crash, deadlock or incorrect behavior, re-run with Fine Grained Locking disabled. If your problem goes away, it is most likely a problem with the layer's locking code. If it remains, then it is probably caused by something else and will require further debugging.

For crashes or deadlocks, it will be extremely helpful you provide stack traces for any threads in your program that were executing in the layer at the time the problem occurred.  In Microsoft Visual Studio, the [Parallel Stacks](https://docs.microsoft.com/en-us/visualstudio/debugger/using-the-parallel-stacks-window?view=vs-2022) window is a good way to check the status of all threads in your program.  With [gdb](https://sourceware.org/gdb/current/onlinedocs/gdb/Threads.html#Threads), you usually need to use the `info thread` and `thread` commands to view the stacks from each thread.

Replay tools such as `gfxrecon` are not always helpful when debugging problems in multi-threaded code. Because these tools usually need to serialize all vulkan commands into a single stream, they will not be able to recreate interactions between threads during replay.  Often, the only way to recreate problems is to rerun the program, but sometimes changes in timing may cause problems to only happen on some types of hardware.

Please file a [Github issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues) if you need help or have feedback on this feature. The more information you can provide about what was happening, the better we will be able to help!
