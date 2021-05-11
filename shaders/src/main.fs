layout(location = 0) out vec4 fragColor;

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
void blendPostLightingColor(const MaterialInputs material, inout vec4 color) {
#if defined(POST_LIGHTING_BLEND_MODE_OPAQUE)
    color = material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_TRANSPARENT)
    color = material.postLightingColor + color * (1.0 - material.postLightingColor.a);
#elif defined(POST_LIGHTING_BLEND_MODE_ADD)
    color += material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_MULTIPLY)
    color *= material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_SCREEN)
    color += material.postLightingColor * (1.0 - color);
#endif
}
#endif

#ifndef FROXEL_BUFFER_WIDTH_SHIFT
#define FROXEL_BUFFER_WIDTH_SHIFT   6u
#define FROXEL_BUFFER_WIDTH         (1u << FROXEL_BUFFER_WIDTH_SHIFT)
#define FROXEL_BUFFER_WIDTH_MASK    (FROXEL_BUFFER_WIDTH - 1u)

#define RECORD_BUFFER_WIDTH_SHIFT   5u
#define RECORD_BUFFER_WIDTH         (1u << RECORD_BUFFER_WIDTH_SHIFT)
#define RECORD_BUFFER_WIDTH_MASK    (RECORD_BUFFER_WIDTH - 1u)
#endif

void main() {
    // See shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    computeShadingParams();

    // Initialize the inputs to sensible default values, see material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    fragColor = evaluateMaterial(inputs);

#if defined(HAS_DIRECTIONAL_LIGHTING) && defined(HAS_SHADOWING)
    bool visualizeCascades = bool(frameUniforms.cascades & 0x10u);
    if (visualizeCascades) {
        fragColor.rgb *= uintToColorDebug(getShadowCascade());
    }
#endif

#if defined(HAS_FOG)
    vec3 view = getWorldPosition() - getWorldCameraPosition();
    fragColor = fog(fragColor, view);
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    blendPostLightingColor(inputs, fragColor);
#endif
}
