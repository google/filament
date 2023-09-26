void addEmissive(const MaterialInputs material, inout vec4 color) {
#if defined(MATERIAL_HAS_EMISSIVE)
    highp vec4 emissive = material.emissive;
    highp float attenuation = mix(1.0, getExposure(), emissive.w);
#if defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)
    attenuation *= color.a;
#endif
    color.rgb += emissive.rgb * attenuation;
#endif
}

vec4 fixupAlpha(vec4 color) {
#if defined(BLEND_MODE_MASKED)
    // If we reach this point in the code, we already know that the fragment is not discarded due
    // to the threshold factor. Therefore we can just output 1.0, which prevents a "punch through"
    // effect from occuring. We do this only for TRANSLUCENT views in order to prevent breakage
    // of ALPHA_TO_COVERAGE.
    return vec4(color.rgb, (frameUniforms.needsAlphaChannel == 1.0) ? 1.0 : color.a);
#else
    return color;
#endif
}

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

#if defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
#if defined(VARIANT_HAS_SHADOWING)
    float visibility = 1.0;
    int cascade = getShadowCascade();
    bool cascadeHasVisibleShadows = bool(frameUniforms.cascades & ((1 << cascade) << 8));
    bool hasDirectionalShadows = bool(frameUniforms.directionalShadows & 1);
    if (hasDirectionalShadows && cascadeHasVisibleShadows) {
        highp vec4 shadowPosition = getShadowPosition(cascade);
        visibility = shadow(true, light_shadowMap, cascade, shadowPosition, 0.0);
        // shadow far attenuation
        highp vec3 v = getWorldPosition() - getWorldCameraPosition();
        // (viewFromWorld * v).z == dot(transpose(viewFromWorld), v)
        highp float z = dot(transpose(getViewFromWorldMatrix())[2].xyz, v);
        highp vec2 p = frameUniforms.shadowFarAttenuationParams;
        visibility = 1.0 - ((1.0 - visibility) * saturate(p.x - z * z * p.y));
    }
    if ((frameUniforms.directionalShadows & 0x2) != 0 && visibility > 0.0) {
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0) {
            visibility *= (1.0 - screenSpaceContactShadow(frameUniforms.lightDirection));
        }
    }
    color *= 1.0 - visibility;
#else
    color = vec4(0.0);
#endif
#elif defined(MATERIAL_HAS_SHADOW_MULTIPLIER)
    color = vec4(0.0);
#endif

    addEmissive(material, color);

    return fixupAlpha(color);
}
