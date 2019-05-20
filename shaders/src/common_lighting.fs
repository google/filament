//------------------------------------------------------------------------------
// Lighting declarations
//------------------------------------------------------------------------------

#if defined(TARGET_MOBILE)
// min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
#define MIN_PERCEPTUAL_ROUGHNESS 0.089
#define MIN_ROUGHNESS            0.007921
#else
#define MIN_PERCEPTUAL_ROUGHNESS 0.045
#define MIN_ROUGHNESS            0.002025
#endif

#define MAX_CLEAR_COAT_PERCEPTUAL_ROUGHNESS 0.6

struct Light {
    vec4 colorIntensity;  // rgb, pre-exposed intensity
    vec3 l;
    float attenuation;
    float NoL;
};

struct PixelParams {
    vec3  diffuseColor;
    float perceptualRoughness;
    vec3  f0;
    float roughness;
    vec3  dfg;
    vec3  energyCompensation;

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float clearCoat;
    float clearCoatPerceptualRoughness;
    float clearCoatRoughness;
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropicT;
    vec3  anisotropicB;
    float anisotropy;
#endif

#if defined(SHADING_MODEL_SUBSURFACE)
    float thickness;
    vec3  subsurfaceColor;
    float subsurfacePower;
#endif

#if defined(SHADING_MODEL_CLOTH) && defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3  subsurfaceColor;
#endif
};

vec3 computeDiffuseColor(const vec4 baseColor, float metallic) {
    return baseColor.rgb * (1.0 - metallic);
}

vec3 computeF0(const vec4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

float computeMetallicFromSpecularColor(const vec3 specularColor) {
    return max3(specularColor);
}

float computeRoughnessFromGlossiness(float glossiness) {
    return 1.0 - glossiness;
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

float computeMicroShadowing(float NoL, float visibility) {
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - visibility);
    float microShadow = saturate(NoL * aperture);
    return microShadow * microShadow;
}
