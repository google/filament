LAYOUT_LOCATION(LOCATION_POSITION) in vec4 position;

LAYOUT_LOCATION(LOCATION_UVS) out vec2 vertex_uv;

struct PostProcessVertexInputs {
    vec2 uv;
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
