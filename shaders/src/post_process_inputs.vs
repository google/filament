LAYOUT_LOCATION(LOCATION_POSITION) ATTRIBUTE vec4 position;

struct PostProcessVertexInputs {

    // We provide normalized texture coordinates to custom vertex shaders.
    vec2 normalizedUV;

    // vertex position, can be modified by the user code
    vec4 position;

#ifdef VARIABLE_CUSTOM0
    vec4 VARIABLE_CUSTOM0;
#endif
#ifdef VARIABLE_CUSTOM1
    vec4 VARIABLE_CUSTOM1;
#endif
#ifdef VARIABLE_CUSTOM2
    vec4 VARIABLE_CUSTOM2;
#endif
#ifdef VARIABLE_CUSTOM3
    vec4 VARIABLE_CUSTOM3;
#endif
};

void initPostProcessMaterialVertex(out PostProcessVertexInputs inputs) {
#ifdef VARIABLE_CUSTOM0
    inputs.VARIABLE_CUSTOM0 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM1
    inputs.VARIABLE_CUSTOM1 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM2
    inputs.VARIABLE_CUSTOM2 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM3
    inputs.VARIABLE_CUSTOM3 = vec4(0.0);
#endif
}
