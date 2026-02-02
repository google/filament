#if MATERIAL_FEATURE_LEVEL > 0

//------------------------------------------------------------------------------
// Image based lighting configuration
//------------------------------------------------------------------------------

// IBL integration algorithm
#define IBL_INTEGRATION_PREFILTERED_CUBEMAP         0
#define IBL_INTEGRATION_IMPORTANCE_SAMPLING         1

#define IBL_INTEGRATION                             IBL_INTEGRATION_PREFILTERED_CUBEMAP

#define IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT   64

//------------------------------------------------------------------------------
// IBL utilities
//------------------------------------------------------------------------------

vec3 decodeDataForIBL(const vec4 data) {
    return data.rgb;
}

//------------------------------------------------------------------------------
// IBL prefiltered DFG term implementations
//------------------------------------------------------------------------------

vec3 PrefilteredDFG_LUT(float lod, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    return textureLod(sampler0_iblDFG, vec2(NoV, lod), 0.0).rgb;
}

//------------------------------------------------------------------------------
// IBL environment BRDF dispatch
//------------------------------------------------------------------------------

vec3 prefilteredDFG(float perceptualRoughness, float NoV) {
    // PrefilteredDFG_LUT() takes a LOD, which is sqrt(roughness) = perceptualRoughness
    return PrefilteredDFG_LUT(perceptualRoughness, NoV);
}

//------------------------------------------------------------------------------
// IBL irradiance implementations
//------------------------------------------------------------------------------

vec3 Irradiance_SphericalHarmonics(const vec3 n) {
    vec3 sphericalHarmonics = frameUniforms.iblSH[0];

    if (CONFIG_SH_BANDS_COUNT >= 2) {
        sphericalHarmonics +=
                  frameUniforms.iblSH[1] * (n.y)
                + frameUniforms.iblSH[2] * (n.z)
                + frameUniforms.iblSH[3] * (n.x);
    }

    if (CONFIG_SH_BANDS_COUNT >= 3) {
        sphericalHarmonics +=
                  frameUniforms.iblSH[4] * (n.y * n.x)
                + frameUniforms.iblSH[5] * (n.y * n.z)
                + frameUniforms.iblSH[6] * (3.0 * n.z * n.z - 1.0)
                + frameUniforms.iblSH[7] * (n.z * n.x)
                + frameUniforms.iblSH[8] * (n.x * n.x - n.y * n.y);
    }

    return max(sphericalHarmonics, 0.0);
}

vec3 Irradiance_RoughnessOne(const vec3 n) {
    // note: lod used is always integer, hopefully the hardware skips tri-linear filtering
    return decodeDataForIBL(textureLod(sampler0_iblSpecular, n, frameUniforms.iblRoughnessOneLevel));
}

//------------------------------------------------------------------------------
// IBL irradiance dispatch
//------------------------------------------------------------------------------

vec3 diffuseIrradiance(const vec3 n) {
    // On Metal devices with certain chipsets, this light_iblSpecular texture sample must be pulled
    // outside the frameUniforms.iblSH check. This is to avoid a Metal pipeline compilation error
    // with the message: "Could not statically determine the target of a texture".
    // The reason for this is unknown, and is possibly a bug that exhibits only on these devices.
    vec3 irradianceRoughnessOne;
    if (CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND) {
        irradianceRoughnessOne = Irradiance_RoughnessOne(n);
    }

    if (frameUniforms.iblSH[0].x == 65504.0) {
#if FILAMENT_QUALITY < FILAMENT_QUALITY_HIGH
        if (CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND) {
            return irradianceRoughnessOne;
        } else {
            return Irradiance_RoughnessOne(n);
        }
#else
        ivec2 s = textureSize(sampler0_iblSpecular, int(frameUniforms.iblRoughnessOneLevel));
        float du = 1.0 / float(s.x);
        float dv = 1.0 / float(s.y);
        vec3 m0 = normalize(cross(n, vec3(0.0, 1.0, 0.0)));
        vec3 m1 = cross(m0, n);
        vec3 m0du = m0 * du;
        vec3 m1dv = m1 * dv;
        vec3 c;
        c  = Irradiance_RoughnessOne(n - m0du - m1dv);
        c += Irradiance_RoughnessOne(n + m0du - m1dv);
        c += Irradiance_RoughnessOne(n + m0du + m1dv);
        c += Irradiance_RoughnessOne(n - m0du + m1dv);
        return c * 0.25;
#endif
    } else {
        return Irradiance_SphericalHarmonics(n);
    }
}

//------------------------------------------------------------------------------
// IBL specular
//------------------------------------------------------------------------------

float perceptualRoughnessToLod(float perceptualRoughness) {
    // The mapping below is a quadratic fit for log2(perceptualRoughness)+iblRoughnessOneLevel when
    // iblRoughnessOneLevel is 4. We found empirically that this mapping works very well for
    // a 256 cubemap with 5 levels used. But also scales well for other iblRoughnessOneLevel values.
    return frameUniforms.iblRoughnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
}

vec3 prefilteredRadiance(const vec3 r, float lod) {
    return decodeDataForIBL(textureLod(sampler0_iblSpecular, r, lod));
}

vec3 prefilteredRadiance(const vec3 r, float roughness, float offset) {
    float lod = frameUniforms.iblRoughnessOneLevel * roughness;
    return decodeDataForIBL(textureLod(sampler0_iblSpecular, r, lod + offset));
}

vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

vec3 specularDFG(const PixelParams pixel) {
#if defined(SHADING_MODEL_CLOTH)
    return pixel.f0 * pixel.dfg.z;
#else
#if defined(MATERIAL_HAS_SPECULAR_COLOR_FACTOR) || defined(MATERIAL_HAS_SPECULAR_FACTOR)
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0) * pixel.specular;
#else
    return mix(pixel.dfg.xxx, pixel.dfg.yyy, pixel.f0);
#endif
#endif
}

vec3 getReflectedVector(const PixelParams pixel, const vec3 n) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 r = getReflectedVector(pixel, shading_view, n);
#else
    vec3 r = shading_reflected;
#endif
    return getSpecularDominantDirection(n, r, pixel.roughness);
}

//------------------------------------------------------------------------------
// Prefiltered importance sampling
//------------------------------------------------------------------------------

#if IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
vec2 hammersley(uint index) {
    const uint numSamples = uint(IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT);
    const float invNumSamples = 1.0 / float(numSamples);
    const float tof = 0.5 / float(0x80000000U);
    uint bits = index;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return vec2(float(index) * invNumSamples, float(bits) * tof);
}

vec3 importanceSamplingNdfDggx(vec2 u, float roughness) {
    // Importance sampling D_GGX
    float a2 = roughness * roughness;
    float phi = 2.0 * PI * u.x;
    float cosTheta2 = (1.0 - u.y) / (1.0 + (a2 - 1.0) * u.y);
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1.0 - cosTheta2);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 hemisphereCosSample(vec2 u) {
    float phi = 2.0 * PI * u.x;
    float cosTheta2 = 1.0 - u.y;
    float cosTheta = sqrt(cosTheta2);
    float sinTheta = sqrt(1.0 - cosTheta2);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 importanceSamplingVNdfDggx(vec2 u, float roughness, vec3 v) {
    // See: "A Simpler and Exact Sampling Routine for the GGX Distribution of Visible Normals", Eric Heitz
    float alpha = roughness;

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

float prefilteredImportanceSampling(float ipdf, float omegaP) {
    // See: "Real-time Shading with Filtered Importance Sampling", Jaroslav Krivanek
    // Prefiltering doesn't work with anisotropy
    const float numSamples = float(IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT);
    const float invNumSamples = 1.0 / float(numSamples);
    const float K = 4.0;
    float omegaS = invNumSamples * ipdf;
    float mipLevel = log2(K * omegaS / omegaP) * 0.5;    // log4
    return mipLevel;
}

vec3 isEvaluateSpecularIBL(const PixelParams pixel, const vec3 n, const vec3 v, const float NoV) {
    const int numSamples = IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT;
    const float invNumSamples = 1.0 / float(numSamples);
    const vec3 up = vec3(0.0, 0.0, 1.0);

    // TODO: for a true anisotropic BRDF, we need a real tangent space
    // tangent space
    mat3 T;
    T[0] = normalize(cross(up, n));
    T[1] = cross(n, T[0]);
    T[2] = n;

    // Random rotation around N per pixel
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    float a = 2.0 * PI * fract(m.z * fract(dot(gl_FragCoord.xy, m.xy)));
    float c = cos(a);
    float s = sin(a);
    mat3 R;
    R[0] = vec3( c, s, 0);
    R[1] = vec3(-s, c, 0);
    R[2] = vec3( 0, 0, 1);
    T *= R;

    float roughness = pixel.roughness;
    float dim = float(textureSize(sampler0_iblSpecular, 0).x);
    float omegaP = (4.0 * PI) / (6.0 * dim * dim);

    vec3 indirectSpecular = vec3(0.0);
    for (int i = 0; i < numSamples; i++) {
        vec2 u = hammersley(i);
        vec3 h = T * importanceSamplingNdfDggx(u, roughness);

        // Since anisotropy doesn't work with prefiltering, we use the same "faux" anisotropy
        // we do when we use the prefiltered cubemap
        vec3 l = getReflectedVector(pixel, v, h);

        // Compute this sample's contribution to the brdf
        float NoL = saturate(dot(n, l));
        if (NoL > 0.0) {
            float NoH = dot(n, h);
            float LoH = saturate(dot(l, h));

            // PDF inverse (we must use D_GGX() here, which is used to generate samples)
            float ipdf = (4.0 * LoH) / (D_GGX(roughness, NoH, h) * NoH);
            float mipLevel = prefilteredImportanceSampling(ipdf, omegaP);
            vec3 L = decodeDataForIBL(textureLod(sampler0_iblSpecular, l, mipLevel));

            float D = distribution(roughness, NoH, h);
            float V = visibility(roughness, NoV, NoL);
            vec3 F = fresnel(pixel.f0, LoH);
            vec3 Fr = F * (D * V * NoL * ipdf * invNumSamples);

            indirectSpecular += (Fr * L);
        }
    }

    return indirectSpecular;
}

vec3 isEvaluateDiffuseIBL(const PixelParams pixel, vec3 n, vec3 v) {
    const int numSamples = IBL_INTEGRATION_IMPORTANCE_SAMPLING_COUNT;
    const float invNumSamples = 1.0 / float(numSamples);
    const vec3 up = vec3(0.0, 0.0, 1.0);

    // TODO: for a true anisotropic BRDF, we need a real tangent space
    // tangent space
    mat3 T;
    T[0] = normalize(cross(up, n));
    T[1] = cross(n, T[0]);
    T[2] = n;

    // Random rotation around N per pixel
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    float a = 2.0 * PI * fract(m.z * fract(dot(gl_FragCoord.xy, m.xy)));
    float c = cos(a);
    float s = sin(a);
    mat3 R;
    R[0] = vec3( c, s, 0);
    R[1] = vec3(-s, c, 0);
    R[2] = vec3( 0, 0, 1);
    T *= R;

    float dim = float(textureSize(sampler0_iblSpecular, 0).x);
    float omegaP = (4.0 * PI) / (6.0 * dim * dim);

    vec3 indirectDiffuse = vec3(0.0);
    for (int i = 0; i < numSamples; i++) {
        vec2 u = hammersley(i);
        vec3 h = T * hemisphereCosSample(u);

        // Since anisotropy doesn't work with prefiltering, we use the same "faux" anisotropy
        // we do when we use the prefiltered cubemap
        vec3 l = getReflectedVector(pixel, v, h);

        // Compute this sample's contribution to the brdf
        float NoL = saturate(dot(n, l));
        if (NoL > 0.0) {
            // PDF inverse (we must use D_GGX() here, which is used to generate samples)
            float ipdf = PI / NoL;
            // we have to bias the mipLevel (+1) to help with very strong highlights
            float mipLevel = prefilteredImportanceSampling(ipdf, omegaP) + 1.0;
            vec3 L = decodeDataForIBL(textureLod(sampler0_iblSpecular, l, mipLevel));
            indirectDiffuse += L;
        }
    }

    return indirectDiffuse * invNumSamples; // we bake 1/PI here, which cancels out
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
    Fr *= attenuation;

    PixelParams p;
    p.perceptualRoughness = pixel.clearCoatPerceptualRoughness;
    p.f0 = vec3(0.04);
    p.roughness = perceptualRoughnessToRoughness(p.perceptualRoughness);
#if defined(MATERIAL_HAS_ANISOTROPY)
    p.anisotropy = 0.0;
#endif

    vec3 clearCoatLobe = isEvaluateSpecularIBL(p, clearCoatNormal, shading_view, clearCoatNoV);
    Fr += clearCoatLobe * (specularAO * pixel.clearCoat);
#endif
}
#endif

//------------------------------------------------------------------------------
// IBL evaluation
//------------------------------------------------------------------------------

void evaluateClothIndirectDiffuseBRDF(const PixelParams pixel, inout float diffuse) {
#if defined(SHADING_MODEL_CLOTH)
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    // Simulate subsurface scattering with a wrap diffuse term
    diffuse *= Fd_Wrap(shading_NoV, 0.5);
#endif
#endif
}

void evaluateSheenIBL(const PixelParams pixel, float diffuseAO,
        const in SSAOInterpolationCache cache, inout vec3 Fd, inout vec3 Fr) {
#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE)
#if defined(MATERIAL_HAS_SHEEN_COLOR)
    // Albedo scaling of the base layer before we layer sheen on top
    Fd *= pixel.sheenScaling;
    Fr *= pixel.sheenScaling;

    vec3 reflectance = pixel.sheenDFG * pixel.sheenColor;
    reflectance *= specularAO(shading_NoV, diffuseAO, pixel.sheenRoughness, cache);

    Fr += reflectance * prefilteredRadiance(shading_reflected,
            perceptualRoughnessToLod(pixel.sheenPerceptualRoughness));
#endif
#endif
}

void evaluateClearCoatIBL(const PixelParams pixel, float diffuseAO,
        const in SSAOInterpolationCache cache, inout vec3 Fd, inout vec3 Fr) {
#if IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
    float specularAO = specularAO(shading_NoV, diffuseAO, pixel.clearCoatRoughness, cache);
    isEvaluateClearCoatIBL(pixel, specularAO, Fd, Fr);
    return;
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    if (pixel.clearCoat > 0.0) {
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
        Fd *= attenuation;
        Fr *= attenuation;

        // TODO: Should we apply specularAO to the attenuation as well?
        float specularAO = specularAO(clearCoatNoV, diffuseAO, pixel.clearCoatRoughness, cache);
        Fr += prefilteredRadiance(clearCoatR,
                perceptualRoughnessToLod(pixel.clearCoatPerceptualRoughness)) * (specularAO * Fc);
    }
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

#if defined(MATERIAL_HAS_REFRACTION)

struct Refraction {
    vec3 position;
    vec3 direction;
    float d;
};

/**
 * Internal helper for solid sphere refraction.
 * Optimized to take pre-calculated geometric constants to support efficient spectral dispersion.
 */
void refractedRaySolidSphere(const vec3 r_in, float NoR_in, float sin2Theta_in,
        float etaIR, float etaRI, float thickness, const vec3 n, out Refraction ray) {

    // Snell's Law for the first interface (entry into medium)
    // We use the pre-calculated sin^2(theta) to solve for the internal ray direction
    float k = 1.0 - etaIR * etaIR * sin2Theta_in;
    vec3 rr = etaIR * r_in - (etaIR * NoR_in + sqrt(max(k, 0.0))) * n;

    float NoR = dot(n, rr);
    float d = thickness * -NoR;

    ray.d = d;
    ray.position = shading_position + rr * d;

    // Second interface exit (Sphere fudge)
    // Simulates the curvature of a sphere without full ray-intersection math
    vec3 n1 = normalize(NoR * rr - n * 0.5);
    ray.direction = refract(rr, n1, etaRI);
}

/**
 * Standard Solid Sphere Refraction (N=1)
 */
void refractionSolidSphere(float etaIR, float etaRI, float thickness,
        const vec3 n, vec3 r, out Refraction ray) {
    float NoR_in = dot(n, r);
    float sin2Theta_in = 1.0 - NoR_in * NoR_in;
    refractedRaySolidSphere(r, NoR_in, sin2Theta_in, etaIR, etaRI, thickness, n, ray);
}

void refractionSolidBox(float etaIR, float thickness,
        const vec3 n, vec3 r, out Refraction ray) {
    vec3 rr = refract(r, n, etaIR);
    float NoR = dot(n, rr);
    float d = thickness / max(-NoR, 0.001);
    ray.position = vec3(shading_position + rr * d);
    ray.direction = r;
    ray.d = d;
#if REFRACTION_MODE == REFRACTION_MODE_CUBEMAP
    // fudge direction vector, so we see the offset due to the thickness of the object
    float envDistance = 10.0; // this should come from a ubo
    ray.direction = normalize((ray.position - shading_position) + ray.direction * envDistance);
#endif
}

void refractionThinSphere(float etaIR, float uThickness,
        const vec3 n, vec3 r, out Refraction ray) {
    float d = 0.0;
#if defined(MATERIAL_HAS_MICRO_THICKNESS)
    // note: we need the refracted ray to calculate the distance traveled
    // we could use shading_NoV, but we would lose the dependency on ior.
    vec3 rr = refract(r, n, etaIR);
    float NoR = dot(n, rr);
    d = uThickness / max(-NoR, 0.001);
    ray.position = vec3(shading_position + rr * d);
#else
    ray.position = vec3(shading_position);
#endif
    ray.direction = r;
    ray.d = d;
}

vec3 evaluateRefraction(const PixelParams pixel, const vec3 n0, const float lod,
    const float etaIR, const float etaRI) {

    Refraction ray;

#if REFRACTION_TYPE == REFRACTION_TYPE_SOLID
    refractionSolidSphere(etaIR, etaRI, pixel.thickness, n0, -shading_view, ray);
#elif REFRACTION_TYPE == REFRACTION_TYPE_THIN
    refractionThinSphere(etaIR, pixel.uThickness, n0, -shading_view, ray);
#else
#   error invalid REFRACTION_TYPE
#endif

    /* sample the cubemap or screen-space */
#if REFRACTION_MODE == REFRACTION_MODE_CUBEMAP
    // when reading from the cubemap, we are not pre-exposed so we apply iblLuminance
    // which is not the case when we'll read from the screen-space buffer
    vec3 t = prefilteredRadiance(ray.direction, lod) * frameUniforms.iblLuminance;
#else
    // compute the point where the ray exits the medium, if needed
    highp vec4 p = mulMat4x4Float3(getClipFromWorldMatrix(), ray.position);
    vec2 uv = uvToRenderTargetUV(p.xy * (0.5 / p.w) + 0.5);
    vec3 t = textureLod(sampler0_ssr, vec3(uv, 0.0), lod).rgb;
#endif

    // apply absorption
#if defined(MATERIAL_HAS_ABSORPTION)
    // compute transmission T
    vec3 T = saturate(exp(-pixel.absorption * ray.d));
    t *= T;
#endif

    return t;
}

#if defined(MATERIAL_HAS_DISPERSION) && (REFRACTION_TYPE == REFRACTION_TYPE_SOLID)

/**
 * HIGH-FIDELITY SPECTRAL DISPERSION (N=4)
 * ---------------------------------------
 * This algorithm approximates the spectral integral of refracted light by sampling
 * four optimized wavelengths and collapsing the color-matching and color-space
 * transforms into four pre-computed 3x3 matrices (K0-K3).
 * See tools/specgen.
 */

vec3 calculateDispersion(const PixelParams pixel, const vec3 n0, const float lod) {
    const mat3 K0 = mat3(
         0.00581637, 0.02312851, 0.01689631,
        -0.11782236, 0.11316202, 0.11098148,
        -0.45422013, 0.04493517, 0.98249798
    ); // 486.1nm

    const mat3 K1 = mat3(
         0.14291703, 0.10429778, -0.01556522,
        -0.27560148, 0.57678541, -0.06412244,
         0.06839811, 0.02732891,  0.01602064
    ); // 546.1nm

    const mat3 K2 = mat3(
        0.70106120, -0.09440402, -0.00241699,
        0.29545674,  0.29931852, -0.04351961,
        0.31884400, -0.05627069,  0.00083808
    ); // 589.3nm

    const mat3 K3 = mat3(
        0.15020522, -0.03302213,  0.00108589,
        0.09796715,  0.01073410, -0.00333946,
        0.06697807, -0.01599341,  0.00064333
    ); // 656.3nm

    const float offsets[4] = float[](0.70795215, 0.24790980, 0.00000000, -0.29204785);

    float n_base = pixel.etaRI;
    float P = pixel.dispersion / 20.0;
    float dispFactor = P * (n_base - 1.0);

    // JITTER / DITHERING
    // Calculate per-pixel jitter to fill the gaps between the 4 samples
    // 0.35 is roughly the distance between the Gaussian sample points.
    float jitter = (interleavedGradientNoise(gl_FragCoord.xy + vec2(frameUniforms.temporalNoise)) - 0.5)
            * 0.35 * dispFactor;

    // Gaussian mapping for t in [0, 1] relative to the 420-680nm range
    float nd0 = n_base + dispFactor * offsets[0] + jitter;
    float nd1 = n_base + dispFactor * offsets[1] + jitter;
    float nd2 = n_base + dispFactor * offsets[2] + jitter;
    float nd3 = n_base + dispFactor * offsets[3] + jitter;

    // Compute 4 Ray Directions
    // We unroll the refraction model logic to compute all 4 rays at once
    Refraction r0, r1, r2, r3;

    vec3 r_in = -shading_view;
    float NoR_in = dot(n0, r_in);
    float sin2Theta_in = 1.0 - NoR_in * NoR_in;
    refractedRaySolidSphere(r_in, NoR_in, sin2Theta_in, 1.0 / nd0, nd0, pixel.thickness, n0, r0);
    refractedRaySolidSphere(r_in, NoR_in, sin2Theta_in, 1.0 / nd1, nd1, pixel.thickness, n0, r1);
    refractedRaySolidSphere(r_in, NoR_in, sin2Theta_in, 1.0 / nd2, nd2, pixel.thickness, n0, r2);
    refractedRaySolidSphere(r_in, NoR_in, sin2Theta_in, 1.0 / nd3, nd3, pixel.thickness, n0, r3);

    // Batch Texture Fetches
    // Issuing all textureLod calls back-to-back is the key to performance here.
    vec3 s0, s1, s2, s3;
#if REFRACTION_MODE == REFRACTION_MODE_CUBEMAP
    s0 = prefilteredRadiance(r0.direction, lod);
    s1 = prefilteredRadiance(r1.direction, lod);
    s2 = prefilteredRadiance(r2.direction, lod);
    s3 = prefilteredRadiance(r3.direction, lod);
    float ibl = frameUniforms.iblLuminance;
    s0 *= ibl;
    s1 *= ibl;
    s2 *= ibl;
    s3 *= ibl;
#else
    // SSR Path: Calculate 4 UVs, then 4 Samples
    highp mat4 clipFromWorld = getClipFromWorldMatrix();
    vec4 p0 = mulMat4x4Float3(clipFromWorld, r0.position);
    vec4 p1 = mulMat4x4Float3(clipFromWorld, r1.position);
    vec4 p2 = mulMat4x4Float3(clipFromWorld, r2.position);
    vec4 p3 = mulMat4x4Float3(clipFromWorld, r3.position);

    vec2 uv0 = uvToRenderTargetUV(p0.xy * (0.5 / p0.w) + 0.5);
    vec2 uv1 = uvToRenderTargetUV(p1.xy * (0.5 / p1.w) + 0.5);
    vec2 uv2 = uvToRenderTargetUV(p2.xy * (0.5 / p2.w) + 0.5);
    vec2 uv3 = uvToRenderTargetUV(p3.xy * (0.5 / p3.w) + 0.5);

    s0 = textureLod(sampler0_ssr, vec3(uv0, 0.0), lod).rgb;
    s1 = textureLod(sampler0_ssr, vec3(uv1, 0.0), lod).rgb;
    s2 = textureLod(sampler0_ssr, vec3(uv2, 0.0), lod).rgb;
    s3 = textureLod(sampler0_ssr, vec3(uv3, 0.0), lod).rgb;
#endif

    // Apply Absorption (Optimized: Use r1 as the representative path length)
#if defined(MATERIAL_HAS_ABSORPTION)
    vec3 T = saturate(exp(-pixel.absorption * r1.d));
    s0 *= T;
    s1 *= T;
    s2 *= T;
    s3 *= T;
#endif

    // 6. Spectral Integration
    return max(K0 * s0 + K1 * s1 + K2 * s2 + K3 * s3, 0.0);
}

#endif

vec3 evaluateRefraction(const PixelParams pixel, const vec3 n0, vec3 E) {
    vec3 Ft;

    // Note: We use the average IOR for the roughness lod calculation.

    // Roughness remapping so that an IOR of 1.0 means no microfacet refraction and an IOR
    // of 1.5 has full microfacet refraction
    float perceptualRoughness = mix(pixel.perceptualRoughnessUnclamped, 0.0, saturate(pixel.etaIR * 3.0 - 2.0));

#if REFRACTION_MODE == REFRACTION_MODE_CUBEMAP
    float lod = perceptualRoughnessToLod(perceptualRoughness);
#else
    // distance to camera plane
    const float invLog2sqrt5 = 0.8614;
    float lod = max(0.0, (2.0 * log2(perceptualRoughness) + frameUniforms.refractionLodOffset) * invLog2sqrt5);
#endif

#if defined(MATERIAL_HAS_DISPERSION) && (REFRACTION_TYPE == REFRACTION_TYPE_SOLID)
    Ft = calculateDispersion(pixel, n0, lod);
#else
    Ft = evaluateRefraction(pixel, n0, lod, pixel.etaIR, pixel.etaRI);
#endif


#if REFRACTION_TYPE == REFRACTION_TYPE_THIN
    // For thin surfaces, the light will bounce off at the second interface in the direction of
    // the reflection, effectively adding to the specular, but this process will repeat itself.
    // Each time the ray exits the surface on the front side after the first bounce,
    // it's multiplied by E^2, and we get: E + E(1-E)^2 + E^3(1-E)^2 + ...
    // This infinite series converges and is easy to simplify.
    // Note: we calculate these bounces only on a single component,
    // since it's a fairly subtle effect.
    E *= 1.0 + pixel.transmission * (1.0 - E.g) / (1.0 + E.g);
#endif

    // fresnel from the first interface
    Ft *= 1.0 - E;

    // base color changes the amount of light passing through the boundary
    Ft *= pixel.diffuseColor;

    return Ft;
}
#endif


void evaluateIBL(const MaterialInputs material, const PixelParams pixel, inout vec3 color) {
    // specular layer
    vec3 Fr = vec3(0.0);

    SSAOInterpolationCache interpolationCache;
#if defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED) || defined(MATERIAL_HAS_REFLECTIONS)
    interpolationCache.uv = uvToRenderTargetUV(getNormalizedPhysicalViewportCoord().xy);
#endif

    // screen-space reflections
#if defined(MATERIAL_HAS_REFLECTIONS)
    vec4 ssrFr = vec4(0.0);
#if defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED)
    // do the uniform based test first
    if (frameUniforms.ssrDistance > 0.0) {
        // There is no point doing SSR for very high roughness because we're limited by the fov
        // of the screen, in addition it doesn't really add much to the final image.
        // TODO: maybe make this a parameter
        const float maxPerceptualRoughness = sqrt(0.5);
        if (pixel.perceptualRoughness < maxPerceptualRoughness) {
            // distance to camera plane
            const float invLog2sqrt5 = 0.8614;
            float d = -mulMat4x4Float3(getViewFromWorldMatrix(), getWorldPosition()).z;
            float lod = max(0.0, (log2(pixel.roughness / d) + frameUniforms.refractionLodOffset) * invLog2sqrt5);
            ssrFr = textureLod(sampler0_ssr, vec3(interpolationCache.uv, 1.0), lod);
        }
    }
#else // BLEND_MODE_OPAQUE
    // TODO: for blended transparency, we have to ray-march here (limited to mirror reflections)
#endif
#else // MATERIAL_HAS_REFLECTIONS
    const vec4 ssrFr = vec4(0.0);
#endif

    // If screen-space reflections are turned on and have full contribution (ssr.a == 1.0), then we
    // skip sampling the IBL down below.

#if IBL_INTEGRATION == IBL_INTEGRATION_PREFILTERED_CUBEMAP
    vec3 E = specularDFG(pixel);
    if (ssrFr.a < 1.0) { // prevent reading the IBL if possible
        vec3 r = getReflectedVector(pixel, shading_normal);
        Fr = E * prefilteredRadiance(r, perceptualRoughnessToLod(pixel.perceptualRoughness));
    }
#elif IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
    vec3 E = vec3(0.0); // TODO: fix for importance sampling
    if (ssrFr.a < 1.0) { // prevent evaluating the IBL if possible
        Fr = isEvaluateSpecularIBL(pixel, shading_normal, shading_view, shading_NoV);
    }
#endif

    // Ambient occlusion
    float ssao = evaluateSSAO(interpolationCache);
    float diffuseAO = min(material.ambientOcclusion, ssao);
    float specularAO = specularAO(shading_NoV, diffuseAO, pixel.roughness, interpolationCache);

    vec3 specularSingleBounceAO = singleBounceAO(specularAO) * pixel.energyCompensation;
    Fr *= specularSingleBounceAO;
#if defined(MATERIAL_HAS_REFLECTIONS)
    ssrFr.rgb *= specularSingleBounceAO;
#endif

    // diffuse layer
    float diffuseBRDF = singleBounceAO(diffuseAO); // Fd_Lambert() is baked in the SH below
    evaluateClothIndirectDiffuseBRDF(pixel, diffuseBRDF);

#if defined(MATERIAL_HAS_BENT_NORMAL)
    vec3 diffuseNormal = shading_bentNormal;
#else
    vec3 diffuseNormal = shading_normal;
#endif

#if IBL_INTEGRATION == IBL_INTEGRATION_PREFILTERED_CUBEMAP
    vec3 diffuseIrradiance = diffuseIrradiance(diffuseNormal);
#elif IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING
    vec3 diffuseIrradiance = isEvaluateDiffuseIBL(pixel, diffuseNormal, shading_view);
#endif
    vec3 Fd = pixel.diffuseColor * diffuseIrradiance * (1.0 - E) * diffuseBRDF;

    // subsurface layer
    evaluateSubsurfaceIBL(pixel, diffuseIrradiance, Fd, Fr);

    // extra ambient occlusion term for the base and subsurface layers
    multiBounceAO(diffuseAO, pixel.diffuseColor, Fd);
    multiBounceSpecularAO(specularAO, pixel.f0, Fr);

    // sheen layer
    evaluateSheenIBL(pixel, diffuseAO, interpolationCache, Fd, Fr);

    // clear coat layer
    evaluateClearCoatIBL(pixel, diffuseAO, interpolationCache, Fd, Fr);

    Fr *= frameUniforms.iblLuminance;
    Fd *= frameUniforms.iblLuminance;

#if defined(MATERIAL_HAS_REFRACTION)
    vec3 Ft = evaluateRefraction(pixel, shading_normal, E);
    Ft *= pixel.transmission;
    Fd *= (1.0 - pixel.transmission);
#endif

#if defined(MATERIAL_HAS_REFLECTIONS)
    Fr = Fr * (1.0 - ssrFr.a) + (E * ssrFr.rgb);
#endif

    // Combine all terms
    // Note: iblLuminance is already premultiplied by the exposure
    color.rgb += Fr + Fd;
#if defined(MATERIAL_HAS_REFRACTION)
    color.rgb += Ft;
#endif
}

#endif  // MATERIAL_FEATURE_LEVEL > 0
