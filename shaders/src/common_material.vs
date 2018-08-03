struct MaterialVertexInputs {
#ifdef HAS_ATTRIBUTE_COLOR
    vec4 color;
#endif
#ifdef HAS_ATTRIBUTE_UV0
    vec2 uv0;
#endif
#ifdef HAS_ATTRIBUTE_UV1
    vec2 uv1;
#endif
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
#ifdef HAS_ATTRIBUTE_TANGENTS
    vec3 worldNormal;
#endif
    vec4 worldPosition;
};

void initMaterialVertex(out MaterialVertexInputs material) {
#ifdef HAS_ATTRIBUTE_COLOR
    material.color = mesh_color;
#endif
#ifdef HAS_ATTRIBUTE_UV0
    material.uv0 = vec2(mesh_uv0.x, 1.0 - mesh_uv0.y);
#endif
#ifdef HAS_ATTRIBUTE_UV1
    material.uv1 = vec2(mesh_uv1.x, 1.0 - mesh_uv1.y);
#endif
#ifdef VARIABLE_CUSTOM0
    material.VARIABLE_CUSTOM0 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM1
    material.VARIABLE_CUSTOM1 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM2
    material.VARIABLE_CUSTOM2 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM3
    material.VARIABLE_CUSTOM3 = vec4(0.0);
#endif
    material.worldPosition = computeWorldPosition();
}
