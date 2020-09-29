/*
 * Largely based on "Dominant Light Shadowing"
 * "Lighting Technology of The Last of Us Part II" by Hawar Doghramachi, Naughty Dog, LLC
 */

#include "ssaoUtils.fs"

const float kSSCTLog2LodRate = 4.0;

struct ConeTraceSetup {
    // fragment info
    highp vec2 ssStartPos;
    highp vec3 vsStartPos;
    vec3 vsNormal;

    // light (cone) info
    vec3 vsConeDirection;
    vec4 coneTraceParams;       // { tan(angle), sin(angle), start trace distance, inverse max contact distance }
    vec2 jitterOffset;          // (x = direction offset, y = step offset)

    // scene infos
    highp mat4 screenFromViewMatrix;
    highp float depthParams;
    float projectionScale;
    vec4 resolution;
    float maxLevel;

    // artistic/quality parameters
    float intensity;
    float depthBias;
    float slopeScaledDepthBias;
    uint sampleCount;
};

highp float getWFromProjectionMatrix(const mat4 p, const vec3 v) {
    // this assumes a projection matrix perspective or ortho
    // returns  (p * v).w
    return p[2][3] * v.z + p[3][3];
}

float coneTraceOcclusion(in ConeTraceSetup setup, const sampler2D depthTexture) {
    // skip fragments that are back-facing trace direction
    // (avoid overshadowing of translucent surfaces)
    float NoL = dot(setup.vsNormal, setup.vsConeDirection);
    if (NoL < 0.0) {
        return 0.0;
    }

    // start position of cone trace
    highp vec2 ssStartPos = setup.ssStartPos;
    highp vec3 vsStartPos = setup.vsStartPos;
    highp float ssStartPosInvW = 1.0 / getWFromProjectionMatrix(setup.screenFromViewMatrix, vsStartPos);

    // end position of cone trace
    highp vec3 vsEndPos = setup.vsConeDirection + vsStartPos;
    highp float ssEndPosW = getWFromProjectionMatrix(setup.screenFromViewMatrix, vsEndPos);
    highp float ssEndPosInvW = 1.0 / ssEndPosW;
    highp vec2 ssEndPos = (setup.screenFromViewMatrix * vec4(vsEndPos, 1.0)).xy * ssEndPosInvW;

    // cone trace direction in screen-space
    float ssConeLength = length(ssEndPos - ssStartPos);
    float ssInvConeLength = 1.0 / ssConeLength;
    vec2 ssConeDirection = (ssEndPos - ssStartPos) * ssInvConeLength;

    // direction perpendicular to cone trace direction
    vec2 perpConeDir = vec2(ssConeDirection.y, -ssConeDirection.x);

    // avoid self-occlusion and reduce banding artifacts by normal variation
    vec3 vsViewVector = normalize(vsStartPos);
    float minTraceDistance = (1.0 - abs(dot(setup.vsNormal, vsViewVector))) * setup.projectionScale * 0.005;

    // init trace distance and sample radius
    highp float invLinearDepth = 1.0 / -setup.vsStartPos.z;
    highp float ssTracedDistance = max(setup.coneTraceParams.z, minTraceDistance) * invLinearDepth;

    float ssSampleRadius = setup.coneTraceParams.y * ssTracedDistance;
    float ssEndRadius    = setup.coneTraceParams.y * ssConeLength;
    float vsEndRadius    = ssEndRadius * (1.0 / setup.projectionScale) * invLinearDepth * ssEndPosW;

    // calculate depth bias
    float vsDepthBias = saturate(1.0 - NoL) * setup.slopeScaledDepthBias + setup.depthBias;

    float occlusion = 0.0;
    for (uint i = 0u; i < setup.sampleCount; i++) {
        // step along cone in screen space
        float ssNextSampleRadius = ssSampleRadius * (ssSampleRadius + ssTracedDistance) / (ssTracedDistance - ssSampleRadius);
        float ssStepDistance = ssSampleRadius + ssNextSampleRadius;
        ssSampleRadius = ssNextSampleRadius;

        // apply jitter offset
        float ssJitterStepDistance = ssStepDistance * setup.jitterOffset.y;
        float ssJitteredTracedDistance = ssTracedDistance + ssJitterStepDistance;
        float ssJitteredSampleRadius = setup.jitterOffset.x * setup.coneTraceParams.x * ssJitteredTracedDistance;
        ssTracedDistance += ssStepDistance;

        // sample depth buffer, using lower LOD as the radius (i.e. distance from origin) grows
        highp vec2 ssSamplePos = perpConeDir * ssJitteredSampleRadius + ssConeDirection * ssJitteredTracedDistance + ssStartPos;
        float level = clamp(floor(log2(ssJitteredTracedDistance)) - kSSCTLog2LodRate, 0.0, setup.maxLevel);
        float vsSampleDepthLinear = -sampleDepthLinear(depthTexture, ssSamplePos * setup.resolution.zw, level, setup.depthParams);

        // calculate depth of cone center
        float ratio = ssJitteredTracedDistance * ssInvConeLength;
        float vsConeAxisDepth = 1.0 / mix(ssStartPosInvW, ssEndPosInvW, ratio);

        // calculate depth range of cone slice
        float vsConeRadius = (ratio * vsEndRadius) * vsConeAxisDepth;
        float vsJitteredSampleRadius = vsConeRadius * setup.jitterOffset.x;
        float vsSliceHalfRange = sqrt(vsConeRadius * vsConeRadius - vsJitteredSampleRadius * vsJitteredSampleRadius);
        float vsSampleDepthMax = vsConeAxisDepth + vsSliceHalfRange;

        // calculate overlap of depth buffer height-field with trace cone
        float vsDepthDifference = vsSampleDepthMax - vsSampleDepthLinear;
        float overlap = saturate((vsDepthDifference - vsDepthBias) / (vsSliceHalfRange * 2.0));

        // attenuate by distance to avoid false occlusion
        float attenuation = saturate(1.0 - (vsDepthDifference * setup.coneTraceParams.w));
        occlusion = max(occlusion, overlap * attenuation);

        if (occlusion >= 1.0) {  // note: this can't get > 1.0 by construction
            // fully occluded, early exit
            break;
        }
    }
    return occlusion * setup.intensity;
}