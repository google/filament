# Allocators

Allocator helpers are used in the command buffer objects as a means to allocate memory for the
latter. Regardless of the inner workings of the allocation method they use, they use the same
interface in the ANGLE code.

## Allocator types

There are currently two types of allocators defined in ANGLE:

* Pool allocators; and

* Ring buffer allocators.

**ANGLE uses pool allocators by default.** To switch to ring buffer allocators, the flag
`angle_enable_vulkan_shared_ring_buffer_cmd_alloc` should be enabled in GN args. This flag appears
as `ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC` in the code, which is used to select the
allocator type.

### Common interface

In `SecondaryCommandBuffer.h`, the helper classes related to the selected allocator type are
aliased as the following:

* `SecondaryCommandMemoryAllocator`

  * This is the main allocator object used in the allocator helper classes. It is used as a type
	for some of the allocator helpers' public functions.

* `SecondaryCommandBlockPool`

  * This allocator is used in `SecondaryCommandBuffer`.

* `SecondaryCommandBlockAllocator`

  * This allocator is defined in `CommandBufferHelperCommon`, and by extension, is used in its
	derived helper classes for render pass and outside render pass command buffer helpers.


### Pool allocator helpers

_Files: `AllocatorHelperPool.cpp` and `AllocatorHelperPool.h`_

- `SecondaryCommandMemoryAllocator` -> `DedicatedCommandMemoryAllocator` -> `angle::PoolAllocator`

- `SecondaryCommandBlockPool` -> `DedicatedCommandBlockPool`

- `SecondaryCommandBlockAllocator` -> `DedicatedCommandBlockAllocator`

#### Notes

* `attachAllocator()` and `detachAllocator()` functions are no-ops for the pool allocators.

* Regarding `SecondaryCommandBlockAllocator` in pool allocators:

  * `init()` works only with pool allocators.

  * `hasAllocatorLinks()` always returns `false`.

  * `terminateLastCommandBlock()` is no-op.

### Ring buffer allocator helpers

_Files: `AllocatorHelperRing.cpp` and `AllocatorHelperRing.h`_

- `SecondaryCommandMemoryAllocator` -> `SharedCommandMemoryAllocator` -> `angle::SharedRingBufferAllocator`

- `SecondaryCommandBlockPool` -> `SharedCommandBlockPool`

- `SecondaryCommandBlockAllocator` -> `SharedCommandBlockAllocator`

#### Notes

* It can be seen that in the context's initialization and destruction, and flushing the command
  processor's commands, there are calls to attach and detach an allocator (via `attachAllocator()`
  and `detachAllocator()`). Please note that **these functions are only defined for ring buffer
  allocators**.

* Regarding `SecondaryCommandBlockAllocator` in ring buffer allocators:

  * `init()` is no-op.

  * `hasAllocatorLinks()` checks the allocator and the shared checkpoint.

  * `terminateLastCommandBlock()` is only used in ring buffer allocators.
