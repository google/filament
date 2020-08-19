void addEmissive(const MaterialInputs material, inout vec4 color) {
#if defined(MATERIAL_HAS_EMISSIVE)
    highp vec4 emissive = material.emissive;
    highp float attenuation = mix(1.0, frameUniforms.exposure, emissive.w);
    color.rgb += emissive.rgb * attenuation;
#endif
}

#if defined(BLEND_MODE_MASKED)
float computeMaskedAlpha(float a) {
    // Use derivatives to smooth alpha tested edges
    return (a - getMaskThreshold()) / max(fwidth(a), 1e-3) + 0.5;
}
#endif

/**
 * Evaluates unlit materials. In this lighting model, only the base color and
 * emissive properties are taken into account:
 *
 * finalColor = baseColor + emissive
 *
 * The emissive factor is only applied if the fragment passes the optional
 * alpha test.
 *
 * When the shadowMultiplier property is enabled on the material, the final
 * color is multiplied by the inverse light visibility to apply a shadow.
 * This is mostly useful in AR to cast shadows on unlit transparent shadow
 * receiving planes.
 */
vec4 evaluateMaterial(const MaterialInputs material) {
    vec4 color = material.baseColor;

#if defined(BLEND_MODE_MASKED)
    color.a = computeMaskedAlpha(color.a);
    if (color.a <= 0.0) {
        discard;
    }
#endif

    addEmissive(material, color);

#if defined(HAS_DIRECTIONAL_LIGHTING)
#if defined(HAS_SHADOWING)
    float visibility = 1.0;
    if ((frameUniforms.directionalShadows & 1u) != 0u) {
        uint cascade = getShadowCascade();
        uint layer = cascade;
#if defined(HAS_VSM)
        // TODO: VSM shadow multiplier
#else
        visibility = shadow(light_shadowMap, layer, getCascadeLightSpacePosition(cascade));
#endif
    }
    if ((frameUniforms.directionalShadows & 0x2u) != 0u && visibility > 0.0) {
        if (objectUniforms.screenSpaceContactShadows != 0u) {
            visibility *= (1.0 - screenSpaceContactShadow(frameUniforms.lightDirection));
        }
    }
    color *= 1.0 - visibility;
#else
    color = vec4(0.0);
#endif
#elif defined(HAS_SHADOW_MULTIPLIER)
    color = vec4(0.0);
#endif

    return color;
}
