<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2025 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Synchronization Validation

Synchronization Validation is implemented in the `VK_LAYER_KHRONOS_validation layer`. When enabled, the Synchronization Object is intended to identify resource access conflicts due to missing or incorrect synchronization operations between actions (Draw, Copy, Dispatch, Blit) reading or writing the same regions of memory.

Synchronization will ideally be run periodically after resolving any outstanding validation checks from all other validation objects, so that issues may be addressed in early stages of development.

The specific areas covered by this layer are currently tracked in the
[Synchronization Validation Project](https://github.com/KhronosGroup/Vulkan-ValidationLayers/projects/5).
Requests for additional checks can be requested by creating a [Github issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues).


## Configuring Synchronization Validation
For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

Synchronization Validation settings are managed by configuring the Validation Layer. These settings are described in the
[VK_LAYER_KHRONOS_validation](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details) document.

The `khronos_validation.validate_sync` configuration variable enables Synchronization Validation. Additional configuration settings have this naming pattern: `khronos_validation.syncval_*`.

Synchronization Validation settings can also be managed using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK.

## Synchronization Validation Functionality

### Overview

The pipelined and multi-threaded nature of Vulkan makes it particularly important for applications to correctly insert needed synchronization primitives, and for validation to diagnose unprotected memory access hazards. Synchronization reports the presence of access hazards including information to identify the Vulkan operations which are in conflict. The reported hazards are:


<table>
  <tr>
   <td>RAW
   </td>
   <td>Read-after-write
   </td>
   <td>Occurs when a subsequent operation uses the result of a previous operation without waiting for the result to be completed.
   </td>
  </tr>
  <tr>
   <td>WAR
   </td>
   <td>Write-after-read
   </td>
   <td>Occurs when a subsequent operation overwrites a memory location read by a previous operation before that operation is complete (requires only execution dependency).
   </td>
  </tr>
  <tr>
   <td>WAW
   </td>
   <td>Write-after-write
   </td>
   <td>Occurs when a subsequent operation writes to the same set of memory locations (in whole or in part) being written by a previous operation.
   </td>
  </tr>
  <tr>
   <td>WRW
   </td>
   <td>Write-racing-write
   </td>
   <td>Occurs when unsynchronized subpasses/queues perform writes to the same set of memory locations.
   </td>
  </tr>
  <tr>
   <td>RRW
   </td>
   <td>Read-racing-write
   </td>
   <td>Occurs when unsynchronized subpasses/queues perform read and write operations on the same set of memory locations.
   </td>
  </tr>
</table>



### Current Feature set

- Hazard detection for memory usage for commands within the *same* command buffer.
- Synchronization operations .
  - vkCmdPipelineBarrier.
  - vkCmdSetEvent/vkCmdWaitEvents/vkCmdResetEvent.
  - renderpass/subpass barriers.
- The `VK_KHR_synchronization2` extension
  - vkCmdPipelineBarrier2KHR
  - vkCmdSetEvent2KHR/vkCmdWaitEvents2KHR/vkCmdResetEvent2KHR.
- Image layout transition hazard and access tracking.
- Load/Store/Resolve operations within Subpasses.
- ExecuteCommands detection of hazard from or with secondary command buffers

- QueueSubmit/QueueSubmit2 time hazard detection
- Semaphore and Fence synchronization operations/effects
- Device and Queue WaitIdle support
- Dynamic Rendering support

### Known Limitations
- Does not support precise tracking of descriptors accessed by the shader (requires integration with GPU-AV)
- Hazards related to memory aliasing are not detected properly
- Indirectly accessed (indirect/indexed) buffers validated at *binding* granularity. (Every valid location assumed to be accessed.)
- Queue family ownership transfer not supported
- Host set event not supported.
- No dedicated support for sparse resources. Need to investigate which kind of support is needed.
- Host memory accesses are not tracked. Corresponding race conditions are not reported.
- Does not include implementation of multi-view renderpass support.
- Memory access checks not suppressed for VK_CULL_MODE_FRONT_AND_BACK.
- Does not include component granularity access tracking, or correctly support swizzling.

## Typical Synchronization Validation Usage

### Debugging Synchronization Validation Issues

To debug synchronization validation issues (all platforms):

- Create a debug callback with `vkCreateDebugUtilsMessengerEXT` with the `VK_DEBUG_REPORT_ERROR_BIT_EXT` set.
- Enable synchronization as noted above. On Linux and Windows this can be simplified by enabling Synchronization Validation using [Vulkan Configurator (vkconfig)](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html).
- Set a breakpoint in the debug callback and run your application in the debugger.
- The callback will be called when a `vkCmd`... command with a hazard is recorded.

On Windows, Synchronization Validation can be run using just vkconfig and the debugger without defining a callback:

*   In vkconfig.
    *   Enable Synchronization Validation.
    *   Select 'Debug Actions' 'Break' and 'Debug Output'.
*   Debug application in Visual Studio.
*   Hazard messages will appear in the debugger output window and the debugger will break (in the validation layer code)  when a `vkCmd`... command with a hazard is recorded.


### Synchronization Validation Messages

All synchronization error messages begin with SYNC-&lt;hazard name>.  The message body is constructed:


```
<cmd name>: Hazard <hazard name> <command specific details> Access info (<...>)
```


Command specific details typically include the specifics of the access within the current command. The Access info is common to all Synchronization Validation error messages. 

<table>
  <tr>
   <td><strong>Field</strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td><code>usage</code>
   </td>
   <td>The stage/access of the current command
   </td>
  </tr>
  <tr>
   <td><code>prior_usage</code>
   </td>
   <td>The stage/access of the previous (hazarded) use
   </td>
  </tr>
  <tr>
   <td><code>read_barrier</code>
   </td>
   <td>For read <code>usage</code>, the list of stages with execution barriers between <code>prior_usage</code> and <code>usage</code>
   </td>
  </tr>
  <tr>
   <td><code>write_barrier</code>
   </td>
   <td>For write <code>usage</code>, the list of stage/access (in <code>usage</code> format) with memory barriers between <code>prior_usage</code> and <code>usage</code>
   </td>
  </tr>
  <tr>
   <td><code>command</code>
   </td>
   <td>The command that performed <code>prior_usage</code>
   </td>
  </tr>
  <tr>
   <td><code>seq_no</code>
   </td>
   <td>The zero based index of <code>command</code> within the command buffer
   </td>
  </tr>
  <tr>
   <td><code>reset_no</code>
   </td>
   <td>the reset count of the command buffer <code>command</code> is recorded to
   </td>
  </tr>
</table>


### Frequently Found Issues

*   Assuming Pipeline stages are logically extended with respect to memory access barriers.  Specifying the vertex shader stage in a barrier will **not** apply to all subsequent shader stages read/write access.
*   Invalid stage/access pairs (specifying a pipeline stage for which a given access is not valid) that yield no barrier.
*   Relying on implicit subpass dependencies with `VK_SUBPASS_EXTERNAL` when memory barriers are needed.
*   Missing memory dependencies with Image Layout Transitions from pipeline barrier or renderpass Begin/Next/End operations.
*   Missing stage/access scopes for load operations, noting that color and depth/stencil are done by different stage/access.


### Debugging Tips

*   Read and write barriers in the error message can help identify the synchronization operation (either subpass dependency or pipeline barrier) with insufficient or incorrect destination stage/access masks (second scope).
*   `Access info read_barrier` and `write_barrier` values of 0, reflect the absence of any barrier, and can indicate an insufficient or incorrect source mask (first scope).
*   Insert additional barriers with stage/access `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`, `VK_ACCESS_MEMORY_READ_BIT`|`VK_ACCESS_MEMORY_WRITE_BIT` for both` src*Mask` and `dst*Mask` fields to locate missing barriers. If the inserted barrier _resolves_ a hazard, the conflicting access _happens-before_ the inserted barrier. (Be sure to delete later.)


## Synchronization blogs/articles

Synchronization Examples[ https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples)

Keeping your GPU fed without getting bitten [ https://www.youtube.com/watch?v=oF7vOTTaAh4](https://www.youtube.com/watch?v=oF7vOTTaAh4)

Yet another blog explaining Vulkan synchronization[ http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/](http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/)

A Guide to Vulkan Synchronization Validation https://www.khronos.org/news/permalink/blog-a-guide-to-vulkan-synchronization-validation

