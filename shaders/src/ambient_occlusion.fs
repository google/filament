//------------------------------------------------------------------------------
// Ambient occlusion helpers
//------------------------------------------------------------------------------

float evaluateSSAO() {
    // TODO: Don't use gl_FragCoord.xy, use the view bounds
    vec2 uv = gl_FragCoord.xy * frameUniforms.resolution.zw;
    return textureLod(light_ssao, uv, 0.0).r;
}

#if defined(MATERIAL_HAS_BENT_NORMAL)
float sphericalCapsIntersection(float cosCap1, float cosCap2, float cosDistance) {
    // Oat and Sander 2007, "Ambient Aperture Lighting"
    // Approximation mentioned by Jimenez et al. 2016
    float r1 = acosFast(cosCap1);
    float r2 = acosFast(cosCap2);
    float d  = acosFast(cosDistance);

    // We work with cosine angles, replace the original paper's use of
    // min(r1, r2) with max(cosCap1, cosCap2)

    if (min(r1, r2) <= max(r1, r2) - d) {
        return 2.0 * PI - 2.0 * PI * max(cosCap1, cosCap2);
    } else if (r1 + r2 <= d) {
        return 0.0;
    }

    float delta = abs(r1 - r2);
    float x = 1.0 - saturate((d - delta) / max(r1 + r2 - delta, 0.0001));
    float x2 = sq(x);
    float area = 2.0 * PI - 2.0 * PI * max(cosCap1, cosCap2);
    // simplified smoothsteph()
    area *= -2.0 * x2 * x + 3.0 * x2;

    return area;
}

float computeBentSpecularAO(float visibility, float roughness) {
    // Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"

    // aperture from ambient occlusion
    float cosAv = sqrt(1.0 - visibility);
    // aperture from roughness, log(10) / log(2) = 3.321928
    float cosAs = exp2(-3.321928 * sq(roughness));
    // angle betwen bent normal and reflection direction
    float cosB  = dot(shading_bentNormal, shading_reflected);

    return sphericalCapsIntersection(cosAv, cosAs, 0.5 * cosB + 0.5) / (2.0 * PI * (1.0 - cosAs));
}
#endif

/**
 * Computes a specular occlusion term from the ambient occlusion term.
 */
float computeSpecularAO(float NoV, float visibility, float roughness) {
#if SPECULAR_AMBIENT_OCCLUSION == 1
#if defined(MATERIAL_HAS_BENT_NORMAL)
    return computeBentSpecularAO(visibility, roughness);
#else
    return saturate(pow(NoV + visibility, exp2(-16.0 * roughness - 1.0)) - 1.0 + visibility);
#endif
#else
    return 1.0;
#endif
}

#if MULTI_BOUNCE_AMBIENT_OCCLUSION == 1
/**
 * Returns a color ambient occlusion based on a pre-computed visibility term.
 * The albedo term is meant to be the diffuse color or f0 for the diffuse and
 * specular terms respectively.
 */
vec3 gtaoMultiBounce(float visibility, const vec3 albedo) {
    // Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;

    return max(vec3(visibility), ((visibility * a + b) * visibility + c) * visibility);
}
#endif

void multiBounceAO(float visibility, const vec3 albedo, inout vec3 color) {
#if MULTI_BOUNCE_AMBIENT_OCCLUSION == 1
    color *= gtaoMultiBounce(visibility, albedo);
#endif
}

void multiBounceSpecularAO(float visibility, const vec3 albedo, inout vec3 color) {
#if MULTI_BOUNCE_AMBIENT_OCCLUSION == 1 && SPECULAR_AMBIENT_OCCLUSION == 1
    color *= gtaoMultiBounce(visibility, albedo);
#endif
}

float singleBounceAO(float visibility) {
#if MULTI_BOUNCE_AMBIENT_OCCLUSION == 1
    return 1.0;
#else
    return visibility;
#endif
}
