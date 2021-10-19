//------------------------------------------------------------------------------
// Shadowing configuration
//------------------------------------------------------------------------------

#define SHADOW_SAMPLING_PCF_HARD                    0
#define SHADOW_SAMPLING_PCF_LOW                     1

#define SHADOW_SAMPLING_ERROR_DISABLED              0
#define SHADOW_SAMPLING_ERROR_ENABLED               1

#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED   0
#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED    1

#define SHADOW_SAMPLING_METHOD            SHADOW_SAMPLING_PCF_HARD
#define SHADOW_SAMPLING_ERROR             SHADOW_SAMPLING_ERROR_DISABLED
#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED

//------------------------------------------------------------------------------
// Shadow sampling methods
//------------------------------------------------------------------------------

vec2 computeReceiverPlaneDepthBias(const highp vec3 position) {
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    vec2 bias;
#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
    highp vec3 du = dFdx(position);
    highp vec3 dv = dFdy(position);

    // Chain rule we use:
    //     | du.x   du.y |^-T      |  dv.y  -du.y |T    |  dv.y  -dv.x |
    // D * | dv.x   dv.y |     =   | -dv.x   du.x |  =  | -du.y   du.x |

    bias = inverse(mat2(du.xy, dv.xy)) * vec2(du.z, dv.z);
#else
    bias = vec2(0.0);
#endif
    return bias;
}

float samplingBias(float depth, const vec2 rpdb, const highp vec2 texelSize) {
#if SHADOW_SAMPLING_ERROR == SHADOW_SAMPLING_ERROR_ENABLED
    // note: if filtering is set to NEAREST, the 2.0 factor below can be changed to 1.0
    float samplingError = min(2.0 * dot(texelSize, abs(rpdb)), 0.01);
    depth += samplingError;
#endif
    return depth;
}

float sampleDepth(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 base, const highp vec2 dudv, float depth, vec2 rpdb) {
#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
    depth += dot(dudv, rpdb);
#endif
    // depth must be clamped to support floating-point depth formats. This is to avoid comparing a
    // value from the depth texture (which is never greater than 1.0) with a greater-than-one
    // comparison value (which is possible with floating-point formats).
    return texture(map, vec4(base + dudv, layer, saturate(depth)));
}

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HARD
float ShadowSample_Hard(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 size, const highp vec3 position) {
    highp vec2 texelSize = vec2(1.0) / size;
    vec2 rpdb = computeReceiverPlaneDepthBias(position);
    float depth = samplingBias(position.z, rpdb, texelSize);
    return sampleDepth(map,layer, position.xy, vec2(0.0f), depth, rpdb);
}
#endif

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW
float ShadowSample_PCF_Low(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 size, highp vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    highp vec2 texelSize = vec2(1.0) / size;
    vec2 rpdb = computeReceiverPlaneDepthBias(position);
    float depth = samplingBias(position.z, rpdb, texelSize);

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

    sum += uw.x * vw.x * sampleDepth(map, layer, base, vec2(u.x, v.x), depth, rpdb);
    sum += uw.y * vw.x * sampleDepth(map, layer, base, vec2(u.y, v.x), depth, rpdb);

    sum += uw.x * vw.y * sampleDepth(map, layer, base, vec2(u.x, v.y), depth, rpdb);
    sum += uw.y * vw.y * sampleDepth(map, layer, base, vec2(u.y, v.y), depth, rpdb);

    return sum * (1.0 / 16.0);
}
#endif

/*
 * DPCF, PCF with contact hardenning.
 * see "Shadow of Cold War", A scalable approach to shadowing -- by Kevin Myers
 */
float ShadowSample_DPCF(const mediump sampler2DArray map, const uint layer,
        const highp vec2 size, highp vec3 position) {
    highp vec2 texelSize = vec2(1.0) / size;
    vec2 rpdb = computeReceiverPlaneDepthBias(position);
    float depth = samplingBias(position.z, rpdb, texelSize);

    float occluders = 0.0f;
    float occluderDistSum = 0.0f;

    // TODO: this needs a lot of improvements
    const float penumbra = 4.0f;
    const uint SHADOW_TAP_COUNT = 9u;
    vec2 offsets[SHADOW_TAP_COUNT] = vec2[9](
        vec2(-1, -1), vec2(0, -1), vec2(1, -1),
        vec2(-1,  0), vec2(0,  0), vec2(1,  0),
        vec2(-1,  1), vec2(0,  1), vec2(1,  1)
    );

    for (uint tap=0; tap<SHADOW_TAP_COUNT; ++tap) {
        highp vec2 uv = position.xy + offsets[tap] * (texelSize * penumbra);

        vec4 depths;
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
        depths = textureGather(map, vec3(uv, layer), 0); // 01, 11, 10, 00
#else
        depths[0] = textureLodOffset(map, vec3(uv, layer), 0.0, ivec2(0, 1)).r;
        depths[1] = textureLodOffset(map, vec3(uv, layer), 0.0, ivec2(1, 1)).r;
        depths[2] = textureLodOffset(map, vec3(uv, layer), 0.0, ivec2(1, 0)).r;
        depths[3] = textureLodOffset(map, vec3(uv, layer), 0.0, ivec2(0, 0)).r;
#endif
        for (uint d = 0; d<4; ++d) {
            float dist = depths[d] - depth;
            float occluder = step(0.0, dist);
            occluders += occluder;
            occluderDistSum += dist * occluder;
        }
    }

    float occluderAvgDist = occluderDistSum / occluders;
    float w = 1.0f / (4.0f * SHADOW_TAP_COUNT);

    float pcfWeight = saturate(occluderAvgDist / depth);
    float percentageOccluded = saturate(occluders * w);

    percentageOccluded = 2.0f * percentageOccluded - 1.0f;
    float occludedSign = sign(percentageOccluded);
    percentageOccluded = 1.0f - (occludedSign * percentageOccluded);

    percentageOccluded = mix(pow(percentageOccluded, 5.0f), percentageOccluded, pcfWeight);
    percentageOccluded = 1.0 - percentageOccluded;
    percentageOccluded *= occludedSign;
    percentageOccluded = 0.5 * percentageOccluded + 0.5f;
    return 1.0f - percentageOccluded;
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
    uint kStepCount = (frameUniforms.directionalShadows >> 8u) & 0xFFu;
    float kDistanceMax = frameUniforms.ssContactShadowDistance;

    ScreenSpaceRay rayData;
    initScreenSpaceRay(rayData, shading_position, lightDirection, kDistanceMax);

    // step
    highp float dt = 1.0 / float(kStepCount);

    // tolerance
    highp float tolerance = abs(rayData.ssViewRayEnd.z - rayData.ssRayStart.z) * dt;

    // dither the ray with interleaved gradient noise
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    float dither = fract(m.z * fract(dot(gl_FragCoord.xy, m.xy))) - 0.5;

    // normalized position on the ray (0 to 1)
    highp float t = dt * dither + dt;

    highp vec3 ray;
    for (uint i = 0u ; i < kStepCount ; i++, t += dt) {
        ray = rayData.uvRayStart + rayData.uvRay * t;
        highp float z = textureLod(light_structure, uvToRenderTargetUV(ray.xy), 0.0).r;
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
// VSM
//------------------------------------------------------------------------------

float linstep(const float min, const float max, const float v) {
    // we could use smoothstep() too
    return clamp((v - min) / (max - min), 0.0, 1.0);
}

float reduceLightBleed(const float pMax, const float amount) {
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, pMax);
}

float chebyshevUpperBound(const highp vec2 moments, const highp float mean,
        const highp float minVariance, const float lightBleedReduction) {
    // Donnelly and Lauritzen 2006, "Variance Shadow Maps"

    highp float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    highp float d = mean - moments.x;
    float pMax = variance / (variance + d * d);

    pMax = reduceLightBleed(pMax, lightBleedReduction);

    return mean <= moments.x ? 1.0 : pMax;
}

//------------------------------------------------------------------------------
// Shadow sampling dispatch
//------------------------------------------------------------------------------

/**
 * Samples the light visibility at the specified position in light (shadow)
 * space. The output is a filtered visibility factor that can be used to multiply
 * the light intensity.
 */

// PCF sampling
float shadow(const mediump sampler2DArrayShadow shadowMap,
        const uint layer, const highp vec4 shadowPosition) {
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
    highp vec2 size = vec2(textureSize(shadowMap, 0));
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HARD
    return ShadowSample_Hard(shadowMap, layer, size, position);
#elif SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW
    return ShadowSample_PCF_Low(shadowMap, layer, size, position);
#endif
}

#define SHADOW_SAMPLING_RUNTIME_VSM     0
#define SHADOW_SAMPLING_RUNTIME_DPCF    1

// VSM or DPCF sampling
float shadow(const mediump sampler2DArray shadowMap,
        const uint layer, const highp vec4 shadowPosition) {

    if (frameUniforms.shadowSamplingType == SHADOW_SAMPLING_RUNTIME_VSM) {

        // note: shadowPosition.z is in linear light space normalized to [0, 1]
        //  see: ShadowMap::computeVsmLightSpaceMatrix() in ShadowMap.cpp
        //  see: computeLightSpacePosition() in common_shadowing.fs

        highp vec3 position = vec3(shadowPosition.xy * (1.0 / shadowPosition.w), shadowPosition.z);

        // Read the shadow map with all available filtering
        highp vec2 moments = texture(shadowMap, vec3(position.xy, layer)).xy;
        highp float depth = position.z;

        // EVSM depth warping
        depth = depth * 2.0 - 1.0;
        depth = exp(frameUniforms.vsmExponent * depth);

        highp float depthScale = frameUniforms.vsmDepthScale * depth;
        highp float minVariance = depthScale * depthScale;
        float lightBleedReduction = frameUniforms.vsmLightBleedReduction;
        return chebyshevUpperBound(moments, depth, minVariance, lightBleedReduction);

    } else if (frameUniforms.shadowSamplingType == SHADOW_SAMPLING_RUNTIME_DPCF) {

        highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
        highp vec2 size = vec2(textureSize(shadowMap, 0));
        return ShadowSample_DPCF(shadowMap, layer, size, position);

    } else {
        // should not happen
        return 0.0f;
    }
}
