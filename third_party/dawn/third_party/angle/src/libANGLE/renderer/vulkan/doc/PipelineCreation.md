# Pipeline Compilation

Vulkan pipelines depend on a number of state, most of which is known only at draw time.
Unfortunately, creating pipelines is a heavy operation (in particular, converting SPIR-V to assembly
and optimizing it), and doing so at draw time has multiple draw backs, including visible hitching.

The first step taken towards alleviating this issue is to maximize ANGLE's use of available dynamic
state.  This allows ANGLE to create fewer pipelines.  Simultaneously, ANGLE keeps the number of
specialization constants to a minimum, to avoid recreating pipelines on state that might
realistically change during the lifetime of the application.

At link time, ANGLE warms up the pipeline cache by creating a few placeholder pipelines, in the hope
that at draw time the pipeline cache is hit.  Without `VK_EXT_graphics_pipeline_library`, this is
hit-or-miss.  With that extension though, ANGLE is able to create pipelines with less visible
overhead.  This document focuses on ANGLE's use of `VK_EXT_graphics_pipeline_library`.

Note that `VK_EXT_graphics_pipeline_library` divides the pipeline in four stages:

- Vertex Input
- Pre-Rasterization Shaders
- Fragment Shader
- Fragment Output

In ANGLE, the two shaders subset are currently always created together.

## At Link Time

At link time, one pipeline library corresponding to the shader stages is pre-created.  Based on
existing specialization constants and unavailability of some dynamic state, multiple variations of
this pipeline library is created.  This helps warm the pipeline cache, but also allows ANGLE to pick
these pipelines directly at draw time.  The pipelines are hashed and cached in the program
executable.

## At Draw Time

At draw time, the vertex input and fragment output pipeline libraries are created similarly to how
full pipelines are created, complete with hashing and caching them.  The cache for these pipelines
is independent of the program executable.

Then, the shaders pipeline is retrieved from the program executable's cache, and linked with the
vertex input and fragment output pipelines to quickly create a complete pipeline for rendering.
Note that creating vertex input and fragment output pipelines is relatively cheap, and as they are
shared between programs, there are also few of them.

Unfortunately, linked pipelines may not be as efficient as complete pipelines.  This largely depends
on the hardware and driver; for example, some drivers optimize the vertex shader based on the vertex
input, or implement the fragment output stage as part of the fragment shader.  To gain back the
efficiency of the pipeline, ANGLE uses background threads to compile monolithic pipelines with all
the necessary state.  Once the monolithic pipeline is ready, it's handle is swapped with the linked
pipeline.

The thread monolithic pipeline creation is orchestrated by the share group.  Currently, only one
pipeline creation job is allowed at a time.  Additionally, posting these jobs is rate limited.  This
is primarily because the app is functional with reasonable efficiency with linked pipelines, so
ANGLE avoids racing to provide monolithic pipelines as fast as possible.  Instead, it ensures
monolithic pipeline creation is inconspicuous and that it interferes as little as possible with the
other threads the application may be running on other cores.

To achieve this, each `PipelineHelper` whose handle refers to a linked pipeline holds a monolithic
pipeline creation task to be scheduled.  Every time the pipeline handle is needed (i.e. at draw
time, after a state change), `PipelineHelper::getPreferredPipeline` attempts to schedule this task
through the share group.  The share group applies the aforementioned limits and may or may not post
the task.  Eventually, future calls to `PipelineHelper::getPreferredPipeline` would end up
scheduling the task and observing its termination.  At that point, the previous handle (from linked
pipelines) is replaced by the handle created by the thread (a monolithic pipeline).
