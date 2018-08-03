# 2.0.0 (2018-03-19)

A major release with many compatibility-breaking changes.

Notable new features:

- Introduction of `VmaAllocation` handle that you must retrieve from allocation functions and pass to deallocation functions next to normal `VkBuffer` and `VkImage`.
- Introduction of `VmaAllocationInfo` structure that you can retrieve from `VmaAllocation` handle to access parameters of the allocation (like `VkDeviceMemory` and offset) instead of retrieving them directly from allocation functions.
- Support for reference-counted mapping and persistently mapped allocations - see `vmaMapMemory`, `VMA_ALLOCATION_CREATE_MAPPED_BIT`.
- Support for custom memory pools - see `VmaPool` handle, `VmaPoolCreateInfo` structure, `vmaCreatePool` function.
- Support for defragmentation (compaction) of allocations - see function `vmaDefragment` and related structures.
- Support for "lost allocations" - see appropriate chapter on documentation Main Page.

# 1.0.1 (2017-07-04)

- Fixes for Linux GCC compilation.
- Changed "CONFIGURATION SECTION" to contain #ifndef so you can define these macros before including this header, not necessarily change them in the file.

# 1.0.0 (2017-06-16)

First public release.
