void main() {
    // Initialize the vertex shader inputs to sensible default values.
    PostProcessVertexInputs inputs;
    initPostProcessMaterialVertex(inputs);

    inputs.normalizedUV = position.xy * 0.5 + 0.5;

    vec4 position = getPosition();

    // GL convention to inverted DX convention
    position.z = position.z * -0.5 + 0.5;

    // Adjust clip-space
#if !defined(TARGET_VULKAN_ENVIRONMENT) && !defined(TARGET_METAL_ENVIRONMENT)
    // This is not needed in Vulkan or Metal because clipControl is always (1, 0)
    // (We don't use a dot() here because it workaround a spirv-opt optimization that in turn
    //  causes a crash on PowerVR, see #5118)
    position.z = position.z * frameUniforms.clipControl.x + position.w * frameUniforms.clipControl.y;
#endif

    // Invoke user code
    postProcessVertex(inputs);

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

    // some PowerVR drivers crash when gl_Position is written more than once
    gl_Position = position;
}
