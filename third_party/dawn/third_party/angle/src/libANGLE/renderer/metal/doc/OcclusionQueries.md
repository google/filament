# Occlusion queries in the Metal back-end

- OpenGL allows occlusion query to start and end at any time.
- On the other hand, Metal only allows occlusion query (called visibility result test) to begin and
  end within a render pass.
- Furthermore, a visibility result buffer must be set before encoding the render pass. This buffer
  must be allocated beforehand and will be used to store the visibility results of all queries
  within the render pass. Each query uses one offset within the buffer to store the result. Once the
  render pass's encoding starts, this buffer must not be changed.
- The visibility result buffer will always be reset to zeros within the render pass.

### Previous implementation
- Metal back-end object `RenderCommandEncoder`'s method restart() will create an instance of Metal
  framework's native object `MTLRenderCommandEncoder` immediately to start encoding a render pass.
- Afterwards, calling `RenderCommandEncoder`'s functions such as draw(), setBuffer(), setTexture(),
  etc will invoke the equivalent `MTLRenderCommandEncoder`'s methods.
- The render pass's encoding ends when `RenderCommandEncoder.endEncoding()` is called.

### Current implementation

- `MTLRenderCommandEncoder` creation will be deferred until all information about the render pass
  have been recorded and known to the Metal backend.
- Invoking `RenderCommandEncoder`'s methods such as draw(), setVisibilityResultMode(), setBuffer(),
  etc will be recorded in a back-end owned buffer instead of encoding directly into an
  `MTLRenderCommandEncoder` object.
- Whenever an occlusion query starts, an offset within a visibility result buffer will be allocated
  for this request. This offset is valid only for the current render pass. The visibility buffer's
  capacity will be extended if needed to have enough storage for all the queries within the render
  pass. The offset will be used to activate the visibility test in the render pass.
- Calling `RenderCommandEncoder.endEncoding()` will:
    - Bind the visibility result buffer allocated above.
    - Create an `MTLRenderCommandEncoder` object.
    - Encode using all render commands memorized in the back-end owned buffer.
- Immediately after `RenderCommandEncoder.endEncoding()`:
    - An extra compute shader or copying pass is added to copy the results from visibility result
      buffer to the respective assigned occlusion queries' buffers. Each query will simply copy the
      value from its respective allocated offset in the visibility buffer.
    - Note that if the query spans across multiple render passes, its old value will be accumulated
      with the result stored in the visibility result buffer instead of being replaced.
- Special cases:
    - If user calls `glClear` between `glBeginQuery` - `glEndQuery` pair, its pixels should not be
      counted by the occlusion test. To avoid this, current visibility test will end, then another
      offset in the visibility buffer will be allocated for the query, this new offset will be used
      to continue the test after the `glClear` operation ends. In the final step, the values stored
      in both the old offset and the new offset will be accumulated together.
    - If user calls `glBeginQuery` then `glEndQuery` then `glBeginQuery` again within a single pass,
      then the query will be allocated 2 offsets since Metal doesn't allow an offset to be re-used
      in a render pass. Only the value stored in the 2nd offset will be copied back to the query at
      the end of the render pass though.

### Future improvements
- One could simply allocates an offset within the visibility result buffer permanently for a query.
  Then the extra copy step at the end of the render pass could be removed.
- However, doing so means the visibility result buffer would be very large in order to store every
  query object created. Even if the query object might never be activated in a render pass.
- Furthermore, in order for the client to read back the result of a query, a host memory
  synchronization for the visibility result buffer must be inserted. This could be slow if the
  buffer is huge, and there are some offsets within the buffer are inactive within a render pass,
  thus it is a wasteful synchronization.