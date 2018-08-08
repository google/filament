//------------------------------------------------------------------------------
// Image based lighting configuration
//------------------------------------------------------------------------------

#ifndef TARGET_MOBILE
#define IBL_SPECULAR_OCCLUSION
#define IBL_OFF_SPECULAR_PEAK
#endif

// Number of spherical harmonics bands (1, 2 or 3)
#if defined(TARGET_MOBILE)
#define SPHERICAL_HARMONICS_BANDS           2
#else
#define SPHERICAL_HARMONICS_BANDS           3
#endif

// Diffuse reflectance
#define IBL_IRRADIANCE_SPHERICAL_HARMONICS  0

// Specular reflectance
#define IBL_PREFILTERED_DFG_LUT             0

#define IBL_IRRADIANCE                      IBL_IRRADIANCE_SPHERICAL_HARMONICS
#define IBL_PREFILTERED_DFG                 IBL_PREFILTERED_DFG_LUT

//------------------------------------------------------------------------------
// IBL utilities
//------------------------------------------------------------------------------

vec3 decodeDataForIBL(const vec4 data) {
#if defined(IBL_USE_RGBM)
    return decodeRGBM(data);
#else
    return data.rgb;
#endif
}

//------------------------------------------------------------------------------
// IBL prefiltered DFG term implementations
//------------------------------------------------------------------------------

vec2 PrefilteredDFG_LUT(float roughness, float NoV) {
    return textureLod(light_iblDFG, vec2(NoV, roughness), 0.0).rg;
}

/**
 * Analytical approximation of the pre-filtered DFG terms for the cloth shading
 * model. This approximation is based on the Ashikhmin distribution term and
 * the Neubelt visibility term. See brdf.fs for more details.
 */
vec2 PrefilteredDFG_Cloth(float roughness, float NoV) {
    const vec4 c0 = vec4(0.24,  0.93, 0.01, 0.20);
    const vec4 c1 = vec4(2.00, -1.30, 0.40, 0.03);

    float s = 1.0 - NoV;
    float e = s - c0.y;
    float g = c0.x * exp2(-(e * e) / (2.0 * c0.z)) + s * c0.w;
    float n = roughness * c1.x + c1.y;
    float r = max(1.0 - n * n, c1.z) * g;

    return vec2(r, r * c1.w);
}

//------------------------------------------------------------------------------
// IBL environment BRDF dispatch
//------------------------------------------------------------------------------

vec2 prefilteredDFG(float roughness, float NoV) {
#if defined(SHADING_MODEL_CLOTH)
    return PrefilteredDFG_Cloth(roughness, NoV);
#else
#if IBL_PREFILTERED_DFG == IBL_PREFILTERED_DFG_LUT
    return PrefilteredDFG_LUT(roughness, NoV);
#endif
#endif
}

//------------------------------------------------------------------------------
// IBL irradiance implementations
//------------------------------------------------------------------------------

vec3 Irradiance_SphericalHarmonics(const vec3 n) {
    return max(
          frameUniforms.iblSH[0]
#if SPHERICAL_HARMONICS_BANDS >= 2
        + frameUniforms.iblSH[1] * (n.y)
        + frameUniforms.iblSH[2] * (n.z)
        + frameUniforms.iblSH[3] * (n.x)
#endif
#if SPHERICAL_HARMONICS_BANDS >= 3
        + frameUniforms.iblSH[4] * (n.y * n.x)
        + frameUniforms.iblSH[5] * (n.y * n.z)
        + frameUniforms.iblSH[6] * (3.0 * n.z * n.z - 1.0)
        + frameUniforms.iblSH[7] * (n.z * n.x)
        + frameUniforms.iblSH[8] * (n.x * n.x - n.y * n.y)
#endif
        , 0.0);
}

//------------------------------------------------------------------------------
// IBL irradiance dispatch
//------------------------------------------------------------------------------

vec3 diffuseIrradiance(const vec3 n) {
#if IBL_IRRADIANCE == IBL_IRRADIANCE_SPHERICAL_HARMONICS
    return Irradiance_SphericalHarmonics(n);
#endif
}

//------------------------------------------------------------------------------
// IBL specular
//------------------------------------------------------------------------------

vec3 specularIrradiance(const vec3 r, float roughness) {
    // lod = nb_mips * sqrt(linear_roughness)
    // where linear_roughness = roughness^2
    // using all the mip levels requires seamless cubemap sampling
    float lod = IBL_MAX_MIP_LEVEL * roughness;
    return decodeDataForIBL(textureLod(light_iblSpecular, r, lod));
}

vec3 specularIrradiance(const vec3 r, float roughness, float offset) {
    float lod = IBL_MAX_MIP_LEVEL * roughness;
    return decodeDataForIBL(textureLod(light_iblSpecular, r, lod + offset));
}

vec3 getSpecularDominantDirection(vec3 n, vec3 r, float linearRoughness) {
#if defined(IBL_OFF_SPECULAR_PEAK)
    float s = 1.0 - linearRoughness;
    return mix(n, r, s * (sqrt(s) + linearRoughness));
#else
    return r;
#endif
}

vec3 specularDFG(const PixelParams pixel) {
#if defined(SHADING_MODEL_CLOTH)
    return pixel.f0 * pixel.dfg.x + pixel.dfg.y;
#else
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
#endif
}

//------------------------------------------------------------------------------
// IBL evaluation
//------------------------------------------------------------------------------

/**
 * Computes a specular occlusion term from the ambient occlusion term.
 */
float computeSpecularAO(float NoV, float ao, float roughness) {
#if defined(IBL_SPECULAR_OCCLUSION) && defined(MATERIAL_HAS_AMBIENT_OCCLUSION)
    return saturate(pow(NoV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao);
#else
    return 1.0;
#endif
}

/**
 * Returns the reflected vector at the current shading point. The reflected vector
 * return by this function might be different from shading_reflected:
 * - For anisotropic material, we bend the reflection vector to simulate
 *   anisotropic indirect lighting
 * - The reflected vector may be modified to point towards the dominant specular
 *   direction to match reference renderings when the roughness increases
 */
vec3 getReflectedVector(const PixelParams pixel, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropyDirection = pixel.anisotropy >= 0.0 ? pixel.anisotropicB : pixel.anisotropicT;
    vec3  anisotropicTangent  = cross(anisotropyDirection, shading_view);
    vec3  anisotropicNormal   = cross(anisotropicTangent, anisotropyDirection);
    float bendFactor          = abs(pixel.anisotropy) * saturate(5.0 * pixel.roughness);
    vec3  bentNormal          = normalize(mix(n, anisotropicNormal, bendFactor));

    vec3 r = reflect(-shading_view, bentNormal);
#else
    vec3 r = shading_reflected;
#endif
    return getSpecularDominantDirection(n, r, pixel.linearRoughness);
}

void evaluateClothIndirectDiffuseBRDF(const PixelParams pixel, inout float diffuse) {
#if defined(SHADING_MODEL_CLOTH)
    diffuse *= (1.0 - F_Schlick(0.05, 1.0, shading_NoV));
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    // Simulate subsurface scattering with a wrap diffuse term
    diffuse *= Fd_Wrap(shading_NoV, 0.5);
#endif
#endif
}

void evaluateClearCoatIBL(const PixelParams pixel, float specularAO, inout vec3 Fd, inout vec3 Fr) {
#if defined(MATERIAL_HAS_CLEAR_COAT)
#if defined(MATERIAL_HAS_NORMAL)
    // We want to use the geometric normal for the clear coat layer
    float clearCoatNoV = abs(dot(shading_tangentToWorld[2], shading_view)) + FLT_EPS;
    vec3 clearCoatR = reflect(-shading_view, shading_tangentToWorld[2]);
#else
    float clearCoatNoV = shading_NoV;
    vec3 clearCoatR = shading_reflected;
#endif
    // The clear coat layer assumes an IOR of 1.5 (4% reflectance)
    float Fc = F_Schlick(0.04, 1.0, clearCoatNoV) * pixel.clearCoat;
    float attenuation = 1.0 - Fc;
    Fr *= sq(attenuation);
    Fr += specularIrradiance(clearCoatR, pixel.clearCoatRoughness) * (specularAO * Fc);
    Fd *= attenuation;
#endif
}

void evaluateSubsurfaceIBL(const PixelParams pixel, const vec3 diffuseIrradiance,
        inout vec3 Fd, inout vec3 Fr) {
#if defined(SHADING_MODEL_SUBSURFACE)
    vec3 viewIndependent = diffuseIrradiance;
    vec3 viewDependent = specularIrradiance(-shading_view, pixel.roughness, 1.0 + pixel.thickness);
    float attenuation = (1.0 - pixel.thickness) / (2.0 * PI);
    Fd += pixel.subsurfaceColor * (viewIndependent + viewDependent) * attenuation;
#elif defined(SHADING_MODEL_CLOTH) && defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    Fd *= saturate(pixel.subsurfaceColor + shading_NoV);
#endif
}

void evaluateIBL(const MaterialInputs material, const PixelParams pixel, inout vec3 color) {
    // Apply transform here if we wanted to rotate the IBL
    vec3 n = shading_normal;
    vec3 r = getReflectedVector(pixel, n);

    float ao = material.ambientOcclusion;
    float specularAO = computeSpecularAO(shading_NoV, ao, pixel.roughness);

    // diffuse indirect
    float diffuseBRDF = Fd_Lambert() * ao;
    evaluateClothIndirectDiffuseBRDF(pixel, diffuseBRDF);

    vec3 diffuseIrradiance = diffuseIrradiance(n);
    vec3 Fd = pixel.diffuseColor * diffuseIrradiance * diffuseBRDF;

    // specular indirect
    vec3 Fr = specularDFG(pixel) * specularIrradiance(r, pixel.roughness) * specularAO;
    Fr *= pixel.energyCompensation;

    evaluateClearCoatIBL(pixel, specularAO, Fd, Fr);
    evaluateSubsurfaceIBL(pixel, diffuseIrradiance, Fd, Fr);

    // Note: iblLuminance is already premultiplied by the exposure
    color.rgb += (Fd + Fr) * frameUniforms.iblLuminance;
}
