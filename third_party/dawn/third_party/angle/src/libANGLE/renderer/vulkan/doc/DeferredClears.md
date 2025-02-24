# Deferred Clears

Take the following scenario:

1. Application binds and clears FBO1
2. Application binds FBO2 and renders to it
3. Application binds FBO1 again and renders to it

Steps 2 and 3 each require a render pass for rendering. The clear in step 1 can potentially be done
through `loadOp` of the render pass for step 3, assuming step 2 doesn't use the attachments of FBO1.
This optimization is achieved in ANGLE by deferring clears.

When a clear is issued, one of the following happens:

- If a render pass is already open, the framebuffer is cleared inline (using
  `vkCmdClearAttachments`)
- If the clear is not to the whole attachment (i.e. is scissored, or masked), a draw call is used to
  perform the clear.
- Otherwise the clear is deferred.

Deferring a clear is done by staging a `Clear` update in the `vk::ImageHelper` corresponding to the
attachment being cleared.

There are two possibilities at this point:

1. The `vk::ImageHelper` is used in any way other than as a framebuffer attachment (for example it's
   sampled from), or
2. It's used as a framebuffer attachment and rendering is done.

In scenario 1, the staged updates in the `vk::ImageHelper` are flushed. That includes the `Clear`
updates which will be done with an out-of-render-pass `vkCmdClear*Image` call.

In scenario 2, `FramebufferVk::syncState` is responsible for extracting the staged `Clear` updates,
assuming there are no subsequent updates to that subresource of the image, and keep them as
_deferred clears_. The `FramebufferVk` call that immediately follows must handle these clears one
way or another. In most cases, this implies starting a new render pass and using `loadOp`s to
perform the clear before the actual operation in that function is performed. This also implies that
the front-end must always follow a `syncState` call with a call to the backend (and for example
cannot decide to no-op the call in between). That way, the backend has a chance to flush any
deferred clears.

If the subsequent call itself is a clear operation, there are further optimizations possible. In
particular, the previously deferred clears are overridden by and/or re-deferred along with the new
clears.
