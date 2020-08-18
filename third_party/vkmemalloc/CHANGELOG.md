# 2.3.0 (2019-12-04)

Major release after a year of development in "master" branch and feature branches. Notable new features: supporting Vulkan 1.1, supporting query for memory budget.

Major changes:

- Added support for Vulkan 1.1.
    - Added member `VmaAllocatorCreateInfo::vulkanApiVersion`.
    - When Vulkan 1.1 is used, there is no need to enable VK_KHR_dedicated_allocation or VK_KHR_bind_memory2 extensions, as they are promoted to Vulkan itself.
- Added support for query for memory budget and staying within the budget.
    - Added function `vmaGetBudget`, structure `VmaBudget`. This can also serve as simple statistics, more efficient than `vmaCalculateStats`.
    - By default the budget it is estimated based on memory heap sizes. It may be queried from the system using VK_EXT_memory_budget extension if you use `VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT` flag and `VmaAllocatorCreateInfo::instance` member.
    - Added flag `VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT` that fails an allocation if it would exceed the budget.
- Added new memory usage options:
    - `VMA_MEMORY_USAGE_CPU_COPY` for memory that is preferably not `DEVICE_LOCAL` but not guaranteed to be `HOST_VISIBLE`.
    - `VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED` for memory that is `LAZILY_ALLOCATED`.
- Added support for VK_KHR_bind_memory2 extension:
    - Added `VMA_ALLOCATION_CREATE_DONT_BIND_BIT` flag that lets you create both buffer/image and allocation, but don't bind them together.
    - Added flag `VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT`, functions `vmaBindBufferMemory2`, `vmaBindImageMemory2` that let you specify additional local offset and `pNext` pointer while binding.
- Added functions `vmaSetPoolName`, `vmaGetPoolName` that let you assign string names to custom pools. JSON dump file format and VmaDumpVis tool is updated to show these names.
- Defragmentation is legal only on buffers and images in `VK_IMAGE_TILING_LINEAR`. This is due to the way it is currently implemented in the library and the restrictions of the Vulkan specification. Clarified documentation in this regard. See discussion in #59.

Minor changes:

- Made `vmaResizeAllocation` function deprecated, always returning failure.
- Made changes in the internal algorithm for the choice of memory type. Be careful! You may now get a type that is not `HOST_VISIBLE` or `HOST_COHERENT` if it's not stated as always ensured by some `VMA_MEMORY_USAGE_*` flag.
- Extended VmaReplay application with more detailed statistics printed at the end.
- Added macros `VMA_CALL_PRE`, `VMA_CALL_POST` that let you decorate declarations of all library functions if you want to e.g. export/import them as dynamically linked library.
- Optimized `VmaAllocation` objects to be allocated out of an internal free-list allocator. This makes allocation and deallocation causing 0 dynamic CPU heap allocations on average.
- Updated recording CSV file format version to 1.8, to support new functions.
- Many additions and fixes in documentation. Many compatibility fixes for various compilers and platforms. Other internal bugfixes, optimizations, updates, refactoring...

# 2.2.0 (2018-12-13)

Major release after many months of development in "master" branch and feature branches. Notable new features: defragmentation of GPU memory, buddy algorithm, convenience functions for sparse binding.

Major changes:

- New, more powerful defragmentation:
  - Added structure `VmaDefragmentationInfo2`, functions `vmaDefragmentationBegin`, `vmaDefragmentationEnd`.
  - Added support for defragmentation of GPU memory.
  - Defragmentation of CPU memory now uses `memmove`, so it can move data to overlapping regions.
  - Defragmentation of CPU memory is now available for memory types that are `HOST_VISIBLE` but not `HOST_COHERENT`.
  - Added structure member `VmaVulkanFunctions::vkCmdCopyBuffer`.
  - Major internal changes in defragmentation algorithm.
  - VmaReplay: added parameters: `--DefragmentAfterLine`, `--DefragmentationFlags`.
  - Old interface (structure `VmaDefragmentationInfo`, function `vmaDefragment`) is now deprecated.
- Added buddy algorithm, available for custom pools - flag `VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT`.
- Added convenience functions for multiple allocations and deallocations at once, intended for sparse binding resources - functions `vmaAllocateMemoryPages`, `vmaFreeMemoryPages`.
- Added function that tries to resize existing allocation in place: `vmaResizeAllocation`.
- Added flags for allocation strategy: `VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT`, `VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT`, `VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT`, and their aliases: `VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT`, `VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT`, `VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT`.

Minor changes:

- Changed behavior of allocation functions to return `VK_ERROR_VALIDATION_FAILED_EXT` when trying to allocate memory of size 0, create buffer with size 0, or image with one of the dimensions 0.
- VmaReplay: Added support for Windows end of lines.
- Updated recording CSV file format version to 1.5, to support new functions.
- Internal optimization: using read-write mutex on some platforms.
- Many additions and fixes in documentation. Many compatibility fixes for various compilers. Other internal bugfixes, optimizations, refactoring, added more internal validation...

# 2.1.0 (2018-09-10)

Minor bugfixes.

# 2.1.0-beta.1 (2018-08-27)

Major release after many months of development in "development" branch and features branches. Many new features added, some bugs fixed. API stays backward-compatible.

Major changes:

- Added linear allocation algorithm, accessible for custom pools, that can be used as free-at-once, stack, double stack, or ring buffer. See "Linear allocation algorithm" documentation chapter.
  - Added `VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT`, `VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT`.
- Added feature to record sequence of calls to the library to a file and replay it using dedicated application. See documentation chapter "Record and replay".
  - Recording: added `VmaAllocatorCreateInfo::pRecordSettings`.
  - Replaying: added VmaReplay project.
  - Recording file format: added document "docs/Recording file format.md".
- Improved support for non-coherent memory.
  - Added functions: `vmaFlushAllocation`, `vmaInvalidateAllocation`.
  - `nonCoherentAtomSize` is now respected automatically.
  - Added `VmaVulkanFunctions::vkFlushMappedMemoryRanges`, `vkInvalidateMappedMemoryRanges`.
- Improved debug features related to detecting incorrect mapped memory usage. See documentation chapter "Debugging incorrect memory usage".
  - Added debug macro `VMA_DEBUG_DETECT_CORRUPTION`, functions `vmaCheckCorruption`, `vmaCheckPoolCorruption`.
  - Added debug macro `VMA_DEBUG_INITIALIZE_ALLOCATIONS` to initialize contents of allocations with a bit pattern.
  - Changed behavior of `VMA_DEBUG_MARGIN` macro - it now adds margin also before first and after last allocation in a block.
- Changed format of JSON dump returned by `vmaBuildStatsString` (not backward compatible!).
  - Custom pools and memory blocks now have IDs that don't change after sorting.
  - Added properties: "CreationFrameIndex", "LastUseFrameIndex", "Usage".
  - Changed VmaDumpVis tool to use these new properties for better coloring.
  - Changed behavior of `vmaGetAllocationInfo` and `vmaTouchAllocation` to update `allocation.lastUseFrameIndex` even if allocation cannot become lost.

Minor changes:

- Changes in custom pools:
  - Added new structure member `VmaPoolStats::blockCount`.
  - Changed behavior of `VmaPoolCreateInfo::blockSize` = 0 (default) - it now means that pool may use variable block sizes, just like default pools do.
- Improved logic of `vmaFindMemoryTypeIndex` for some cases, especially integrated GPUs.
- VulkanSample application: Removed dependency on external library MathFu. Added own vector and matrix structures.
- Changes that improve compatibility with various platforms, including: Visual Studio 2012, 32-bit code, C compilers.
  - Changed usage of "VK_KHR_dedicated_allocation" extension in the code to be optional, driven by macro `VMA_DEDICATED_ALLOCATION`, for compatibility with Android.
- Many additions and fixes in documentation, including description of new features, as well as "Validation layer warnings".
- Other bugfixes.

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
