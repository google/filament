<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2021 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Object Lifetimes Validation

The object tracking validation object tracks all Vulkan objects. Object lifetimes are validated
along with issues related to unknown objects and object destruction and cleanup.

All Vulkan dispatchable and non-dispatchable objects are tracked by this module.

This layer validates that:

- only known objects are referenced and destroyed
- lookups are performed only on objects being tracked
- objects are correctly freed/destroyed

Validation will print errors if validation checks are not correctly met and warnings if improper
reference of objects is detected.
