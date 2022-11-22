//------------------------------------------------------------------------------
// Ambient occlusion configuration
//------------------------------------------------------------------------------

// Diffuse BRDFs
#define SPECULAR_AO_OFF             0
#define SPECULAR_AO_SIMPLE          1
#define SPECULAR_AO_BENT_NORMALS    2

//------------------------------------------------------------------------------
// Ambient occlusion helpers
//------------------------------------------------------------------------------

float unpack(vec2 depth) {
    // this is equivalent to (x8 * 256 + y8) / 65535, which gives a value between 0 and 1
    return (depth.x * (256.0 / 257.0) + depth.y * (1.0 / 257.0));
}

struct SSAOInterpolationCache {
    highp vec4 weights;
#if defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED) || defined(MATERIAL_HAS_REFLECTIONS)
    highp vec2 uv;
#endif
};

float evaluateSSAO(inout SSAOInterpolationCache cache) {
#if defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED)

    if (frameUniforms.aoSamplingQualityAndEdgeDistance < 0.0) {
        // SSAO is disabled
        return 1.0;
    }

    // Upscale the SSAO buffer in real-time, in high quality mode we use a custom bilinear
    // filter. This adds about 2.0ms @ 250MHz on Pixel 4.

    if (frameUniforms.aoSamplingQualityAndEdgeDistance > 0.0) {
        highp vec2 size = vec2(textureSize(light_ssao, 0));

        // Read four AO samples and their depths values
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
        vec4 ao = textureGather(light_ssao, vec3(cache.uv, 0.0), 0);
        vec4 dg = textureGather(light_ssao, vec3(cache.uv, 0.0), 1);
        vec4 db = textureGather(light_ssao, vec3(cache.uv, 0.0), 2);
#else
        vec3 s01 = textureLodOffset(light_ssao, vec3(cache.uv, 0.0), 0.0, ivec2(0, 1)).rgb;
        vec3 s11 = textureLodOffset(light_ssao, vec3(cache.uv, 0.0), 0.0, ivec2(1, 1)).rgb;
        vec3 s10 = textureLodOffset(light_ssao, vec3(cache.uv, 0.0), 0.0, ivec2(1, 0)).rgb;
        vec3 s00 = textureLodOffset(light_ssao, vec3(cache.uv, 0.0), 0.0, ivec2(0, 0)).rgb;
        vec4 ao = vec4(s01.r, s11.r, s10.r, s00.r);
        vec4 dg = vec4(s01.g, s11.g, s10.g, s00.g);
        vec4 db = vec4(s01.b, s11.b, s10.b, s00.b);
#endif
        // bilateral weights
        vec4 depths;
        depths.x = unpack(vec2(dg.x, db.x));
        depths.y = unpack(vec2(dg.y, db.y));
        depths.z = unpack(vec2(dg.z, db.z));
        depths.w = unpack(vec2(dg.w, db.w));
        depths *= -frameUniforms.cameraFar;

        // bilinear weights
        vec2 f = fract(cache.uv * size - 0.5);
        vec4 b;
        b.x = (1.0 - f.x) * f.y;
        b.y = f.x * f.y;
        b.z = f.x * (1.0 - f.y);
        b.w = (1.0 - f.x) * (1.0 - f.y);

        highp mat4 m = getViewFromWorldMatrix();
        highp float d = dot(vec3(m[0].z, m[1].z, m[2].z), shading_position) + m[3].z;
        highp vec4 w = (vec4(d) - depths) * frameUniforms.aoSamplingQualityAndEdgeDistance;
        w = max(vec4(MEDIUMP_FLT_MIN), 1.0 - w * w) * b;
        cache.weights = w / (w.x + w.y + w.z + w.w);
        return dot(ao, cache.weights);
    } else {
        return textureLod(light_ssao, vec3(cache.uv, 0.0), 0.0).r;
    }
#else
    // SSAO is not applied when blending is enabled
    return 1.0;
#endif
}

float SpecularAO_Lagarde(float NoV, float visibility, float roughness) {
    // Lagarde and de Rousiers 2014, "Moving Frostbite to PBR"
    return saturate(pow(NoV + visibility, exp2(-16.0 * roughness - 1.0)) - 1.0 + visibility);
}

float sphericalCapsIntersection(float cosCap1, float cosCap2, float cosDistance) {
    // Oat and Sander 2007, "Ambient Aperture Lighting"
    // Approximation mentioned by Jimenez et al. 2016
    float r1 = acosFastPositive(cosCap1);
    float r2 = acosFastPositive(cosCap2);
    float d  = acosFast(cosDistance);

    // We work with cosine angles, replace the original paper's use of
    // cos(min(r1, r2)) with max(cosCap1, cosCap2)
    // We also remove a multiplication by 2 * PI to simplify the computation
    // since we divide by 2 * PI in computeBentSpecularAO()

    if (min(r1, r2) <= max(r1, r2) - d) {
        return 1.0 - max(cosCap1, cosCap2);
    } else if (r1 + r2 <= d) {
        return 0.0;
    }

    float delta = abs(r1 - r2);
    float x = 1.0 - saturate((d - delta) / max(r1 + r2 - delta, 1e-4));
    // simplified smoothstep()
    float area = sq(x) * (-2.0 * x + 3.0);
    return area * (1.0 - max(cosCap1, cosCap2));
}

// This function could (should?) be implemented as a 3D LUT instead, but we need to save samplers
float SpecularAO_Cones(vec3 bentNormal, float visibility, float roughness) {
    // Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"

    // aperture from ambient occlusion
    float cosAv = sqrt(1.0 - visibility);
    // aperture from roughness, log(10) / log(2) = 3.321928
    float cosAs = exp2(-3.321928 * sq(roughness));
    // angle betwen bent normal and reflection direction
    float cosB  = dot(bentNormal, shading_reflected);

    // Remove the 2 * PI term from the denominator, it cancels out the same term from
    // sphericalCapsIntersection()
    float ao = sphericalCapsIntersection(cosAv, cosAs, cosB) / (1.0 - cosAs);
    // Smoothly kill specular AO when entering the perceptual roughness range [0.1..0.3]
    // Without this, specular AO can remove all reflections, which looks bad on metals
    return mix(1.0, ao, smoothstep(0.01, 0.09, roughness));
}

/**
 * Computes a specular occlusion term from the ambient occlusion term.
 */
vec3 unpackBentNormal(vec3 bn) {
    // this must match src/materials/ssao/ssaoUtils.fs
    return bn * 2.0 - 1.0;
}

float specularAO(float NoV, float visibility, float roughness, const in SSAOInterpolationCache cache) {
    float specularAO = 1.0;

// SSAO is not applied when blending is enabled
#if defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED)

#if SPECULAR_AMBIENT_OCCLUSION == SPECULAR_AO_SIMPLE
    // TODO: Should we even bother computing this when screen space bent normals are enabled?
    specularAO = SpecularAO_Lagarde(NoV, visibility, roughness);
#elif SPECULAR_AMBIENT_OCCLUSION == SPECULAR_AO_BENT_NORMALS
#   if defined(MATERIAL_HAS_BENT_NORMAL)
        specularAO = SpecularAO_Cones(shading_bentNormal, visibility, roughness);
#   else
        specularAO = SpecularAO_Cones(shading_normal, visibility, roughness);
#   endif
#endif

    if (frameUniforms.aoBentNormals > 0.0) {
        vec3 bn;
        if (frameUniforms.aoSamplingQualityAndEdgeDistance > 0.0) {
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
            vec4 bnr = textureGather(light_ssao, vec3(cache.uv, 1.0), 0);
            vec4 bng = textureGather(light_ssao, vec3(cache.uv, 1.0), 1);
            vec4 bnb = textureGather(light_ssao, vec3(cache.uv, 1.0), 2);
#else
            vec3 s01 = textureLodOffset(light_ssao, vec3(cache.uv, 1.0), 0.0, ivec2(0, 1)).rgb;
            vec3 s11 = textureLodOffset(light_ssao, vec3(cache.uv, 1.0), 0.0, ivec2(1, 1)).rgb;
            vec3 s10 = textureLodOffset(light_ssao, vec3(cache.uv, 1.0), 0.0, ivec2(1, 0)).rgb;
            vec3 s00 = textureLodOffset(light_ssao, vec3(cache.uv, 1.0), 0.0, ivec2(0, 0)).rgb;
            vec4 bnr = vec4(s01.r, s11.r, s10.r, s00.r);
            vec4 bng = vec4(s01.g, s11.g, s10.g, s00.g);
            vec4 bnb = vec4(s01.b, s11.b, s10.b, s00.b);
#endif
            bn.r = dot(bnr, cache.weights);
            bn.g = dot(bng, cache.weights);
            bn.b = dot(bnb, cache.weights);
        } else {
            bn = textureLod(light_ssao, vec3(cache.uv, 1.0), 0.0).xyz;
        }

        bn = unpackBentNormal(bn);
        bn = normalize(bn);

        float ssSpecularAO = SpecularAO_Cones(bn, visibility, roughness);
        // Combine the specular AO from the texture with screen space specular AO
        specularAO = min(specularAO, ssSpecularAO);

        // For now we don't use the screen space AO bent normal for the diffuse because the
        // AO bent normal is currently a face normal.
    }
#endif

    return specularAO;
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
#if MULTI_BOUNCE_AMBIENT_OCCLUSION == 1 && SPECULAR_AMBIENT_OCCLUSION != SPECULAR_AO_OFF
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
