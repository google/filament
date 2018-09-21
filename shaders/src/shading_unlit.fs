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
    if (color.a < getMaskThreshold()) {
        discard;
    }
#endif

#if defined(MATERIAL_HAS_EMISSIVE)
    color.rgb += material.emissive.rgb;
#endif

#if defined(HAS_DIRECTIONAL_LIGHTING)
#if defined(HAS_SHADOWING)
    color *= 1.0 - shadow(light_shadowMap, getLightSpacePosition());
#else
    color = vec4(0.0);
#endif
#elif defined(HAS_SHADOW_MULTIPLIER)
    color = vec4(0.0);
#endif

    return color;
}
