<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Handle Wrapping Functionality

The handle wrapping facility is a feature of the Khronos Layer which aliases all non-dispatchable Vulkan objects with a unique identifier at object-creation time. The aliased handles are used during validation to ensure that duplicate object handles are correctly managed and tracked by the validation layers. This enables consistent and coherent validation in addition to proper operation on systems which return non-unique object handles.

**Note**:

* If you are developing Vulkan extensions which include new APIs taking one or more Vulkan dispatchable objects as parameters, you may find it necessary to disable handle-wrapping in order use the validation layers. Handle wrapping can be disabled in the Khronos validation Layer using the VkConfig utility or as described in 
[khronos_validation_layer.html](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details).

