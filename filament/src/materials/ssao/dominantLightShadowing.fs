/*
 * Largely based on "Dominant Light Shadowing"
 * "Lighting Technology of The Last of Us Part II" by Hawar Doghramachi, Naughty Dog, LLC
 */

#include "ssaoUtils.fs"

struct ConeTraceSetup {
    // runtime parameters
    highp vec2 ssStartPos;
    highp vec3 vsStartPos;
    vec3 vsNormal;
    highp mat4 screenFromViewMatrix;
    vec2 jitterOffset;          // (x = direction offset, y = step offset)
    highp float depthParams;
    vec3 vsConeDirection;

    // artistic/quality parameters
    vec4 coneTraceParams;       // { tan(angle), sin(angle), start trace distance, inverse max contact distance }
    float intensity;
    float invZoom;
    float depthBias;
    float slopeScaledDepthBias;
    uint sampleCount;
};

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
    highp float ssStartInvW = 1.0 / (setup.screenFromViewMatrix * vec4(vsStartPos, 1.0)).w;

    // end position of cone trace
    highp vec3 vsEndPos = setup.vsConeDirection + vsStartPos;
    highp vec4 ssEndPos = setup.screenFromViewMatrix * vec4(vsEndPos, 1.0);
    highp float ssEndInvW = 1.0 / ssEndPos.w;
    ssEndPos.xy *= ssEndInvW;

    // cone trace direction in screen-space
    float ssConeLength = length(ssEndPos.xy - ssStartPos);
    float ssInvConeLength = 1.0 / ssConeLength;
    vec2 ssConeDirection = (ssEndPos.xy - ssStartPos) * ssInvConeLength;

    // direction perpendicular to cone trace direction
    vec2 perpConeDir = vec2(ssConeDirection.y, -ssConeDirection.x);

    // avoid self-occlusion and reduce banding artifacts by normal variation
    vec3 vsViewVector = normalize(vsStartPos);
    float minTraceDistance = (1.0 - abs(dot(setup.vsNormal, vsViewVector))) * 0.005;

    // init trace distance and sample radius
    highp float invLinearDepth = 1.0 / -setup.vsStartPos.z;
    highp float ssTracedDistance = max(setup.coneTraceParams.z, minTraceDistance) * invLinearDepth;

    float ssSampleRadius = setup.coneTraceParams.y * ssTracedDistance;
    float ssEndRadius    = setup.coneTraceParams.y * ssConeLength;
    float vsEndRadius    = ssEndRadius * setup.invZoom * invLinearDepth * ssEndPos.w;

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

        // sample depth buffer
        highp vec2 ssSamplePos = perpConeDir * ssJitteredSampleRadius + ssConeDirection * ssJitteredTracedDistance + ssStartPos;
        float vsSampleDepthLinear = -sampleDepthLinear(depthTexture, ssSamplePos, 0.0, setup.depthParams);

        // calculate depth of cone center
        float ratio = ssJitteredTracedDistance * ssInvConeLength;
        float vsConeAxisDepth = 1.0 / mix(ssStartInvW, ssEndInvW, ratio);

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
