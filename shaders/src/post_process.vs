void main() {
    // Initialize the vertex shader inputs to sensible default values.
    PostProcessVertexInputs inputs;
    initPostProcessMaterialVertex(inputs);

    inputs.uv = (position.xy * 0.5 + 0.5) * frameUniforms.resolution.xy;

    gl_Position = position;

    // Invoke user code
    postProcessVertex(inputs);

    vertex_uv = inputs.uv;

    // Handle user-defined interpolated attributes
#if defined(VARIABLE_CUSTOM0)
    VARIABLE_CUSTOM_AT0 = inputs.VARIABLE_CUSTOM0;
#endif
#if defined(VARIABLE_CUSTOM1)
    VARIABLE_CUSTOM_AT1 = inputs.VARIABLE_CUSTOM1;
#endif
#if defined(VARIABLE_CUSTOM2)
    VARIABLE_CUSTOM_AT2 = inputs.VARIABLE_CUSTOM2;
#endif
#if defined(VARIABLE_CUSTOM3)
    VARIABLE_CUSTOM_AT3 = inputs.VARIABLE_CUSTOM3;
#endif

#if defined(TARGET_METAL_ENVIRONMENT)
    // Metal texture space is vertically flipped that of OpenGL's, so flip the Y coords so we sample
    // the frame correctly. Vulkan doesn't need this fix because its clip space is mirrored
    // (the Y axis points down the screen).
    gl_Position.y = -gl_Position.y;
#endif
}
