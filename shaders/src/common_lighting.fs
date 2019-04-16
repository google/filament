//------------------------------------------------------------------------------
// Lighting declarations
//------------------------------------------------------------------------------

#if defined(TARGET_MOBILE)
// min roughness such that (MIN_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), slightly rounded up)
#define MIN_ROUGHNESS              0.089
#define MIN_LINEAR_ROUGHNESS       0.007921
#else
#define MIN_ROUGHNESS              0.045
#define MIN_LINEAR_ROUGHNESS       0.002025
#endif

#define MAX_CLEAR_COAT_ROUGHNESS   0.6

struct Light {
    vec4 colorIntensity;  // rgb, pre-exposed intensity
    vec3 l;
    float attenuation;
    float NoL;
};

struct PixelParams {
    vec3  diffuseColor;
    float roughness;
    vec3  f0;
    float linearRoughness;
    vec3  dfg;
    vec3  energyCompensation;

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float clearCoat;
    float clearCoatRoughness;
    float clearCoatLinearRoughness;
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

float computeMicroShadowing(float NoL, float ao) {
    // Brinck and Maximov 2016, "Technical Art of Uncharted 4"
    float aperture = 2.0 * ao * ao;
    return saturate(NoL + aperture - 1.0);
}
