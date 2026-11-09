struct Light {
    vec4 colorIntensity;  // rgb, pre-exposed intensity
    vec3 l;
    float attenuation;
    highp vec3 worldPosition;
    float NoL;
    highp vec3 direction;
    float zLight;
    bool castsShadows;
    bool contactShadows;
    uint lightType;
    int shadowIndex;
    int channels;
};

struct PixelParams {
    vec3 diffuseColor;
    float perceptualRoughness;
#if defined(MATERIAL_HAS_REFRACTION)
    float perceptualRoughnessUnclamped;
#endif
    vec3 f0;
#if defined(MATERIAL_HAS_SPECULAR_COLOR_FACTOR) || defined(MATERIAL_HAS_SPECULAR_FACTOR)
    float f90;
    float specular;
#endif
    float roughness;
    vec3 dfg;
    vec3 energyCompensation;

#if defined(MATERIAL_HAS_SECOND_SPECULAR_LOBE)
    float secondRoughness;
    float secondPerceptualRoughness;
    vec2 secondDfg;
    vec3 secondEnergyCompensation;
    float secondRoughnessWeight;
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float clearCoat;
    float clearCoatPerceptualRoughness;
    float clearCoatRoughness;
#endif

#if defined(MATERIAL_HAS_SHEEN_COLOR)
    vec3 sheenColor;
#if !defined(SHADING_MODEL_CLOTH)
    float sheenRoughness;
    float sheenPerceptualRoughness;
    float sheenScaling;
    float sheenDFG;
#endif
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 anisotropicT;
    vec3 anisotropicB;
    float anisotropy;
#endif

#if defined(MATERIAL_HAS_RETROREFLECTION)
    float retroreflection;
#endif

#if defined(MATERIAL_HAS_DIFFUSE_TRANSMISSION)
    float diffuseTransmission;
    vec3 diffuseTransmissionColor;
#endif

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    float thickness;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    vec3 subsurfaceColor;
    float subsurfacePower;
#endif

#if defined(SHADING_MODEL_CLOTH) && defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3 subsurfaceColor;
#endif

#if defined(MATERIAL_HAS_REFRACTION)
    float etaRI;
    float etaIR;
#if defined(MATERIAL_HAS_DISPERSION) && (REFRACTION_TYPE == REFRACTION_TYPE_SOLID)
    float dispersion;
#endif
    float transmission;
    float uThickness;
    vec3 absorption;
#endif
};

float computeMicroShadowing(float NoL, float visibility) {
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - min(visibility, 0.9999));
    float microShadow = saturate(NoL * aperture);
    return microShadow * microShadow;
}

#if defined(GEOMETRIC_SPECULAR_AA)
float computeKernelRoughness(const vec3 worldNormal) {
    vec3 du = dFdx(worldNormal);
    vec3 dv = dFdy(worldNormal);

    float variance = materialParams._specularAntiAliasingVariance *
            // specular AA factor to correct for resolution scaling (DSR and TAAx4)
            dot(vec2(dot(du, du), dot(dv, dv)), sq(frameUniforms.derivativesScale));
    return min(2.0 * variance, materialParams._specularAntiAliasingThreshold);
}

float normalFiltering(float kernelRoughness, float perceptualRoughness) {
    // Kaplanyan 2016, "Stable specular highlights"
    // Tokuyoshi 2017, "Error Reduction and Simplification for Shading Anti-Aliasing"
    // Tokuyoshi and Kaplanyan 2019, "Improved Geometric Specular Antialiasing"

    // This implementation is meant for deferred rendering in the original paper but
    // we use it in forward rendering as well (as discussed in Tokuyoshi and Kaplanyan
    // 2019). The main reason is that the forward version requires an expensive transform
    // of the half vector by the tangent frame for every light. This is therefore an
    // approximation but it works well enough for our needs and provides an improvement
    // over our original implementation based on Vlachos 2015, "Advanced VR Rendering".

    // The first half of the implementation is in computeKernelRoughness() to share
    // some of the computations across multiple lobes

    float roughness = perceptualRoughnessToRoughness(perceptualRoughness);
    float squareRoughness = saturate(sq(roughness) + kernelRoughness);

    return roughnessToPerceptualRoughness(sqrt(squareRoughness));
}
#endif

/**
 * Returns the reflected vector at the current shading point. The reflected vector
 * return by this function might be different from shading_reflected:
 * - For anisotropic material, we bend the reflection vector to simulate
 *   anisotropic indirect lighting
 * - The reflected vector may be modified to point towards the dominant specular
 *   direction to match reference renderings when the roughness increases
 */
vec3 getReflectedVector(const PixelParams pixel, const vec3 v, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropyDirection = pixel.anisotropy >= 0.0 ? pixel.anisotropicB : pixel.anisotropicT;
    vec3  anisotropicTangent  = cross(anisotropyDirection, v);
    vec3  anisotropicNormal   = cross(anisotropicTangent, anisotropyDirection);
    float  bendFactor          = abs(pixel.anisotropy) * saturate(5.0 * pixel.perceptualRoughness);
    vec3  bentNormal          = normalize(mix(n, anisotropicNormal, bendFactor));

    vec3 r = reflect(-v, bentNormal);
#else
    vec3 r = reflect(-v, n);
#endif
    return r;
}

void getAnisotropyPixelParams(const MaterialInputs material, inout PixelParams pixel) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 direction = material.anisotropyDirection;
    pixel.anisotropy = material.anisotropy;
    pixel.anisotropicT = normalize(shading_tangentToWorld * direction);
    pixel.anisotropicB = normalize(cross(getWorldGeometricNormalVector(), pixel.anisotropicT));
#endif
}

void getRoughnessPixelParams(
        const MaterialInputs material, inout PixelParams pixel, float kernelRoughness) {
#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float perceptualRoughness = computeRoughnessFromGlossiness(material.glossiness);
#else
    float perceptualRoughness = material.roughness;
#endif

#if defined(MATERIAL_HAS_SECOND_SPECULAR_LOBE)
    float secondPerceptualRoughness = material.secondRoughness;
#endif

#if defined(MATERIAL_HAS_REFRACTION)
    // This is used by the refraction code and must be saved before we apply specular AA
    pixel.perceptualRoughnessUnclamped = perceptualRoughness;
#endif

#if defined(GEOMETRIC_SPECULAR_AA)
    perceptualRoughness = normalFiltering(kernelRoughness, perceptualRoughness);
#if defined(MATERIAL_HAS_SECOND_SPECULAR_LOBE)
    secondPerceptualRoughness = normalFiltering(kernelRoughness, secondPerceptualRoughness);
#endif
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_ROUGHNESS)
    // This is a hack but it will do: the base layer must be at least as rough
    // as the clear coat layer to take into account possible diffusion by the
    // top layer
    float basePerceptualRoughness = max(perceptualRoughness, pixel.clearCoatPerceptualRoughness);
    perceptualRoughness = mix(perceptualRoughness, basePerceptualRoughness, pixel.clearCoat);
#if defined(MATERIAL_HAS_SECOND_SPECULAR_LOBE)
    basePerceptualRoughness = max(secondPerceptualRoughness, pixel.clearCoatPerceptualRoughness);
    secondPerceptualRoughness =
            mix(secondPerceptualRoughness, basePerceptualRoughness, pixel.clearCoat);
#endif
#endif

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);

#if defined(MATERIAL_HAS_SECOND_SPECULAR_LOBE)
    pixel.secondPerceptualRoughness =
            clamp(secondPerceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    pixel.secondRoughness = perceptualRoughnessToRoughness(pixel.secondPerceptualRoughness);
    pixel.secondRoughnessWeight = material.secondRoughnessWeight;
#endif
}
