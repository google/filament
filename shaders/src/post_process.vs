void main() {
    // Initialize the vertex shader inputs to sensible default values.
    PostProcessVertexInputs inputs;
    initPostProcessMaterialVertex(inputs);

    inputs.normalizedUV = position.xy * 0.5 + 0.5;
    inputs.texelCoords = inputs.normalizedUV * frameUniforms.resolution.xy;

// In Vulkan and Metal, texture coords are Y-down. In OpenGL, texture coords are Y-up.
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    inputs.texelCoords.y = frameUniforms.resolution.y - inputs.texelCoords.y;
    inputs.normalizedUV.y = 1.0 - inputs.normalizedUV.y;
#endif

    gl_Position = getPosition();
    // GL convention to inverted DX convention
    gl_Position.z = gl_Position.z * -0.5 + 0.5;

    // Adjust clip-space
#if !defined(TARGET_VULKAN_ENVIRONMENT) && !defined(TARGET_METAL_ENVIRONMENT)
    // This is not needed in Vulkan or Metal because clipControl is always (1, 0)
    gl_Position.z = dot(gl_Position.zw, frameUniforms.clipControl);
#endif

    // Invoke user code
    postProcessVertex(inputs);

    vertex_uv = inputs.texelCoords;

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
}
