//------------------------------------------------------------------------------
// PCF Shadow Sampling
//------------------------------------------------------------------------------

float sampleDepth(const mediump sampler2DArrayShadow map, const uint layer,
        const highp vec2 uv, float depth) {
    // depth must be clamped to support floating-point depth formats. This is to avoid comparing a
    // value from the depth texture (which is never greater than 1.0) with a greater-than-one
    // comparison value (which is possible with floating-point formats).
    return texture(map, vec4(uv, layer, saturate(depth)));
}

// use hardware assisted PCF
float ShadowSample_PCF(const mediump sampler2DArrayShadow map,
        const uint layer, const highp vec3 position) {
    return sampleDepth(map, layer, position.xy, position.z);
}

// use manual PCF
float ShadowSample_PCF(const mediump sampler2DArray shadowMap,
        const uint layer, const highp vec3 position) {
    highp vec2 size = vec2(textureSize(shadowMap, 0));
    highp vec2 st = position.xy * size - 0.5;
    vec4 d;
#if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
    d = textureGather(shadowMap, vec3(position.xy, layer), 0); // 01, 11, 10, 00
#else
    highp ivec3 tc = ivec3(st, layer);
    d[0] = texelFetchOffset(shadowMap, tc, 0, ivec2(0, 1)).r;
    d[1] = texelFetchOffset(shadowMap, tc, 0, ivec2(1, 1)).r;
    d[2] = texelFetchOffset(shadowMap, tc, 0, ivec2(1, 0)).r;
    d[3] = texelFetchOffset(shadowMap, tc, 0, ivec2(0, 0)).r;
#endif
    vec4 pcf = step(0.0, d - position.zzzz);
    highp vec2 grad = fract(st);
    return mix(mix(pcf.w, pcf.z, grad.x), mix(pcf.x, pcf.y, grad.x), grad.y);
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

float ShadowSample_VSM(const mediump sampler2DArray shadowMap,
        const uint layer, const highp vec3 position) {
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
    // note: shadowPosition.z is in the [1, 0] range (reversed Z)
    return ShadowSample_PCF(shadowMap, layer, position);
}

// VSM sampling
float shadow(const mediump sampler2DArray shadowMap,
        const uint layer, const highp vec4 shadowPosition) {
    // note: shadowPosition.z is in linear light-space normalized to [0, 1]
    //  see: ShadowMap::computeVsmLightSpaceMatrix() in ShadowMap.cpp
    //  see: computeLightSpacePosition() in common_shadowing.fs
    highp vec3 position = vec3(shadowPosition.xy * (1.0 / shadowPosition.w), shadowPosition.z);
    return ShadowSample_VSM(shadowMap, layer, position);
}
