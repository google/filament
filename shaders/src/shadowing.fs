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

#ifdef TARGET_MOBILE
  #define SHADOW_SAMPLING_METHOD            SHADOW_SAMPLING_PCF_LOW
  #define SHADOW_SAMPLING_ERROR             SHADOW_SAMPLING_ERROR_DISABLED
  #define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED
#else
  #define SHADOW_SAMPLING_METHOD            SHADOW_SAMPLING_PCF_LOW
  #define SHADOW_SAMPLING_ERROR             SHADOW_SAMPLING_ERROR_DISABLED
  #define SHADOW_RECEIVER_PLANE_DEPTH_BIAS  SHADOW_RECEIVER_PLANE_DEPTH_BIAS_DISABLED
#endif

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

vec2 computeReceiverPlaneDepthBias(const vec3 position) {
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    vec2 bias;
#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
    vec3 du = dFdx(position);
    vec3 dv = dFdy(position);

    // Chain rule we use:
    //     | du.x   du.y |^-T      |  dv.y  -du.y |T    |  dv.y  -dv.x |
    // D * | dv.x   dv.y |     =   | -dv.x   du.x |  =  | -du.y   du.x |

    bias = inverse(mat2(du.xy, dv.xy)) * vec2(du.z, dv.z);
#else
    bias = vec2(0.0);
#endif
    return bias;
}

float samplingBias(float depth, const vec2 rpdb, const vec2 texelSize) {
#if SHADOW_SAMPLING_ERROR == SHADOW_SAMPLING_ERROR_ENABLED
    // note: if filtering is set to NEAREST, the 2.0 factor below can be changed to 1.0
    float samplingError = min(2.0 * dot(texelSize, abs(rpdb)), 0.01);
    depth -= samplingError;
#endif
    return depth;
}

float sampleDepth(const lowp sampler2DArrayShadow map, const uint layer, vec2 base, vec2 dudv, float depth, vec2 rpdb) {
#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
 #if SHADOW_SAMPLING_METHOD >= SHADOW_RECEIVER_PLANE_DEPTH_BIAS_MIN_SAMPLING_METHOD
    depth += dot(dudv, rpdb);
 #endif
#endif
    // depth must be clamped to support floating-point depth formats. This is to avoid comparing a
    // value from the depth texture (which is never greater than 1.0) with a greater-than-one
    // comparison value (which is possible with floating-point formats).
    return texture(map, vec4(base + dudv, layer, clamp(depth, 0.0, 1.0)));
}

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_HARD
float ShadowSample_Hard(const lowp sampler2DArrayShadow map, const uint layer, const vec2 size, const vec3 position) {
    vec2 rpdb = computeReceiverPlaneDepthBias(position);
    float depth = samplingBias(position.z, rpdb, vec2(1.0) / size);
    return texture(map, vec4(position.xy, layer, clamp(depth, 0.0, 1.0)));
}
#endif

#if SHADOW_SAMPLING_METHOD == SHADOW_SAMPLING_PCF_LOW
float ShadowSample_PCF_Low(const lowp sampler2DArrayShadow map, const uint layer, const vec2 size, vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    vec2 texelSize = vec2(1.0) / size;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    vec2 uv = (position.xy * size) + offset;
    vec2 base = (floor(uv) - offset) * texelSize;
    vec2 st = fract(uv);

    vec2 uw = vec2(3.0 - 2.0 * st.x, 1.0 + 2.0 * st.x);
    vec2 vw = vec2(3.0 - 2.0 * st.y, 1.0 + 2.0 * st.y);

    vec2 u = vec2((2.0 - st.x) / uw.x - 1.0, st.x / uw.y + 1.0);
    vec2 v = vec2((2.0 - st.y) / vw.x - 1.0, st.y / vw.y + 1.0);

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
float ShadowSample_PCF_Medium(const lowp sampler2DArrayShadow map, const uint layer, const vec2 size, vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    vec2 texelSize = vec2(1.0) / size;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    vec2 uv = (position.xy * size) + offset;
    vec2 base = (floor(uv) - offset) * texelSize;
    vec2 st = fract(uv);

    vec3 uw = vec3(4.0 - 3.0 * st.x, 7.0, 1.0 + 3.0 * st.x);
    vec3 vw = vec3(4.0 - 3.0 * st.y, 7.0, 1.0 + 3.0 * st.y);

    vec3 u = vec3((3.0 - 2.0 * st.x) / uw.x - 2.0, (3.0 + st.x) / uw.y, st.x / uw.z + 2.0);
    vec3 v = vec3((3.0 - 2.0 * st.y) / vw.x - 2.0, (3.0 + st.y) / vw.y, st.y / vw.z + 2.0);

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
float ShadowSample_PCF_High(const lowp sampler2DArrayShadow map, const uint layer, const vec2 size, vec3 position) {
    //  Castaño, 2013, "Shadow Mapping Summary Part 1"
    vec2 texelSize = vec2(1.0) / size;

    // clamp position to avoid overflows below, which cause some GPUs to abort
    position.xy = clamp(position.xy, vec2(-1.0), vec2(2.0));

    vec2 offset = vec2(0.5);
    vec2 uv = (position.xy * size) + offset;
    vec2 base = (floor(uv) - offset) * texelSize;
    vec2 st = fract(uv);

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
    float sum = 0.0;

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

    // ray start/end in clip space
    highp vec4 csRayStart = worldToClip * vec4(wsRayStart, 1.0);
    highp vec4 csRayEnd = worldToClip * vec4(wsRayEnd, 1.0);
    highp vec4 csViewRayEnd = csRayStart + viewToClip * vec4(0.0, 0.0, wsRayLength, 0.0);

    // ray start/end in screen space
    ray.ssRayStart = csRayStart.xyz * 1.0 / csRayStart.w;
    ray.ssRayEnd = csRayEnd.xyz * 1.0 / csRayEnd.w;
    ray.ssViewRayEnd = csViewRayEnd.xyz * 1.0 / csViewRayEnd.w;

    // convert all to uv (texture) space
    highp vec3 uvRayEnd = ray.ssRayEnd.xyz * 0.5 + 0.5;
    ray.uvRayStart = ray.ssRayStart.xyz * 0.5 + 0.5;
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
    float tolerance = abs(rayData.ssViewRayEnd.z - rayData.ssRayStart.z) * dt * 0.5;

    // dithter the ray with interleaved grandient noise
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    float dither = fract(m.z * fract(dot(gl_FragCoord.xy, m.xy))) - 0.5;

    // normalized postion on the ray (0 to 1)
    float t = dt * dither + dt;

    highp vec3 ray;
    for (uint i = 0u ; i < kStepCount ; i++, t += dt) {
        ray = rayData.uvRayStart + rayData.uvRay * t;
        float z = textureLod(light_structure, uvToRenderTargetUV(ray.xy), 0.0).r;
        float dz = ray.z - z;
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

highp float linstep(const highp float a, const highp float b, const highp float v) {
    return clamp((v - a) / (b - a), 0.0, 1.0);
}

highp float reduceLightBleed(const highp float pMax, const highp float amount) {
    return linstep(amount, 1.0, pMax);
}

highp float chebyshevUpperBound(const highp vec2 moments, const highp float mean,
        const highp float minVariance, const highp float lightBleedReduction) {
    // Donnelly and Lauritzen 2006, "Variance Shadow Maps"

    highp float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    highp float d = mean - moments.x;
    highp float pMax = variance / (variance + d * d);
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
float shadow(const lowp sampler2DArrayShadow shadowMap, const uint layer, const vec3 shadowPosition) {
    vec2 size = vec2(textureSize(shadowMap, 0));
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

float shadowVsm(const highp sampler2DArray shadowMap, const uint layer, const highp vec3 shadowPosition,
        const highp float fragDepth) {
    const highp vec2 moments = texture(shadowMap, vec3(shadowPosition.xy, layer)).xy;

    // TODO: bias and lightBleedReduction should be uniforms
    const float bias = 0.01;
    const float lightBleedReduction = 0.2;

    const float minVariance = bias * 0.01;
    return chebyshevUpperBound(moments, fragDepth, minVariance, lightBleedReduction);
}
