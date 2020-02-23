//------------------------------------------------------------------------------
// Ambient occlusion helpers
//------------------------------------------------------------------------------

float evaluateSSAO() {
    // TODO: Don't use gl_FragCoord.xy, use the view bounds
    vec2 uv = gl_FragCoord.xy * frameUniforms.resolution.zw;
    return textureLod(light_ssao, uv, 0.0).r;
}

/**
 * Computes a specular occlusion term from the ambient occlusion term.
 */
float computeSpecularAO(float NoV, float visibility, float roughness) {
#if SPECULAR_AMBIENT_OCCLUSION == 1
    return saturate(pow(NoV + visibility, exp2(-16.0 * roughness - 1.0)) - 1.0 + visibility);
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
