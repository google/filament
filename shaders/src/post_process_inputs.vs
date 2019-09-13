LAYOUT_LOCATION(LOCATION_POSITION) in vec4 position;

LAYOUT_LOCATION(LOCATION_UVS) out vec2 vertex_uv;

struct PostProcessVertexInputs {

    // We provide normalized and non-normalized texture coordinates to custom vertex shaders.
    // By default the non-normalized coordinates are passed through to the fragment shader.
    vec2 normalizedUV;
    vec2 texelCoords;

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
