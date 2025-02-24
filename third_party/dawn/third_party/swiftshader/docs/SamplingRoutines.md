Sampling Routines
=================

Introduction
------------

Like other modern real-time graphics APIs, Vulkan has support for [sampler objects](https://www.khronos.org/registry/vulkan/specs/1.2/html/vkspec.html#samplers) which provide the sampling state to be used by image reading and sampling instructions in the shaders. [Sampler descriptors](https://www.khronos.org/registry/vulkan/specs/1.2/html/vkspec.html#descriptorsets-sampler) contain or reference this state. The sampler descriptor is [combined](https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpSampledImage) with an image descriptor, and this combination may only be known at shader execution time.

This poses a challenge to SwiftShader's use of [dynamic code generation](Reactor.md), where we wish to specialize the sampling code for both the image's properties (most notably the format) and the sampler state. Historically, sampler state was either part of the texture objects or both the texture and sampler object were bound to a texture unit, ahead of shader execution.

JIT Trampolines
---------------

The solution is to defer code generation for the sampling instructions until shader execution. For each image sampling operation we generate a call to a C++ function which will provide the specialized routine based on the image and sampler descriptor used at run-time. Then we call the returned routine.

Note that this differs from typical JIT-compilers' use of trampoline functions in that we generate code specific to the combination of state, and adapt it to changes in state dynamically.

3-Level Caching
---------------

We cache the generated sampling routines, using the descriptors as well as the type of sampling instruction, as the key. This is done at three levels, described in reverse order for easier understanding:

L3: At the third and last level, we use a generic least-recently-used (LRU) cache, just like the caches of the pipeline stages' routines. It is protected by a mutex, which may experience high contention due to all shader worker threads needing the sampling routines.

L2: To mitigate that, there's a second-level cache which contains a 'snapshot' of the last-level cache, which can be queried concurrently without locking. The snapshot is updated at pipeline barriers. While much faster than the last-level cache's critical section, the hash table lookup is still a lot of work per sampling instruction.

L1: Often the descriptors being used don't change between executions of the sampling instruction. Which is where the first-level or '[inline](https://en.wikipedia.org/wiki/Inline_caching)' cache comes in. It is a single-entry cache implemented at the compiled sampling instruction level. Before calling out to the C++ function to retrieve the routine, we check if the sampler and image descriptor haven't changed since the last execution of the instruction. Note that this cache doesn't use the instruction type as part of the lookup key, since each sampling instruction instance gets its own inline cache.

Descriptor Identifiers
----------------------

To make testing whether the descriptor state remained the same fast, they have unique 32-bit identifiers. Note that sampler object state and image view state that is relevant to sampling routine specialization may not be unique among sampler and image view objects. For image views we're able to compress the state into the 32-bit identifier itself to avoid unnecessary recompiles.

For sampler state, which is considerably larger than 32-bit, we keep a map of it to the unique identifiers. We keep count of how many sampler objects share each identifier, so we know when we can remove the entry.

Both these 32-bit identifiers are the only thing used as the key of the first-level sampling routine cache.