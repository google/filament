//------------------------------------------------------------------------------
// Lighting
//------------------------------------------------------------------------------

float computeDiffuseAlpha(float a) {
#if defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE) || defined(BLEND_MODE_MASKED)
    return a;
#else
    return 1.0;
#endif
}

#if defined(BLEND_MODE_MASKED)
float computeMaskedAlpha(float a) {
    return (a - getMaskThreshold()) / max(fwidth(a), 1e-3) + 0.5;
}
#endif

void applyAlphaMask(inout vec4 baseColor) {
#if defined(BLEND_MODE_MASKED)
    // Use derivatives to smooth alpha tested edges
    baseColor.a = computeMaskedAlpha(baseColor.a);
    if (baseColor.a <= 0.0) {
        discard;
    }
#endif
}

float geometricSpecularAntiAliasing(const vec3 n) {
    // Increase the roughness based on the curvature of the geometry to reduce
    // shading aliasing. The curvature is approximated using the derivatives
    // of the geometric normal
    vec3 ndFdx = dFdx(n);
    vec3 ndFdy = dFdy(n);
    float geometricRoughness = pow(saturate(max(dot(ndFdx, ndFdx), dot(ndFdy, ndFdy))), 0.333);
    return geometricRoughness;
}

void getCommonPixelParams(const MaterialInputs material, inout PixelParams pixel) {
    vec4 baseColor = material.baseColor;
    applyAlphaMask(baseColor);

#if defined(BLEND_MODE_FADE) && !defined(SHADING_MODEL_UNLIT)
    // Since we work in premultiplied alpha mode, we need to un-premultiply
    // in fade mode so we can apply alpha to both the specular and diffuse
    // components at the end
    unpremultiply(baseColor);
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    // This is from KHR_materials_pbrSpecularGlossiness.
    vec3 specularColor = material.specularColor;
    float metallic = computeMetallicFromSpecularColor(specularColor);

    pixel.diffuseColor = computeDiffuseColor(baseColor, metallic);
    pixel.f0 = specularColor;
#elif !defined(SHADING_MODEL_CLOTH)
    float metallic = material.metallic;
    // Assumes an interface from air to an IOR of 1.5 for dielectrics
    float reflectance = computeDielectricF0(material.reflectance);

    pixel.diffuseColor = computeDiffuseColor(baseColor, metallic);
    pixel.f0 = computeF0(baseColor, metallic, reflectance);
#else
    pixel.diffuseColor = baseColor.rgb;
    pixel.f0 = material.sheenColor;
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    pixel.subsurfaceColor = material.subsurfaceColor;
#endif
#endif
}

void getClearCoatPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(MATERIAL_HAS_CLEAR_COAT)
    pixel.clearCoat = material.clearCoat;

    // Clamp the clear coat roughness to avoid divisions by 0
    float clearCoatPerceptualRoughness = material.clearCoatRoughness;
    clearCoatPerceptualRoughness = mix(MIN_PERCEPTUAL_ROUGHNESS,
            MAX_CLEAR_COAT_PERCEPTUAL_ROUGHNESS, clearCoatPerceptualRoughness);

    pixel.clearCoatPerceptualRoughness = clearCoatPerceptualRoughness;
    pixel.clearCoatRoughness = perceptualRoughnessToRoughness(clearCoatPerceptualRoughness);
#if defined(CLEAR_COAT_IOR_CHANGE)
    // The base layer's f0 is computed assuming an interface from air to an IOR
    // of 1.5, but the clear coat layer forms an interface from IOR 1.5 to IOR
    // 1.5. We recompute f0 by first computing its IOR, then reconverting to f0
    // by using the correct interface
    pixel.f0 = mix(pixel.f0, f0ClearCoatToSurface(pixel.f0), pixel.clearCoat);
#endif
#endif
}

void getRoughnessPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float perceptualRoughness = computeRoughnessFromGlossiness(material.glossiness);
#else
    float perceptualRoughness = material.roughness;
#endif

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);

#if defined(GEOMETRIC_SPECULAR_AA_ROUGHNESS)
    float geometricRoughness = geometricSpecularAntiAliasing(shading_tangentToWorld[2]);
    perceptualRoughness = max(perceptualRoughness, geometricRoughness);
#if defined(MATERIAL_HAS_CLEAR_COAT)
    pixel.clearCoatPerceptualRoughness = max(pixel.clearCoatPerceptualRoughness, geometricRoughness);
    pixel.clearCoatRoughness = perceptualRoughnessToRoughness(pixel.clearCoatPerceptualRoughness);
#endif
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_ROUGHNESS)
    // This is a hack but it will do: the base layer must be at least as rough
    // as the clear coat layer to take into account possible diffusion by the
    // top layer
    perceptualRoughness = max(perceptualRoughness, pixel.clearCoatPerceptualRoughness);
#endif

    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.perceptualRoughness = perceptualRoughness;
    pixel.roughness = perceptualRoughnessToRoughness(perceptualRoughness);
}

void getSubsurfacePixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(SHADING_MODEL_SUBSURFACE)
    pixel.subsurfacePower = material.subsurfacePower;
    pixel.subsurfaceColor = material.subsurfaceColor;
    pixel.thickness = saturate(material.thickness);
#endif
}

void getAnisotropyPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 direction = material.anisotropyDirection;
    pixel.anisotropy = material.anisotropy;
    pixel.anisotropicT = normalize(shading_tangentToWorld * direction);
    pixel.anisotropicB = normalize(cross(shading_tangentToWorld[2], pixel.anisotropicT));
#endif
}

void getEnergyCompensationPixelParams(inout PixelParams pixel) {
    // Pre-filtered DFG term used for image-based lighting
    pixel.dfg = prefilteredDFG(pixel.perceptualRoughness, shading_NoV);

#if defined(USE_MULTIPLE_SCATTERING_COMPENSATION) && !defined(SHADING_MODEL_CLOTH)
    // Energy compensation for multiple scattering in a microfacet model
    // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
    pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);
#else
    pixel.energyCompensation = vec3(1.0);
#endif
}

/**
 * Computes all the parameters required to shade the current pixel/fragment.
 * These parameters are derived from the MaterialInputs structure computed
 * by the user's material code.
 *
 * This function is also responsible for discarding the fragment when alpha
 * testing fails.
 */
void getPixelParams(const MaterialInputs material, out PixelParams pixel) {
    getCommonPixelParams(material, pixel);
    getClearCoatPixelParams(material, pixel);
    getRoughnessPixelParams(material, pixel);
    getSubsurfacePixelParams(material, pixel);
    getAnisotropyPixelParams(material, pixel);
    getEnergyCompensationPixelParams(pixel);
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
    evaluateDirectionalLight(material, pixel, color);
#endif

#if defined(HAS_DYNAMIC_LIGHTING)
    evaluatePunctualLights(pixel, color);
#endif

#if defined(BLEND_MODE_FADE) && !defined(SHADING_MODEL_UNLIT)
    // In fade mode we un-premultiply baseColor early on, so we need to
    // premultiply again at the end (affects diffuse and specular lighting)
    color *= material.baseColor.a;
#endif

    return vec4(color, computeDiffuseAlpha(material.baseColor.a));
}

void addEmissive(const MaterialInputs material, inout vec4 color) {
#if defined(MATERIAL_HAS_EMISSIVE)
    // The emissive property applies independently of the shading model
    // It is defined as a color + exposure compensation
    highp vec4 emissive = material.emissive;
    highp float attenuation = computePreExposedIntensity(
            pow(2.0, frameUniforms.ev100 + emissive.w - 3.0), frameUniforms.exposure);
    color.rgb += emissive.rgb * attenuation;
#endif
}

/**
 * Evaluate lit materials. The actual shading model used to do so is defined
 * by the function surfaceShading() found in shading_model_*.fs.
 *
 * Returns a pre-exposed HDR RGBA color in linear space.
 */
vec4 evaluateMaterial(const MaterialInputs material) {
    vec4 color = evaluateLights(material);
    addEmissive(material, color);
    return color;
}
