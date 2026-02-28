//------------------------------------------------------------------------------
// Shadow Sampling Types
//------------------------------------------------------------------------------

// Keep this in sync with PerViewUniforms.h
#define SHADOW_SAMPLING_RUNTIME_PCF     0u
#define SHADOW_SAMPLING_RUNTIME_EVSM    1u
#define SHADOW_SAMPLING_RUNTIME_PCSS    2u

#define SHADOW_SAMPLING_PCF_HARD        0
#define SHADOW_SAMPLING_PCF_LOW         1

// number of samples for PCSS w/ EVSM, must be 1 or 5
#define SHADOW_SAMPLING_PCSS_TAPS       5
// whether to use a rotated noise (1) or not (0)
#define SHADOW_SAMPLING_PCSS_NOISE      1

//------------------------------------------------------------------------------
// PCF Shadow Sampling
//------------------------------------------------------------------------------

float sampleDepth(const mediump sampler2DArrayShadow map,
        const highp vec4 scissorNormalized,
        const uint layer,  highp vec2 uv, highp float depth) {

    // clamp needed for directional lights and/or large kernels
    uv = clamp(uv, scissorNormalized.xy, scissorNormalized.zw);

    // depth must be clamped to support floating-point depth formats which are always in
    // the range [0, 1].
    return texture(map, vec4(uv, layer, saturate(depth)));
}

// use hardware assisted PCF
float ShadowSample_PCF_Hard(const mediump sampler2DArrayShadow map,
        const highp vec4 scissorNormalized,
        const uint layer, const highp vec4 shadowPosition) {
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
    return sampleDepth(map, scissorNormalized, layer, position.xy, position.z);
}

// use hardware assisted PCF + 3x3 gaussian filter
float ShadowSample_PCF_Low(const mediump sampler2DArrayShadow map,
        const highp vec4 scissorNormalized,
        const uint layer, const highp vec4 shadowPosition) {
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
    highp vec2 size = vec2(textureSize(map, 0));
    highp vec2 texelSize = vec2(1.0) / size;

    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    highp float depth = position.z;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    highp vec2 uv = (position.xy * size) + offset;
    highp vec2 base = (floor(uv) - offset) * texelSize;
    highp vec2 st = fract(uv);

    vec2 uw = vec2(3.0 - 2.0 * st.x, 1.0 + 2.0 * st.x);
    vec2 vw = vec2(3.0 - 2.0 * st.y, 1.0 + 2.0 * st.y);

    highp vec2 u = vec2((2.0 - st.x) / uw.x - 1.0, st.x / uw.y + 1.0);
    highp vec2 v = vec2((2.0 - st.y) / vw.x - 1.0, st.y / vw.y + 1.0);

    u *= texelSize.x;
    v *= texelSize.y;

    float sum = 0.0;
    sum += uw.x * vw.x * sampleDepth(map, scissorNormalized, layer, base + vec2(u.x, v.x), depth);
    sum += uw.y * vw.x * sampleDepth(map, scissorNormalized, layer, base + vec2(u.y, v.x), depth);
    sum += uw.x * vw.y * sampleDepth(map, scissorNormalized, layer, base + vec2(u.x, v.y), depth);
    sum += uw.y * vw.y * sampleDepth(map, scissorNormalized, layer, base + vec2(u.y, v.y), depth);
    return sum * (1.0 / 16.0);
}

// use manual PCF
float ShadowSample_PCF(const mediump sampler2DArray map,
        const highp vec4 scissorNormalized,
        const uint layer, const highp vec4 shadowPosition) {
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
    highp vec2 tc = clamp(position.xy, scissorNormalized.xy, scissorNormalized.zw);
    return step(0.0, position.z - textureLod(map, vec3(tc, layer), 0.0).r);
}

highp vec2 computeReceiverPlaneDepthBias(const highp vec3 position) {
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    // Chain rule to compute dz/du and dz/dv
    // |dz/du|   |du/dx du/dy|^-T   |dz/dx|
    // |dz/dv| = |dv/dx dv/dy|    * |dz/dy|
    highp vec3 duvz_dx = dFdx(position);
    highp vec3 duvz_dy = dFdy(position);
    highp vec2 dz_duv = inverse(transpose(mat2(duvz_dx.xy, duvz_dy.xy))) * vec2(duvz_dx.z, duvz_dy.z);
    return dz_duv;
}

//------------------------------------------------------------------------------
// EVSM
//------------------------------------------------------------------------------

float chebyshevUpperBound(const highp vec2 moments, const highp float depth,
        const highp float minVariance, const highp float lbrAmount) {
    // Fast path: if the receiver is fully in front of the caster
    if (depth <= moments.x) {
        return 1.0;
    }

    // Calculate variance with our dynamically injected floor
    highp float variance = max(moments.y - (moments.x * moments.x), minVariance);

    // Standard Chebyshev inequality
    highp float d = depth - moments.x;
    highp float p_max = variance / (variance + d * d);

    // Apply Light Bleeding Reduction (LBR)
    return saturate((p_max - lbrAmount) / (1.0 - lbrAmount));
}

float evaluateEVSM(const bool ELVSM, float c,
        const highp vec4 moments, const highp float zReceiver,
        const highp vec2 dzduv, const highp vec2 texelSize) {
    const highp float EPSILON_MULTIPLIER = 0.002; // could be 0.00001 in fp32
    float lbrAmount = frameUniforms.vsmLightBleedReduction;

    // Scale the UV-space gradient down to a single shadow map texel footprint.
    highp vec2 texel_dzduv = dzduv * texelSize;

    // squared magnitude of the linear depth gradient across a single shadow map texel footprint
    highp float dz2 = dot(texel_dzduv, texel_dzduv);

    // remap depth to [-1, 1]
    highp float depth = zReceiver * 2.0 - 1.0;

    // positive wrap
    highp float pw = exp(c * depth);
    highp float epsilon = EPSILON_MULTIPLIER * (pw * pw);
    // Dynamic variance for the positive side (derivative of wraped depth w.r.t. light-space depth via Chain Rule)
    highp float dpwdz = 2.0 * c * pw;
    highp float pMinVariance = epsilon + 0.25 * (dpwdz * dpwdz) * dz2;
    float p = chebyshevUpperBound(moments.xy, pw, pMinVariance, lbrAmount);

    // negative wrap
    if (ELVSM) {
        highp float nw = -1.0 / pw;
        highp float epsilon = EPSILON_MULTIPLIER * (nw * nw);
        // Dynamic variance for the negative side (derivative of wraped depth w.r.t. light-space depth via Chain Rule)
        highp float dnwdz = 2.0 * c * nw;
        highp float nMinVariance = epsilon + 0.25 * (dnwdz * dnwdz) * dz2;
        float n = chebyshevUpperBound(moments.zw, nw, nMinVariance, lbrAmount);
       p = min(p, n);
    }

    return p;
}

float ShadowSample_VSM(const bool DIRECTIONAL, const highp sampler2DArray shadowMap,
        const highp vec4 scissorNormalized,
        const uint layer, const int index,
        const highp vec4 shadowPosition, const highp float zLight) {

    bool ELVSM = shadowUniforms.shadows[index].elvsm;
    float c = shadowUniforms.shadows[index].vsmExponent;
    highp vec2 texelSize = vec2(1.0) / vec2(textureSize(shadowMap, 0)); // TODO: put this in a uniform

    // note: shadowPosition.z is in linear light-space normalized to [0, 1]
    //  see: ShadowMap::computeVsmLightSpaceMatrix() in ShadowMap.cpp
    //  see: computeLightSpacePosition() in common_shadowing.fs
    highp vec3 position = vec3(shadowPosition.xy * (1.0 / shadowPosition.w), shadowPosition.z);

    // plane receiver bias to reduce shadow acnee on the received plane (before clamp)
    highp vec2 dzduv = computeReceiverPlaneDepthBias(position);

    // clamp uv to border
    position.xy = clamp(position.xy, scissorNormalized.xy, scissorNormalized.zw);

    // Read the shadow map with all available filtering
    highp vec4 moments = texture(shadowMap, vec3(position.xy, layer));

    return evaluateEVSM(ELVSM, c, moments, position.z, dzduv, texelSize);
}

// Extracts the average blocker depth from EVSM moments in O(1)
highp float computeBlockerDistance(float c, highp vec4 searchMoments, highp float zReceiver, float pLit) {
    // 1. Unwarp the expected depth (E[x]) from the positive moment.
    // moments.x ≈ exp(c * (2z - 1))  =>  ln(moments.x) / c ≈ (2z - 1)
    // We clamp the moment to prevent log(0) in areas of extreme negative depth.
    highp float warpedZ = max(searchMoments.x, 1e-8);
    highp float zAvg = (log(warpedZ) / c + 1.0) * 0.5;

    // 2. The VSSM Blocker Equation:
    highp float pUnlit = 1.0 - pLit;
    highp float zBlocker = (zAvg - (pLit * zReceiver)) / pUnlit;

    // 3. Clamp to valid physical bounds [0, zReceiver]
    return clamp(zBlocker, 0.0, zReceiver);
}

float ShadowSample_PCSS_VSM(const bool DIRECTIONAL, const highp sampler2DArray shadowMap,
        const highp vec4 scissorNormalized,
        const uint layer, const int index,
        const highp vec4 shadowPosition, const highp float zLight) {

    bool ELVSM = shadowUniforms.shadows[index].elvsm;
    float c = shadowUniforms.shadows[index].vsmExponent;
    float bulbRadiusLs = shadowUniforms.shadows[index].bulbRadiusLs;

    // PCSS parameters
    const float MAX_MIP_LEVEL = 3.0; // Your 4-level chain (0, 1, 2, 3)

    highp vec2 texelSize = vec2(1.0) / vec2(textureSize(shadowMap, 0));
    highp vec3 position = vec3(shadowPosition.xy * (1.0 / shadowPosition.w), shadowPosition.z);

    // 1. Compute derivatives BEFORE clamping UVs to prevent dzduv from zeroing out on edges
    highp vec2 dzduv = computeReceiverPlaneDepthBias(position);
    position.xy = clamp(position.xy, scissorNormalized.xy, scissorNormalized.zw);

    // ==========================================
    // STEP 1: DYNAMIC O(1) BLOCKER SEARCH
    // ==========================================
    // Calculate the search radius in texels.
    // We scale the baseline light size by the receiver's depth.
    // The further away the receiver is from the light/shadow map plane,
    // the wider the search cone becomes at the texture surface.
    highp float searchRadiusInTexels = bulbRadiusLs * position.z;

    // Convert the search radius into a Mipmap LOD level, exactly like the penumbra filter.
    // A radius of 1 texel = LOD 0. Radius of 2 = LOD 1. Radius of 4 = LOD 2.
    // We clamp it to MAX_MIP_LEVEL so we don't fetch non-existent textures.
    float searchLod = clamp(log2(max(searchRadiusInTexels, 1.0)), 0.0, MAX_MIP_LEVEL);

    // Fetch moments using hardware trilinear interpolation to get a perfectly smooth,
    // continuously varying search area as the receiver moves.
    highp vec4 searchMoments = textureLod(shadowMap, vec3(position.xy, layer), searchLod);

    // Scale the texel footprint for the dynamic variance floor based on the calculated search LOD
    highp vec2 searchTexelSize = texelSize * exp2(searchLod);

    // Evaluate the probability that this dynamically sized macro-region is lit
    float pLitSearch = evaluateEVSM(ELVSM, c, searchMoments, position.z, dzduv, searchTexelSize);

    // If the region is almost entirely lit, skip the penumbra filter.
    if (pLitSearch >= 0.999) {
        return 1.0;
    }

    // If the region is almost entirely occluded, skip the penumbra filter.
    if (pLitSearch <= 0.001) {
        return 0.0;
    }

    // Extract the average depth of the occluding pixels
    highp float zBlocker = computeBlockerDistance(c, searchMoments, position.z, pLitSearch);

    // ==========================================
    // STEP 2: PENUMBRA ESTIMATION
    // ==========================================
    float penumbraWidthInTexels;
    if (DIRECTIONAL) {
        // Orthographic projection (Cascades)
        penumbraWidthInTexels = (position.z - zBlocker) * bulbRadiusLs;
    } else {
        // Perspective projection (Spot/Point lights)
        penumbraWidthInTexels = ((position.z - zBlocker) / zBlocker) * bulbRadiusLs;
    }

    // ==========================================
    // STEP 3: PENUMBRA FILTERING (Compile-Time Variants)
    // ==========================================
    float targetLod = clamp(log2(max(penumbraWidthInTexels, 1.0)), 0.0, MAX_MIP_LEVEL);

    // The base footprint of one texel at the chosen LOD
    highp vec2 mipTexelSize = texelSize * exp2(targetLod);
    highp vec2 r = mipTexelSize * 0.5;

    highp vec4 finalMoments;
    highp vec2 effectiveTexelSize;

    // Pre-calculate trigonometric noise once if ANY noise variant is enabled
    #if defined(SHADOW_SAMPLING_PCSS_NOISE) && SHADOW_SAMPLING_PCSS_NOISE == 1
        highp float noise = interleavedGradientNoise(gl_FragCoord.xy);
        // Multiply by 2 * PI (6.2831853) to get a full 360-degree random rotation
        highp float theta = noise * 6.28318530718;
        highp float cosT = cos(theta);
        highp float sinT = sin(theta);
    #endif

        // ---------------------------------------------------------
        // VARIANT A: 1-TAP (Standard or Jittered)
        // ---------------------------------------------------------
    #if SHADOW_SAMPLING_PCSS_TAPS == 1
        #if defined(SHADOW_SAMPLING_PCSS_NOISE) && SHADOW_SAMPLING_PCSS_NOISE == 1
            // Randomly offset the fetch coordinate along a circular radius
            highp vec2 jitterOffset = vec2(cosT, sinT) * r;
            highp vec2 uv = clamp(position.xy + jitterOffset, scissorNormalized.xy, scissorNormalized.zw);
            finalMoments = textureLod(shadowMap, vec3(uv, layer), targetLod);

            // Jittering effectively widens the sampled area over time,
            // so we must double the variance footprint to prevent acne.
            effectiveTexelSize = mipTexelSize * 2.0;
        #else
            // Pure standard trilinear fetch
            highp vec2 uv = clamp(position.xy, scissorNormalized.xy, scissorNormalized.zw);
            finalMoments = textureLod(shadowMap, vec3(uv, layer), targetLod);
            effectiveTexelSize = mipTexelSize;
        #endif

        // ---------------------------------------------------------
        // VARIANT B: 5-TAP QUINCUNX (Standard or Rotated)
        // ---------------------------------------------------------
    #elif SHADOW_SAMPLING_PCSS_TAPS == 5
        highp vec2 uvCenter = clamp(position.xy, scissorNormalized.xy, scissorNormalized.zw);
        highp vec4 mC = textureLod(shadowMap, vec3(uvCenter, layer), targetLod);
        #if defined(SHADOW_SAMPLING_PCSS_NOISE) && SHADOW_SAMPLING_PCSS_NOISE == 1
            // Calculate the randomly rotated base corner offsets
            highp vec2 rot0 = vec2(cosT - sinT, sinT + cosT) * r;
            highp vec2 rot1 = vec2(cosT + sinT, sinT - cosT) * r;
            highp vec2 uv0 = clamp(position.xy + rot0, scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv1 = clamp(position.xy + rot1, scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv2 = clamp(position.xy - rot0, scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv3 = clamp(position.xy - rot1, scissorNormalized.xy, scissorNormalized.zw);
        #else
            // Standard axis-aligned Quincunx corners
            highp vec2 uv0 = clamp(position.xy + vec2(-r.x, -r.y), scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv1 = clamp(position.xy + vec2( r.x, -r.y), scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv2 = clamp(position.xy + vec2(-r.x,  r.y), scissorNormalized.xy, scissorNormalized.zw);
            highp vec2 uv3 = clamp(position.xy + vec2( r.x,  r.y), scissorNormalized.xy, scissorNormalized.zw);
        #endif

        highp vec4 m0 = textureLod(shadowMap, vec3(uv0, layer), targetLod);
        highp vec4 m1 = textureLod(shadowMap, vec3(uv1, layer), targetLod);
        highp vec4 m2 = textureLod(shadowMap, vec3(uv2, layer), targetLod);
        highp vec4 m3 = textureLod(shadowMap, vec3(uv3, layer), targetLod);

        // Safe FP16 FMA accumulation
        finalMoments = (mC * 0.5) + (m0 * 0.125) + (m1 * 0.125) + (m2 * 0.125) + (m3 * 0.125);
        effectiveTexelSize = mipTexelSize * 2.0;
    #endif

    // Evaluate EVSM exactly once using the selected footprint and moments
    return evaluateEVSM(ELVSM, c, finalMoments, position.z, dzduv, effectiveTexelSize);
}

//------------------------------------------------------------------------------
// Screen-space Contact Shadows
//------------------------------------------------------------------------------

struct ScreenSpaceRay {
    highp vec3 ssRayStart;
    highp vec3 ssRayEnd;
    highp vec3 ssViewRayEnd;
    highp vec3 uvRayStart;
    highp vec3 uvRay;
};

void initScreenSpaceRay(out ScreenSpaceRay ray, highp vec3 wsRayStart, vec3 wsRayDirection, float wsRayLength) {
    highp mat4 worldToClip = getClipFromWorldMatrix();
    highp mat4 viewToClip = getClipFromViewMatrix();

    // ray end in world space
    highp vec3 wsRayEnd = wsRayStart + wsRayDirection * wsRayLength;

    // ray start/end in clip space (z is inverted: [1,0])
    highp vec4 csRayStart = worldToClip * vec4(wsRayStart, 1.0);
    highp vec4 csRayEnd = worldToClip * vec4(wsRayEnd, 1.0);
    highp vec4 csViewRayEnd = csRayStart + viewToClip * vec4(0.0, 0.0, wsRayLength, 0.0);

    // ray start/end in screen space (z is inverted: [1,0])
    ray.ssRayStart = csRayStart.xyz * (1.0 / csRayStart.w);
    ray.ssRayEnd = csRayEnd.xyz * (1.0 / csRayEnd.w);
    ray.ssViewRayEnd = csViewRayEnd.xyz * (1.0 / csViewRayEnd.w);

    // convert all to uv (texture) space (z is inverted: [1,0])
    highp vec3 uvRayEnd = vec3(ray.ssRayEnd.xy * 0.5 + 0.5, ray.ssRayEnd.z);
    ray.uvRayStart = vec3(ray.ssRayStart.xy * 0.5 + 0.5, ray.ssRayStart.z);
    ray.uvRay = uvRayEnd - ray.uvRayStart;
}

float screenSpaceContactShadow(vec3 lightDirection) {
    // cast a ray in the direction of the light
    float occlusion = 0.0;
    int kStepCount = (frameUniforms.directionalShadows >> 8) & 0xFF;
    float kDistanceMax = frameUniforms.ssContactShadowDistance;

    ScreenSpaceRay rayData;
    initScreenSpaceRay(rayData, shading_position, lightDirection, kDistanceMax);

    // step
    highp float dt = 1.0 / float(kStepCount);

    // tolerance
    highp float tolerance = abs(rayData.ssViewRayEnd.z - rayData.ssRayStart.z) * dt;

    // dither the ray with interleaved gradient noise
    float dither = interleavedGradientNoise(gl_FragCoord.xy) - 0.5;

    // normalized position on the ray (0 to 1)
    highp float t = dt * dither + dt;

    highp vec3 ray;
    for (int i = 0 ; i < kStepCount ; i++, t += dt) {
        ray = rayData.uvRayStart + rayData.uvRay * t;
        highp float z = textureLod(sampler0_structure, uvToRenderTargetUV(ray.xy), 0.0).r;
        highp float dz = z - ray.z;
        if (abs(tolerance - dz) < tolerance) {
            occlusion = 1.0;
            break;
        }
    }

    // we fade out the contribution of contact shadows towards the edge of the screen
    // because we don't have depth data there
    vec2 fade = max(12.0 * abs(ray.xy - 0.5) - 5.0, 0.0);
    occlusion *= saturate(1.0 - dot(fade, fade));
    return occlusion;
}

//------------------------------------------------------------------------------
// Shadow sampling dispatch
//------------------------------------------------------------------------------

/**
 * Samples the light visibility at the specified position in light (shadow)
 * space. The output is a filtered visibility factor that can be used to multiply
 * the light intensity.
 */

// get texture coordinate for directional and spot shadow maps
#if defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
highp vec4 getShadowPosition(const int cascade) {
    return getCascadeLightSpacePosition(cascade);
}
#endif

#if defined(VARIANT_HAS_DYNAMIC_LIGHTING)
highp vec4 getShadowPosition(const int index,  const highp vec3 dir, const highp float zLight) {
    return getSpotLightSpacePosition(index, dir, zLight);
}
#endif

int getPointLightFace(const highp vec3 r) {
    highp vec4 tc;
    highp float rx = abs(r.x);
    highp float ry = abs(r.y);
    highp float rz = abs(r.z);
    highp float d = max(rx, max(ry, rz));
    if (d == rx) {
        return (r.x >= 0.0 ? 0 : 1);
    } else if (d == ry) {
        return (r.y >= 0.0 ? 2 : 3);
    } else {
        return (r.z >= 0.0 ? 4 : 5);
    }
}

#if defined(MATERIAL_HAS_SHADOW_STRENGTH)
void applyShadowStrength(inout float visibility, float strength) {
    visibility = 1.0 - (1.0 - visibility) * strength;
}
#endif

// PCF sampling
float shadow(const bool DIRECTIONAL,
        const mediump sampler2DArrayShadow shadowMap,
        const int index, highp vec4 shadowPosition, highp float zLight) {
    highp vec4 scissorNormalized = shadowUniforms.shadows[index].scissorNormalized;
    uint layer = shadowUniforms.shadows[index].layer;

    if (CONFIG_SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HARD) {
        return ShadowSample_PCF_Hard(shadowMap, scissorNormalized, layer, shadowPosition);
    } else if (CONFIG_SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW) {
        return ShadowSample_PCF_Low(shadowMap, scissorNormalized, layer, shadowPosition);
    }

    // should not happen
    return 0.0;
}

// Shadow requiring a sampler2D sampler (VSM and PCSS)
float shadow(const bool DIRECTIONAL,
        const highp sampler2DArray shadowMap,
        const int index, highp vec4 shadowPosition, highp float zLight) {
    highp vec4 scissorNormalized = shadowUniforms.shadows[index].scissorNormalized;
    uint layer = shadowUniforms.shadows[index].layer;
    // This conditional is resolved at compile time
    if (frameUniforms.shadowSamplingType == SHADOW_SAMPLING_RUNTIME_EVSM) {
        return ShadowSample_VSM(DIRECTIONAL, shadowMap, scissorNormalized, layer, index,
                shadowPosition, zLight);
    }

    if (frameUniforms.shadowSamplingType == SHADOW_SAMPLING_RUNTIME_PCSS) {
        return ShadowSample_PCSS_VSM(DIRECTIONAL, shadowMap, scissorNormalized, layer, index,
                shadowPosition, zLight);
    }

    if (frameUniforms.shadowSamplingType == SHADOW_SAMPLING_RUNTIME_PCF) {
        // This is here mostly for debugging at this point.
        // Note: In this codepath, the normal bias is not applied because we're in the VSM variant.
        // (see: get{Cascade|Spot}LightSpacePosition)
        return ShadowSample_PCF(shadowMap, scissorNormalized, layer,
                shadowPosition);
    }

    // should not happen
    return 0.0;
}
