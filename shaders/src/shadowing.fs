//------------------------------------------------------------------------------
// Shadowing configuration
//------------------------------------------------------------------------------

#define SHADOW_SAMPLING_PCF_HARD          0
#define SHADOW_SAMPLING_PCF_LOW           1
#define SHADOW_SAMPLING_PCF_MEDIUM        2
#define SHADOW_SAMPLING_PCF_HIGH          3

#define SHADOW_SAMPLING_ERROR_DISABLED   0
#define SHADOW_SAMPLING_ERROR_ENABLED    1

#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED   0
#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED    1

#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS_MIN_SAMPLING_METHOD    SHADOW_SAMPLING_PCF_MEDIUM

#define SHADOW_SAMPLING_METHOD            SHADOW_SAMPLING_PCF_LOW
#define SHADOW_SAMPLING_ERROR             SHADOW_SAMPLING_ERROR_DISABLED
#define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED

#if SHADOW_SAMPLING_ERROR == SHADOW_SAMPLING_ERROR_ENABLED
  #undef SHADOW_RECEIVER_PLANE_DEPTH_BIAS
  #define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
#elif SHADOW_SAMPLING_METHOD < SHADOW_RECEIVER_PLANE_DEPTH_BIAS_MIN_SAMPLING_METHOD
  #undef SHADOW_RECEIVER_PLANE_DEPTH_BIAS
  #define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED
#endif

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
 #if SHADOW_SAMPLING_METHOD >= SHADOW_RECEIVER_PLANE_DEPTH_BIAS_MIN_SAMPLING_METHOD
    depth += dot(dudv, rpdb);
 #endif
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
    return texture(map, vec4(position.xy, layer, saturate(depth)));
}
#endif

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW
float ShadowSample_PCF_Low(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 size, highp vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    highp vec2 texelSize = vec2(1.0) / size;

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

    vec2 rpdb = computeReceiverPlaneDepthBias(position);

    float depth = samplingBias(position.z, rpdb, texelSize);
    float sum = 0.0;

    sum += uw.x * vw.x * sampleDepth(map, layer, base, vec2(u.x, v.x), depth, rpdb);
    sum += uw.y * vw.x * sampleDepth(map, layer, base, vec2(u.y, v.x), depth, rpdb);

    sum += uw.x * vw.y * sampleDepth(map, layer, base, vec2(u.x, v.y), depth, rpdb);
    sum += uw.y * vw.y * sampleDepth(map, layer, base, vec2(u.y, v.y), depth, rpdb);

    return sum * (1.0 / 16.0);
}
#endif

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_MEDIUM
float ShadowSample_PCF_Medium(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 size, highp vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    highp vec2 texelSize = vec2(1.0) / size;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    highp vec2 uv = (position.xy * size) + offset;
    highp vec2 base = (floor(uv) - offset) * texelSize;
    highp vec2 st = fract(uv);

    vec3 uw = vec3(4.0 - 3.0 * st.x, 7.0, 1.0 + 3.0 * st.x);
    vec3 vw = vec3(4.0 - 3.0 * st.y, 7.0, 1.0 + 3.0 * st.y);

    highp vec3 u = vec3((3.0 - 2.0 * st.x) / uw.x - 2.0, (3.0 + st.x) / uw.y, st.x / uw.z + 2.0);
    highp vec3 v = vec3((3.0 - 2.0 * st.y) / vw.x - 2.0, (3.0 + st.y) / vw.y, st.y / vw.z + 2.0);

    u *= texelSize.x;
    v *= texelSize.y;

    vec2 rpdb = computeReceiverPlaneDepthBias(position);

    float depth = samplingBias(position.z, rpdb, texelSize);
    float sum = 0.0;

    sum += uw.x * vw.x * sampleDepth(map, layer, base, vec2(u.x, v.x), depth, rpdb);
    sum += uw.y * vw.x * sampleDepth(map, layer, base, vec2(u.y, v.x), depth, rpdb);
    sum += uw.z * vw.x * sampleDepth(map, layer, base, vec2(u.z, v.x), depth, rpdb);

    sum += uw.x * vw.y * sampleDepth(map, layer, base, vec2(u.x, v.y), depth, rpdb);
    sum += uw.y * vw.y * sampleDepth(map, layer, base, vec2(u.y, v.y), depth, rpdb);
    sum += uw.z * vw.y * sampleDepth(map, layer, base, vec2(u.z, v.y), depth, rpdb);

    sum += uw.x * vw.z * sampleDepth(map, layer, base, vec2(u.x, v.z), depth, rpdb);
    sum += uw.y * vw.z * sampleDepth(map, layer, base, vec2(u.y, v.z), depth, rpdb);
    sum += uw.z * vw.z * sampleDepth(map, layer, base, vec2(u.z, v.z), depth, rpdb);

    return sum * (1.0 / 144.0);
}
#endif

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HIGH
float ShadowSample_PCF_High(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 size, highp vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    highp vec2 texelSize = vec2(1.0) / size;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    highp vec2 uv = (position.xy * size) + offset;
    highp vec2 base = (floor(uv) - offset) * texelSize;
    highp vec2 st = fract(uv);

    vec4 uw = vec4(
         5.0 * st.x - 6.0,
         11.0 * st.x - 28.0,
        -(11.0 * st.x + 17.0),
        -(5.0 * st.x + 1.0));
    vec4 vw = vec4(
         5.0 * st.y - 6.0,
         11.0 * st.y - 28.0,
        -(11.0 * st.y + 17.0),
        -(5.0 * st.y + 1.0));

    vec4 u = vec4(
         (4.0 * st.x - 5.0) / uw.x - 3.0,
         (4.0 * st.x - 16.0) / uw.y - 1.0,
        -(7.0 * st.x + 5.0) / uw.z + 1.0,
        -st.x / uw.w + 3.0);
    vec4 v = vec4(
         (4.0 * st.y - 5.0) / vw.x - 3.0,
         (4.0 * st.y - 16.0) / vw.y - 1.0,
        -(7.0 * st.y + 5.0) / vw.z + 1.0,
        -st.y / vw.w + 3.0);

    u *= texelSize.x;
    v *= texelSize.y;

    vec2 rpdb = computeReceiverPlaneDepthBias(position);

    float depth = samplingBias(position.z, rpdb, texelSize);
    highp float sum = 0.0;

    sum += uw.x * vw.x * sampleDepth(map, layer, base, vec2(u.x, v.x), depth, rpdb);
    sum += uw.y * vw.x * sampleDepth(map, layer, base, vec2(u.y, v.x), depth, rpdb);
    sum += uw.z * vw.x * sampleDepth(map, layer, base, vec2(u.z, v.x), depth, rpdb);
    sum += uw.w * vw.x * sampleDepth(map, layer, base, vec2(u.w, v.x), depth, rpdb);

    sum += uw.x * vw.y * sampleDepth(map, layer, base, vec2(u.x, v.y), depth, rpdb);
    sum += uw.y * vw.y * sampleDepth(map, layer, base, vec2(u.y, v.y), depth, rpdb);
    sum += uw.z * vw.y * sampleDepth(map, layer, base, vec2(u.z, v.y), depth, rpdb);
    sum += uw.w * vw.y * sampleDepth(map, layer, base, vec2(u.w, v.y), depth, rpdb);

    sum += uw.x * vw.z * sampleDepth(map, layer, base, vec2(u.x, v.z), depth, rpdb);
    sum += uw.y * vw.z * sampleDepth(map, layer, base, vec2(u.y, v.z), depth, rpdb);
    sum += uw.z * vw.z * sampleDepth(map, layer, base, vec2(u.z, v.z), depth, rpdb);
    sum += uw.w * vw.z * sampleDepth(map, layer, base, vec2(u.w, v.z), depth, rpdb);

    sum += uw.x * vw.w * sampleDepth(map, layer, base, vec2(u.x, v.w), depth, rpdb);
    sum += uw.y * vw.w * sampleDepth(map, layer, base, vec2(u.y, v.w), depth, rpdb);
    sum += uw.z * vw.w * sampleDepth(map, layer, base, vec2(u.z, v.w), depth, rpdb);
    sum += uw.w * vw.w * sampleDepth(map, layer, base, vec2(u.w, v.w), depth, rpdb);

    return sum * (1.0 / 2704.0);
}
#endif

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
    float tolerance = abs(rayData.ssViewRayEnd.z - rayData.ssRayStart.z) * dt;

    // dither the ray with interleaved gradient noise
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    float dither = fract(m.z * fract(dot(gl_FragCoord.xy, m.xy))) - 0.5;

    // normalized position on the ray (0 to 1)
    float t = dt * dither + dt;

    highp vec3 ray;
    for (uint i = 0u ; i < kStepCount ; i++, t += dt) {
        ray = rayData.uvRayStart + rayData.uvRay * t;
        float z = textureLod(light_structure, uvToRenderTargetUV(ray.xy), 0.0).r;
        float dz = z - ray.z;
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
        const uint layer, const highp vec3 shadowPosition) {
    highp vec2 size = vec2(textureSize(shadowMap, 0));
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HARD
    return ShadowSample_Hard(shadowMap, layer, size, shadowPosition);
#elif SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW
    return ShadowSample_PCF_Low(shadowMap, layer, size, shadowPosition);
#elif SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_MEDIUM
    return ShadowSample_PCF_Medium(shadowMap, layer, size, shadowPosition);
#elif SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HIGH
    return ShadowSample_PCF_High(shadowMap, layer, size, shadowPosition);
#endif
}

// VSM sampling
float shadow(const mediump sampler2DArray shadowMap,
        const uint layer, const highp vec3 shadowPosition) {
    // note: shadowPosition.z is in linear light space normalized to [0, 1]
    //  see: ShadowMap::computeVsmLightSpaceMatrix() in ShadowMap.cpp
    //  see: computeLightSpacePosition() in common_shadowing.fs

    // Read the shadow map with all available filtering
    highp vec2 moments = texture(shadowMap, vec3(shadowPosition.xy, layer)).xy;
    highp float depth = shadowPosition.z;

    // EVSM depth warping
    depth = depth * 2.0 - 1.0;
    depth = exp(frameUniforms.vsmExponent * depth);

    highp float depthScale = frameUniforms.vsmDepthScale * depth;
    highp float minVariance = depthScale * depthScale;
    float lightBleedReduction = frameUniforms.vsmLightBleedReduction;
    return chebyshevUpperBound(moments, depth, minVariance, lightBleedReduction);
}
