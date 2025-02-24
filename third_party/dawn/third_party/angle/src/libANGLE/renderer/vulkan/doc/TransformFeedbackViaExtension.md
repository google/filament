# Transform Feedback via extension

## Outline

ANGLE emulates transform feedback using the vertexPipelineStoresAndAtomics features in Vulkan.
But some GPU vendors do not support these atomics. Also the emulation becomes more difficult in
GLES 3.2. Therefore ANGLE must support using the VK_EXT_transform_feedback extension.

We also expect a performance gain when we use this extension.

## Implementation of Pause/Resume using CounterBuffer

The Vulkan extension does not provide separate APIs for `glPauseTransformFeedback` /
`glEndTransformFeedback`.

Instead, Vulkan introduced Counter buffers in `vkCmdBeginTransformFeedbackEXT` /
`vkCmdEndTransformFeedbackEXT` as API parameters.

To pause, we call `vkCmdEndTransformFeedbackEXT` and provide valid buffer handles in the
`pCounterBuffers` array and valid offsets in the `pCounterBufferOffsets` array for the
implementation to save the resume points.

Then to resume, we call `vkCmdBeginTransformFeedbackEXT` with the previous `pCounterBuffers`
and `pCounterBufferOffsets` values.

Between the pause and resume there needs to be a memory barrier for the counter buffers with a
source access of `VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT` at pipeline stage
`VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT` to a destination access of
`VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT` at pipeline stage
`VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT`.

## Implementation of glTransformFeedbackVaryings

There is no equivalent function for glTransformFeedbackVaryings in Vulkan. The Vulkan specification
states that the last vertex processing stage shader must be declared with the XFB execution mode.

The SPIR-V transformer takes care of adding this execution mode, as well as decorating the variables
that need to be captured.

ANGLE modifies gl_position.z in vertex shader for the Vulkan coordinate system. So, if we capture
the value of 'gl_position' in the XFB buffer, the captured values will be incorrect. To resolve
this, we declare an internal position varying and copy the value from 'gl_position'. We capture the
internal position varying during transform feedback operation. For simplicity, we do this for every
captured varying, even though we could decorate the `gl_PerVertex` struct members in SPIR-V
directly.
