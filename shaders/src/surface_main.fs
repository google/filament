#if !defined(HAS_CUSTOM_OUTPUT)
layout(location = 0) out vec4 fragColor;
#else
// Define fragColor even with custom outputs enabled to satisfy usages. It will be removed
// during dead-code elimination.
vec4 fragColor;
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

    // See surface_shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    computeShadingParams();

    // Initialize the inputs to sensible default values, see surface_material_inputs.fs
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

#if MATERIAL_FEATURE_LEVEL > 0
#   if defined (FILAMENT_LINEAR_FOG)
    vec4 fogColor = fogLinear(view, sampler0_fog);
#   else
    vec4 fogColor = fog(view, sampler0_fog);
#   endif
#else
    vec4 fogColor = fogLinear(view);
#endif

#   if   defined(BLEND_MODE_OPAQUE)
    // nothing to do here
#   elif defined(BLEND_MODE_TRANSPARENT)
    fogColor.rgb *= fragColor.a;
#   elif defined(BLEND_MODE_ADD)
    fogColor.rgb = vec3(0.0);
#   elif defined(BLEND_MODE_MASKED)
    // nothing to do here
#   elif defined(BLEND_MODE_MULTIPLY)
    // FIXME: unclear what to do here
#   elif defined(BLEND_MODE_SCREEN)
    // FIXME: unclear what to do here
#endif
    fragColor.rgb = fragColor.rgb * (1.0 - fogColor.a) + fogColor.rgb;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(MATERIAL_HAS_LIGHTING)
    if (RUNTIME_CONFIG_HAS_DIRECTIONAL_LIGHTING && CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP) {
        float a = fragColor.a;
        highp vec4 p = getShadowPosition(getShadowCascade());
        p.xy = p.xy * (1.0 / p.w);
        if (p.xy != saturate(p.xy)) {
            vec4 c = vec4(1.0, 0, 1.0, 1.0) * a;
            fragColor = mix(fragColor, c, 0.2);
        } else {
            highp vec2 size = vec2(textureSize(sampler0_shadowMap, 0));
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
#if defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)
            unpremultiply(fragColor);
            fragColor.rgb = pow(fragColor.rgb, vec3(0.45454));
            fragColor.rgb *= fragColor.a;
#else
            fragColor.rgb = pow(fragColor.rgb, vec3(0.45454));
#endif
        }
    }
#endif


    // Some GLES drivers (e.g. Qualcomm Adreno) ignore precision qualifiers on
    // non-UBO struct members in fragment shaders, see
    // https://github.com/KhronosGroup/WebGL/issues/3351. As a result, a custom
    // output routed through `MaterialInputs` can lose precision, e.g. a
    // `highp uvec4` can be treated as a 16-bit `mediump uvec4`, truncating
    // each component to 16 bits. To work around this, a material can write
    // directly to the fragment output `FRAG_OUTPUT_AT0` and define
    // `FILAMENT_WORKAROUND_BYPASS_FRAG_OUTPUT_COPY` to skip the copy below and
    // avoid routing the writes through a struct field.
#if defined(HAS_CUSTOM_OUTPUT) && !defined(FILAMENT_WORKAROUND_BYPASS_FRAG_OUTPUT_COPY)
    FRAG_OUTPUT_AT0 = inputs.FRAG_OUTPUT0;
#endif
}
