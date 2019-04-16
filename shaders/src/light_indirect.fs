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

// Cloth DFG approximation
#define CLOTH_DFG_CHARLIE                   0

#define CLOTH_DFG                           CLOTH_DFG_CHARLIE

// IBL integration algorithm
#define IBL_INTEGRATION_PREFILTERED_CUBEMAP         0
#define IBL_INTEGRATION_IMPORTANCE_SAMPLING         1

#define IBL_INTEGRATION                             IBL_INTEGRATION_PREFILTERED_CUBEMAP

#define IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT   64

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

vec3 PrefilteredDFG_LUT(float coord, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    return textureLod(light_iblDFG, vec2(NoV, coord), 0.0).rgb;
}

//------------------------------------------------------------------------------
// IBL environment BRDF dispatch
//------------------------------------------------------------------------------

vec3 prefilteredDFG(float roughness, float NoV) {
    // PrefilteredDFG_LUT() takes a coordinate, which is sqrt(linear_roughness) = roughness
    return PrefilteredDFG_LUT(roughness, NoV);
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
    return Irradiance_SphericalHarmonics(n);
}

//------------------------------------------------------------------------------
// IBL specular
//------------------------------------------------------------------------------

vec3 prefilteredRadiance(const vec3 r, float roughness) {
    // lod = lod_count * sqrt(linear_roughness), which is the mapping used by cmgen
    // where linear_roughness = roughness^2
    // using all the mip levels requires seamless cubemap sampling
    float lod = IBL_MAX_MIP_LEVEL * roughness;
    return decodeDataForIBL(textureLod(light_iblSpecular, r, lod));
}

vec3 prefilteredRadiance(const vec3 r, float roughness, float offset) {
    float lod = IBL_MAX_MIP_LEVEL * roughness * roughness;
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
    return pixel.f0 * pixel.dfg.z;
#elif !defined(USE_MULTIPLE_SCATTERING_COMPENSATION)
    return pixel.f0 * pixel.dfg.x + pixel.dfg.y;
#else
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
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

vec3 getReflectedVector(const PixelParams pixel, const vec3 v, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3  anisotropyDirection = pixel.anisotropy >= 0.0 ? pixel.anisotropicB : pixel.anisotropicT;
    vec3  anisotropicTangent  = cross(anisotropyDirection, v);
    vec3  anisotropicNormal   = cross(anisotropicTangent, anisotropyDirection);
    float bendFactor          = abs(pixel.anisotropy) * saturate(5.0 * pixel.roughness);
    vec3  bentNormal          = normalize(mix(n, anisotropicNormal, bendFactor));

    vec3 r = reflect(-v, bentNormal);
#else
    vec3 r = reflect(-v, n);
#endif
    return r;
}

vec3 getReflectedVector(const PixelParams pixel, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 r = getReflectedVector(pixel, shading_view, n);
#else
    vec3 r = shading_reflected;
#endif
    return getSpecularDominantDirection(n, r, pixel.linearRoughness);
}

//------------------------------------------------------------------------------
// Prefiltered importance sampling
//------------------------------------------------------------------------------

#if IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
vec2 hammersley(uint index) {
    // Compute Hammersley sequence
    // TODO: these should come from uniforms
    // TODO: we should do this with logical bit operations
    const uint numSamples = uint(IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT);
    const uint numSampleBits = uint(log2(float(numSamples)));
    const float invNumSamples = 1.0 / float(numSamples);
    uint i = uint(index);
    uint t = i;
    uint bits = 0u;
    for (uint j = 0u; j < numSampleBits; j++) {
        bits = bits * 2u + (t - (2u * (t / 2u)));
        t /= 2u;
    }
    return vec2(float(i), float(bits)) * invNumSamples;
}

vec3 importanceSamplingNdfDggx(vec2 u, float linearRoughness) {
    // Importance sampling D_GGX
    float a2 = linearRoughness * linearRoughness;
    float phi = 2.0 * PI * u.x;
    float cosTheta2 = (1.0 - u.y) / (1.0 + (a2 - 1.0) * u.y);
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1.0 - cosTheta2);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 importanceSamplingVNdfDggx(vec2 u, float linearRoughness, vec3 v) {
    // See: "A Simpler and Exact Sampling Routine for the GGX Distribution of Visible Normals", Eric Heitz
    float alpha = linearRoughness;

    // stretch view
    v = normalize(vec3(alpha * v.x, alpha * v.y, v.z));

    // orthonormal basis
    vec3 up = abs(v.z) < 0.9999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 t = normalize(cross(up, v));
    vec3 b = cross(t, v);

    // sample point with polar coordinates (r, phi)
    float a = 1.0 / (1.0 + v.z);
    float r = sqrt(u.x);
    float phi = (u.y < a) ? u.y / a * PI : PI + (u.y - a) / (1.0 - a) * PI;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u.y < a) ? 1.0 : v.z);

    // compute normal
    vec3 h = p1 * t + p2 * b + sqrt(max(0.0, 1.0 - p1*p1 - p2*p2)) * v;

    // unstretch
    h = normalize(vec3(alpha * h.x, alpha * h.y, max(0.0, h.z)));
    return h;
}

float prefilteredImportanceSampling(float ipdf) {
    // See: "Real-time Shading with Filtered Importance Sampling", Jaroslav Krivanek
    // Prefiltering doesn't work with anisotropy
    const float numSamples = float(IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT);
    const float invNumSamples = 1.0 / float(numSamples);
    const float dim = float(1u << uint(IBL_MAX_MIP_LEVEL));
    const float omegaP = (4.0 * PI) / (6.0 * dim * dim);
    const float invOmegaP = 1.0 / omegaP;
    const float K = 4.0;
    float omegaS = invNumSamples * ipdf;
    float mipLevel = clamp(log2(K * omegaS * invOmegaP) * 0.5, 0.0, IBL_MAX_MIP_LEVEL);
    return mipLevel;
}

vec3 isEvaluateIBL(const PixelParams pixel, vec3 n, vec3 v, float NoV) {
    // TODO: for a true anisotropic BRDF, we need a real tangent space
    vec3 up = abs(n.z) < 0.9999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);

    mat3 tangentToWorld;
    tangentToWorld[0] = normalize(cross(up, n));
    tangentToWorld[1] = cross(n, tangentToWorld[0]);
    tangentToWorld[2] = n;

    float linearRoughness = pixel.linearRoughness;
    float a2 = linearRoughness * linearRoughness;

    const uint numSamples = uint(IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT);
    const float invNumSamples = 1.0 / float(numSamples);

    vec3 indirectSpecular = vec3(0.0);
    for (uint i = 0u; i < numSamples; i++) {
        vec2 u = hammersley(i);
        vec3 h = tangentToWorld * importanceSamplingNdfDggx(u, linearRoughness);

        // Since anisotropy doesn't work with prefiltering, we use the same "faux" anisotropy
        // we do when we use the prefiltered cubemap
        vec3 l = getReflectedVector(pixel, v, h);

        // Compute this sample's contribution to the brdf
        float NoL = dot(n, l);
        if (NoL > 0.0) {
            float NoH = dot(n, h);
            float LoH = max(dot(l, h), 0.0);

            // PDF inverse (we must use D_GGX() here, which is used to generate samples)
            float ipdf = (4.0 * LoH) / (D_GGX(linearRoughness, NoH, h) * NoH);

            float mipLevel = prefilteredImportanceSampling(ipdf);

            // we use texture() instead of textureLod() to take advantage of mipmapping
            vec3 L = decodeDataForIBL(texture(light_iblSpecular, l, mipLevel));

            float D = distribution(linearRoughness, NoH, h);
            float V = visibility(linearRoughness, NoV, NoL, LoH);
            vec3  F = fresnel(pixel.f0, LoH);
            vec3 Fr = F * (D * V * NoL * ipdf * invNumSamples);

            indirectSpecular += (Fr * L);
        }
    }

    return indirectSpecular;
}

void isEvaluateClearCoatIBL(const PixelParams pixel, float specularAO, inout vec3 Fd, inout vec3 Fr) {
#if defined(MATERIAL_HAS_CLEAR_COAT)
#if defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    // We want to use the geometric normal for the clear coat layer
    float clearCoatNoV = clampNoV(dot(shading_clearCoatNormal, shading_view));
    vec3 clearCoatNormal = shading_clearCoatNormal;
#else
    float clearCoatNoV = shading_NoV;
    vec3 clearCoatNormal = shading_normal;
#endif
    // The clear coat layer assumes an IOR of 1.5 (4% reflectance)
    float Fc = F_Schlick(0.04, 1.0, clearCoatNoV) * pixel.clearCoat;
    float attenuation = 1.0 - Fc;
    Fd *= attenuation;
    Fr *= sq(attenuation);

    PixelParams p;
    p.roughness = pixel.clearCoatRoughness;
    p.f0 = vec3(0.04);
    p.linearRoughness = p.roughness * p.roughness;
    p.anisotropy = 0.0;

    vec3 clearCoatLobe = isEvaluateIBL(p, clearCoatNormal, shading_view, clearCoatNoV);
    Fr += clearCoatLobe * (specularAO * pixel.clearCoat);
#endif
}
#endif

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

void evaluateClothIndirectDiffuseBRDF(const PixelParams pixel, inout float diffuse) {
#if defined(SHADING_MODEL_CLOTH)
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    // Simulate subsurface scattering with a wrap diffuse term
    diffuse *= Fd_Wrap(shading_NoV, 0.5);
#endif
#endif
}

void evaluateClearCoatIBL(const PixelParams pixel, float specularAO, inout vec3 Fd, inout vec3 Fr) {
#if defined(MATERIAL_HAS_CLEAR_COAT)
#if defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    // We want to use the geometric normal for the clear coat layer
    float clearCoatNoV = clampNoV(dot(shading_clearCoatNormal, shading_view));
    vec3 clearCoatR = reflect(-shading_view, shading_clearCoatNormal);
#else
    float clearCoatNoV = shading_NoV;
    vec3 clearCoatR = shading_reflected;
#endif
    // The clear coat layer assumes an IOR of 1.5 (4% reflectance)
    float Fc = F_Schlick(0.04, 1.0, clearCoatNoV) * pixel.clearCoat;
    float attenuation = 1.0 - Fc;
    Fr *= sq(attenuation);
    Fr += prefilteredRadiance(clearCoatR, pixel.clearCoatRoughness) * (specularAO * Fc);
    Fd *= attenuation;
#endif
}

void evaluateSubsurfaceIBL(const PixelParams pixel, const vec3 diffuseIrradiance,
        inout vec3 Fd, inout vec3 Fr) {
#if defined(SHADING_MODEL_SUBSURFACE)
    vec3 viewIndependent = diffuseIrradiance;
    vec3 viewDependent = prefilteredRadiance(-shading_view, pixel.roughness, 1.0 + pixel.thickness);
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
    float diffuseBRDF = ao; // Fd_Lambert() is baked in the SH below
    evaluateClothIndirectDiffuseBRDF(pixel, diffuseBRDF);

    vec3 diffuseIrradiance = diffuseIrradiance(n);
    vec3 Fd = pixel.diffuseColor * diffuseIrradiance * diffuseBRDF;

    // specular indirect
    vec3 Fr;
#if IBL_INTEGRATION == IBL_INTEGRATION_PREFILTERED_CUBEMAP
    Fr = specularDFG(pixel) * prefilteredRadiance(r, pixel.roughness);
    Fr *= specularAO * pixel.energyCompensation;
    evaluateClearCoatIBL(pixel, specularAO, Fd, Fr);
#elif IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
    Fr = isEvaluateIBL(pixel, shading_normal, shading_view, shading_NoV);
    Fr *= specularAO * pixel.energyCompensation;
    isEvaluateClearCoatIBL(pixel, specularAO, Fd, Fr);
#endif
    evaluateSubsurfaceIBL(pixel, diffuseIrradiance, Fd, Fr);

    // Note: iblLuminance is already premultiplied by the exposure
    color.rgb += (Fd + Fr) * frameUniforms.iblLuminance;
}
