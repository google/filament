# gl_ClipDistance extension support in Vulkan back-end

OpenGL GLSL's `gl_ClipDistance` is supported by Vulkan. However, OpenGL supports disabling/enabling
individual `gl_ClipDistance[i]` on the API level side. Writing to `gl_ClipDistance[i]` in shader
will be ignored if it is disabled. Vulkan doesn't have any equivalent API to disable/enable the
writing, though writing to a `gl_ClipDistance[i]` variable automatically enables it.

To implement this enabling/disabling API in Vulkan back-end:

- The shader compiler will translate each `gl_ClipDistance[i]` assignment to an assignment to
  `ANGLEClipDistance[i]` variable.
- A special driver uniform variable `clipDistancesEnabled` will contain one bit flag for each
  enabled `gl_ClipDistance[i]`. This variable supports up to 32 `gl_ClipDistance` indices.
- At the end of vertex shader, the enabled `gl_ClipDistance[i]` will be assigned the respective
  value from `ANGLEClipDistance[i]`. On the other hand, those disabled elements will be assigned
  zero value. This step is described in the following code:
    ```
    for (int index : arraylength(gl_ClipDistance))
    {
        if (ANGLEUniforms.clipDistancesEnabled & (0x1 << index))
            gl_ClipDistance[index] = ANGLEClipDistance[index];
        else
            gl_ClipDistance[index] = 0;
    }
    ```
- Some minor optimizations:
    - Only those indices that are referenced in the original code will be used in if else block
      above.
    - Those elements whose index not referenced in the original code will be zeroised instead.
    - If the original code only uses up to an index < `gl_MaxClipDistances`, then
      the loop will have at most `index+1` iterations. If there is at least one index not being
      integral constant value known at compile time then declared size of `gl_ClipDistance`
      will be the loop size.
    - Finally, if the original code doesn't use `gl_ClipDistance`, then all the steps above will be
      omitted.