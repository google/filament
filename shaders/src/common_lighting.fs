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

float computeMicroShadowing(float NoL, float visibility) {
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - visibility);
    float microShadow = saturate(NoL * aperture);
    return microShadow * microShadow;
}
