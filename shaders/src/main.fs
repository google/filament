#if __VERSION__ == 100
vec4 fragColor;
#else
layout(location = 0) out vec4 fragColor;
#endif

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

void main() {
    filament_lodBias = frameUniforms.lodBias;
#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
    logical_instance_index = instance_index;
#endif

    initObjectUniforms();

    // See shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    computeShadingParams();

    // Initialize the inputs to sensible default values, see material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    fragColor = evaluateMaterial(inputs);

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR) && !defined(MATERIAL_HAS_REFLECTIONS)
    blendPostLightingColor(inputs, fragColor);
#endif

#if defined(VARIANT_HAS_FOG)
    highp vec3 view = getWorldPosition() - getWorldCameraPosition();
    view = frameUniforms.fogFromWorldMatrix * view;
    fragColor = fog(fragColor, view);
#endif

#if MATERIAL_FEATURE_LEVEL == 0
    if (CONFIG_SRGB_SWAPCHAIN_EMULATION) {
        if (frameUniforms.rec709 != 0) {
            fragColor.r = pow(fragColor.r, 0.45454);
            fragColor.g = pow(fragColor.g, 0.45454);
            fragColor.b = pow(fragColor.b, 0.45454);
        }
    }
#endif

#if __VERSION__ == 100
    gl_FragData[0] = fragColor;
#endif
}
