# Fast OpenGL State Transitions

Typical OpenGL programs issue a few small state change commands between draw call commands. We want
the typical app's use case to be as fast as possible so this leads to unique performance challenges.

Vulkan is quite different from OpenGL because it requires a separate compiled
[VkPipeline][VkPipeline] for each state vector. Compiling VkPipelines is multiple orders of
magnitude slower than enabling or disabling an OpenGL render state. To speed this up we use three
levels of caching when transitioning states in the Vulkan back-end.

## L3 Cache

The outermost level is the driver's [VkPipelineCache][VkPipelineCache]. The driver
cache reduces pipeline recompilation time significantly. But even cached
pipeline recompilations are orders of magnitude slower than OpenGL state changes.

## L2 Cache

The second level cache is an ANGLE-owned hash map from OpenGL state vectors to compiled pipelines.
See [GraphicsPipelineCache][GraphicsPipelineCache] in [vk_cache_utils.h](../vk_cache_utils.h). ANGLE's
[GraphicsPipelineDesc][GraphicsPipelineDesc] class is a tightly packed description of the
current OpenGL rendering state. We also use a [xxHash](https://github.com/Cyan4973/xxHash) for the
fastest possible hash computation. The hash map speeds up state changes considerably. But it is
still significantly slower than OpenGL implementations.

## L1 Cache

To get best performance we use a transition table from each OpenGL state vector to neighbouring
state vectors. The transition table points from GraphicsPipelineCache entries directly to
neighbouring VkPipeline objects. When the application changes state the state change bits are
recorded into a compact bit mask that covers the GraphicsPipelineDesc state vector. Then on the next
draw call we scan the transition bit mask and compare the GraphicsPipelineDesc of the current state
vector and the state vector of the cached transition. With the hash map we compute a hash over the
entire state vector and then do a `memcmp` to guard against hash collisions. With the
transition table we will only compare as many bytes as were changed in the transition bit mask. By
skipping the expensive hashing and `memcmp` we can get as good or faster performance than native
OpenGL drivers.

Note that the current design of the transition table stores transitions in an unsorted list. If
applications map from one state to many this will slow down the transition time. This could be
improved in the future using a faster look up. For instance we could keep a sorted transition table
or use a small hash map for transitions.

## L0 Cache

The current active PSO is stored as a handle in the `ContextVk` for use between draws with no state
change.

[VkPipeline]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipeline.html
[VkPipelineCache]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipelineCache.html
[GraphicsPipelineCache]: https://chromium.googlesource.com/angle/angle/+/225f08bf85a368f905362cdd1366e4795680452c/src/libANGLE/renderer/vulkan/vk_cache_utils.h#498
[GraphicsPipelineDesc]: https://chromium.googlesource.com/angle/angle/+/225f08bf85a368f905362cdd1366e4795680452c/src/libANGLE/renderer/vulkan/vk_cache_utils.h#244
