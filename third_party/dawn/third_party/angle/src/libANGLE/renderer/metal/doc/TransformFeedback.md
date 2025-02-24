# Transform Feedback implementation on Metal back-end

### Overview
- OpenGL ES 3.0 introduces Transform Feedback as a way to capture vertex outputs to buffers before
  the introduction of Compute Shader in later versions.
- Metal doesn't support Transform Feedback natively but it is possible to be emulated using Compute
  Shader or Vertex Shader to write vertex outputs to buffers directly.
- If Vertex Shader writes to buffers directly as well as to stage output (i.e. `[[position]]`,
  varying variables, ...) then the Metal runtime won't allow the `MTLRenderPipelineState` to be
  created. It is only allowed to either write to buffers or to stage output not both on Metal. This
  brings challenges to implement Transform Feedback when `GL_RASTERIZER_DISCARD` is not enabled,
  because in that case, by right OpenGL will do both the Transform Feedback and rasterization
  (feeding stage output to Fragment Shader) at the same time.

### Current implementation
- Transform Feedback will be implemented by inserting additional code snippet to write vertex's
  varying variables to buffers called XFB buffers at compilation time. The buffers' offsets are
  calculated based on `[[vertex_id]]`/`gl_VertexIndex` & `[[instance_id]]`/`gl_InstanceID`.
- When Transform Feedback ends, a memory barrier must be inserted because the XFB buffers could be
  used as vertex inputs in future draw calls. Due to Metal not supporting explicit memory barrier
  (currently only macOS 10.14 and above supports it, ARM based macOS doesn't though), the only
  reliable way to insert memory barrier currently is ending the render pass.
- In order to support Transform Feedback capturing and rasterization at the same time, the draw call
  must be split into 2 passes:
    - First pass: Vertex Shader will write captured varyings to XFB buffers.
      `MTLRenderPipelineState`'s rasterization will be disabled. This can be done in `spirv-cross`
      translation step. `spirv-cross` can convert the Vertex Shader to a `void` function,
      effectively won't produce any stage output values for Fragment Shader.
    - Second pass: Vertex Shader will write to stage output normally, but the XFB buffers writing
      snippet are disabled. Note that the Vertex Shader in this pass is essential the same as the
      first pass's, only difference is the output route (stage output vs XFB buffers). This
      effectively executes the same Vertex Shader's internal logic twice.
- If `GL_RASTERIZER_DISCARD` is enabled when Transform Feedback is enabled:
    - Only first pass above will be executed, the render pass will use 1x1 empty texture attachment
      because rasterization is not needed and small texture attachment's load & store at render
      pass's start & end boundary could be cheap. Recall that we have to end the render pass to
      enforce XFB buffers' memory barrier as mentioned above.
- If `GL_RASTERIZER_DISCARD` is enabled and Transform Feedback is NOT enabled, we cannot disable
  `MTLRenderPipelineState`'s rasterization because if doing so, Metal runtime requires the Vertex
  Shader to be a `void` function, i.e. not returning any stage output values. In order to
  work-around this:
    - `MTLRenderPipelineState`'s rasterization will still be enabled this case.
    - However, the Vertex Shader will be translated to write `(-3, -3, -3, 1)` to
      `[[position]]`/`gl_Position` variable at the end. Effectively forcing the vertex to be clipped
      and preventing it from being sent down to Fragment Shader. Note that the `(-3, -3, -3, 1)`
      writing are controlled by a specialized constant, thus it could be turned on and off base on
      `GL_RASTERIZER_DISCARD` state. It is more efficient doing this way than re-translating the
      whole shader code again using `spirv-cross` to turn it to a `void` function.

### Future improvements
- Use explicit memory barrier on macOS devices supporting it instead of ending the render pass.
- Instead of executing the same Vertex Shader's logic twice, one alternative approach is writing the
  vertex outputs to a temporary buffer. Then in second pass, copy the varyings from that buffer to
  XFB buffers. If rasterization is still enabled, then the 3rd pass will be invoked to use the
  temporary buffer as vertex input, the Vertex Shader in 3rd pass might just a simple passthrough
  shader:
    1. Original VS -> All outputs to temp buffer.
    2. Temp buffer -> Copy captured varying to XFB buffers. Could be done in a Compute Shader.
    3. Temp buffer -> VS pass through to FS for rasterization.
- However, this approach might even be slower than executing the Vertex Shader twice. Because a
  memory barrier must be inserted after 1st step. This prevents multiple draw calls with Transform
  Feedback to be parallelized. Furthermore, on iOS devices or devices not supporting explicit
  barrier, the render pass must be ended and restarted after each draw call.
- Most of the time, the application usually uses Transform Feedback with `GL_RASTERIZER_DISCARD`
  enabled, the original approach will just simply executes the Vertex Shader once and use a cheap
  1x1 render pass, thus it should be fast enough.
