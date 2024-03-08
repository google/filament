#if __VERSION__ == 100
vec4 fragColor;
#else
layout(location = 0) out vec4 fragColor;
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
void blendPostLightingColor(const MaterialInputs material, inout vec4 color) {
    vec4 blend = color;
#if defined(POST_LIGHTING_BLEND_MODE_OPAQUE)
    blend = material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_TRANSPARENT)
    blend = material.postLightingColor + color * (1.0 - material.postLightingColor.a);
#elif defined(POST_LIGHTING_BLEND_MODE_ADD)
    blend += material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_MULTIPLY)
    blend *= material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_SCREEN)
    blend += material.postLightingColor * (1.0 - color);
#endif
    color = mix(color, blend, material.postLightingMixFactor);
}
#endif

#if defined(BLEND_MODE_MASKED)
void applyAlphaMask(inout vec4 baseColor) {
    // Use derivatives to sharpen alpha tested edges, combined with alpha to
    // coverage to smooth the result
    baseColor.a = (baseColor.a - getMaskThreshold()) / max(fwidth(baseColor.a), 1e-3) + 0.5;
    if (baseColor.a <= getMaskThreshold()) {
        discard;
    }
}
#else
void applyAlphaMask(inout vec4 baseColor) {}
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

    applyAlphaMask(inputs.baseColor);

    fragColor = evaluateMaterial(inputs);

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR) && !defined(MATERIAL_HAS_REFLECTIONS)
    blendPostLightingColor(inputs, fragColor);
#endif

#if defined(VARIANT_HAS_FOG)
    highp vec3 view = getWorldPosition() - getWorldCameraPosition();
    view = frameUniforms.fogFromWorldMatrix * view;
    fragColor = fog(fragColor, view);
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
    if (CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP) {
        float a = fragColor.a;
        highp vec4 p = getShadowPosition(getShadowCascade());
        p.xy = p.xy * (1.0 / p.w);
        if (p.xy != saturate(p.xy)) {
            vec4 c = vec4(1.0, 0, 1.0, 1.0) * a;
            fragColor = mix(fragColor, c, 0.2);
        } else {
            highp vec2 size = vec2(textureSize(light_shadowMap, 0));
            highp int ix = int(floor(p.x * size.x));
            highp int iy = int(floor(p.y * size.y));
            float t = float((ix ^ iy) & 1) * 0.2;
            vec4 c = vec4(vec3(t * a), a);
            fragColor = mix(fragColor, c, 0.5);
        }
    }
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
