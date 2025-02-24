# Limitations

While the Validation Layers thrive to catch all undefined behavior, because the checking is done at a Vulkan Layer level, it is not possible to validate everything.

This documentation is not an exhaustive list of known limitations, but rather a general outline so people are aware of some them.

## Unimplementable Validation

The Validation Layers use the [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid) to know what validation is [covered](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_error_database.html). There are times where we are unable to validate some VUID, for various reasons. But we don't want to mark it as "yet to do" as there is nothing for us to actually do. The [unimplementable_validation.h](../layers/error_message/unimplementable_validation.h) file was created to list those. It lists all such VUIDs, and the reason why we cannot validate them.


## Source Code

There are things that are invalid, that can only be caught by the source language.

### pNext and sType

If an app is trying to use `VkBindImageMemoryInfo`, but is setting the `sType` as `VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO` (mixed up the buffer and image name), this might cause issues.

The Validation Layers (as well as any other layer or driver) uses the `sType` to know how to cast the `void* pNext`, so if it is wrong, there is no way a Vulkan Layer could know.

There are VUID such as `VUID-VkBufferCreateInfo-pNext-pNext` that limit which `sType` can appear in the pNext chain, but sometimes the 2 mixed up `sType` can both be valid by chance, and there will be no validation error.

### Dereferencing Pointers

There are VUs that will be worded something like `must be a valid pointer to ...`.

The Validation Layers will check that the pointer is not null, but if the pointer is pointing to garbage, there is no way to safely dereference it.

## stdlib exit()

It is possible for an app to call `exit()` and do cleanup in their `atexit()` callback.
Unfortunately, this will destroy our static allocation from under us and there is no way to detect it at runtime.
We have our own `atexit()` call set at `vkCreateDevice()` time to cleanup the layers, so we require the application to call their `atexit()` **before** `vkCreateDevice()`.

