//------------------------------------------------------------------------------
// Lighting
//------------------------------------------------------------------------------

/**
 * Computes all the parameters required to shade the current pixel/fragment.
 * These parameters are derived from the MaterialInputs structure computed
 * by the user's material code.
 *
 * This function is also responsible for discarding the fragment when alpha
 * testing fails.
 */
void getPixelParams(const MaterialInputs material, out PixelParams pixel) {
    vec4 baseColor = material.baseColor;
#if defined(BLEND_MODE_MASKED)
    // Use derivatives to smooth alpha tested edges
    baseColor.a = (baseColor.a - getMaskThreshold()) / max(fwidth(baseColor.a), 1e-3) + 0.5;
    if (baseColor.a <= 0.0) {
        discard;
    }
#endif

#if defined(BLEND_MODE_FADE) && !defined(SHADING_MODEL_UNLIT)
    // Since we work in premultiplied alpha mode, we need to un-premultiply
    // in fade mode so we can apply alpha to both the specular and diffuse
    // components at the end
    baseColor.rgb /= max(baseColor.a, FLT_EPS);
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    // This is from KHR_materials_pbrSpecularGlossiness.
    vec3 specular = material.specularColor;
    float maxSpecularComponent = max(max(specular.r, specular.g), specular.b);
    pixel.diffuseColor = baseColor.rgb * (1.0 - maxSpecularComponent);
    pixel.f0 = specular;
#elif !defined(SHADING_MODEL_CLOTH)
    float metallic = material.metallic;
    float reflectance = material.reflectance;

    pixel.diffuseColor = (1.0 - metallic) * baseColor.rgb;
    // Assumes an interface from air to an IOR of 1.5 for dielectrics
    pixel.f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor.rgb * metallic;
#else
    pixel.diffuseColor = baseColor.rgb;
    pixel.f0 = material.sheenColor;
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    pixel.subsurfaceColor = material.subsurfaceColor;
#endif
#endif

    // Clamp the roughness to a minimum value to avoid divisions by 0 in the
    // lighting code
#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float roughness = 1.0 - material.glossiness;
#else
    float roughness = material.roughness;
#endif
    roughness = clamp(roughness, MIN_ROUGHNESS, 1.0);

#if defined(GEOMETRIC_SPECULAR_AA_ROUGHNESS)
    // Increase the roughness based on the curvature of the geometry to reduce
    // shading aliasing. The curvature is approximated using the derivatives
    // of the geometric normal
    vec3 ndFdx = dFdx(shading_tangentToWorld[2]);
    vec3 ndFdy = dFdy(shading_tangentToWorld[2]);
    float geometricRoughness = pow(saturate(max(dot(ndFdx, ndFdx), dot(ndFdy, ndFdy))), 0.333);
    roughness = max(roughness, geometricRoughness);
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    pixel.clearCoat = material.clearCoat;
    // Clamp the clear coat roughness to avoid divisions by 0
    float clearCoatRoughness = material.clearCoatRoughness;
    clearCoatRoughness = mix(MIN_ROUGHNESS, MAX_CLEAR_COAT_ROUGHNESS, clearCoatRoughness);
#if defined(GEOMETRIC_SPECULAR_AA_ROUGHNESS)
    clearCoatRoughness = max(clearCoatRoughness, geometricRoughness);
#endif
    // Remap the roughness to perceptually linear roughness
    pixel.clearCoatRoughness = clearCoatRoughness;
    pixel.clearCoatLinearRoughness = clearCoatRoughness * clearCoatRoughness;
#if defined(CLEAR_COAT_IOR_CHANGE)
    // The base layer's f0 is computed assuming an interface from air to an IOR
    // of 1.5, but the clear coat layer forms an interface from IOR 1.5 to IOR
    // 1.5. We recompute f0 by first computing its IOR, then reconverting to f0
    // by using the correct interface
    pixel.f0 = mix(pixel.f0, f0ClearCoatToSurface(pixel.f0), pixel.clearCoat);
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT_ROUGHNESS)
    // This is a hack but it will do: the base layer must be at least as rough
    // as the clear coat layer to take into account possible diffusion by the
    // top layer
    roughness = max(roughness, pixel.clearCoatRoughness);
#endif
#endif

    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = roughness;
    pixel.linearRoughness = roughness * roughness;

#if defined(SHADING_MODEL_SUBSURFACE)
    pixel.subsurfacePower = material.subsurfacePower;
    pixel.subsurfaceColor = material.subsurfaceColor;
    pixel.thickness = saturate(material.thickness);
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 direction = material.anisotropyDirection;
    pixel.anisotropy = material.anisotropy;
    pixel.anisotropicT = normalize(shading_tangentToWorld * direction);
    pixel.anisotropicB = normalize(cross(shading_tangentToWorld[2], pixel.anisotropicT));
#endif

    // Pre-filtered DFG term used for image-based lighting
    pixel.dfg = prefilteredDFG(pixel.roughness, shading_NoV);

#if defined(USE_MULTIPLE_SCATTERING_COMPENSATION) && !defined(SHADING_MODEL_CLOTH)
    // Energy compensation for multiple scattering in a microfacet model
    // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
    pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);
#else
    pixel.energyCompensation = vec3(1.0);
#endif
}

float getDiffuseAlpha(float a) {
#if defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE) || defined(BLEND_MODE_MASKED)
    return a;
#else
    return 1.0;
#endif
}

/**
 * This function evaluates all lights one by one:
 * - Image based lights (IBL)
 * - Directional lights
 * - Punctual lights
 *
 * Area lights are currently not supported.
 *
 * Returns a pre-exposed HDR RGBA color in linear space.
 */
vec4 evaluateLights(const MaterialInputs material) {
    PixelParams pixel;
    getPixelParams(material, pixel);

    // Ideally we would keep the diffuse and specular components separate
    // until the very end but it costs more ALUs on mobile. The gains are
    // currently not worth the extra operations
    vec3 color = vec3(0.0);

    // We always evaluate the IBL as not having one is going to be uncommon,
    // it also saves 1 shader variant
    evaluateIBL(material, pixel, color);

#if defined(HAS_DIRECTIONAL_LIGHTING)
    evaluateDirectionalLight(pixel, color);
#endif

#if defined(HAS_DYNAMIC_LIGHTING)
    evaluatePunctualLights(pixel, color);
#endif

#if defined(BLEND_MODE_FADE) && !defined(SHADING_MODEL_UNLIT)
    // In fade mode we un-premultiply baseColor early on, so we need to
    // premultiply again at the end (affects diffuse and specular lighting)
    color *= material.baseColor.a;
#endif
    return vec4(color, getDiffuseAlpha(material.baseColor.a));
}

/**
 * Evaluate lit materials. The actual shading model used to do so is defined
 * by the function surfaceShading() found in shading_model_*.fs.
 *
 * Returns a pre-exposed HDR RGBA color in linear space.
 */
vec4 evaluateMaterial(const MaterialInputs material) {
    vec4 color = evaluateLights(material);

#if defined(MATERIAL_HAS_EMISSIVE)
    // The emissive property applies independently of the shading model
    // It is defined as a color + exposure compensation
    HIGHP vec4 emissive = material.emissive;
    HIGHP float attenuation = computePreExposedIntensity(
            pow(2.0, frameUniforms.ev100 + emissive.w - 3.0), frameUniforms.exposure);
    color.rgb += emissive.rgb * attenuation;
#endif

    return color;
}
