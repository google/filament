#if defined(VARIANT_HAS_VSM)
layout(location = 0) out highp vec4 fragColor;
#elif defined(VARIANT_HAS_PICKING)
#   if __VERSION__ == 100
highp vec4 outPicking;
#   else
#       if MATERIAL_FEATURE_LEVEL == 0
layout(location = 0) out highp vec4 outPicking;
#       else
layout(location = 0) out highp uvec2 outPicking;
#       endif
#   endif
#else
// not color output
#endif

//------------------------------------------------------------------------------
// Depth
//
// note: VARIANT_HAS_VSM and VARIANT_HAS_PICKING are mutually exclusive
//------------------------------------------------------------------------------

highp vec4 computeDepthMomentsVSM(const highp float depth);

void main() {
    filament_lodBias = frameUniforms.lodBias;

    initObjectUniforms();

#if defined(MATERIAL_HAS_CUSTOM_DEPTH) || defined(BLEND_MODE_MASKED) || ((defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)) && defined(MATERIAL_HAS_TRANSPARENT_SHADOW))
    MaterialInputs inputs;
    initMaterial(inputs);
    material(inputs);

    float alpha = inputs.baseColor.a;
#if defined(BLEND_MODE_MASKED)
    if (alpha < getMaskThreshold()) {
        discard;
    }
#endif

#if defined(MATERIAL_HAS_TRANSPARENT_SHADOW)
    // Interleaved gradient noise, see inline_dithering.fs
    float noise = interleavedGradientNoise(gl_FragCoord.xy);
    if (noise >= alpha) {
        discard;
    }
#endif
#endif

#if defined(VARIANT_HAS_VSM)
    // interpolated depth is stored in vertex_worldPosition.w (see surface_main.vs)
    // we always compute the "negative" side of ELVSM because the cost is small, and this allows
    // EVSM/ELVSM choice to be done on the CPU side more easily.
    highp float depth = vertex_worldPosition.w;
    fragColor = computeDepthMomentsVSM(depth);
#elif defined(VARIANT_HAS_PICKING)
#if FILAMENT_EFFECTIVE_VERSION == 100
    outPicking.a = mod(float(object_uniforms_objectId / 65536), 256.0) / 255.0;
    outPicking.b = mod(float(object_uniforms_objectId /   256), 256.0) / 255.0;
    outPicking.g = mod(float(object_uniforms_objectId)        , 256.0) / 255.0;
    outPicking.r = vertex_position.z / vertex_position.w;
#else
    outPicking.x = uint(object_uniforms_objectId);
    outPicking.y = floatBitsToUint(vertex_position.z / vertex_position.w);
#endif
#if __VERSION__ == 100
    gl_FragData[0] = outPicking;
#endif
#else
    // that's it
#endif
}

#if MATERIAL_FEATURE_LEVEL > 0

highp vec4 computeDepthMomentsVSM(const highp float depth) {
    // See GPU Gems 3
    // https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps
    // computes the first two moments
    float c = frameUniforms.vsmExponent;
    highp float MAX_MOMENT = frameUniforms.vsmMaxMoment;

    // wrap depth for EVSM
    highp float z = exp(c * depth);

    // compute EVSM moments
    highp vec2 m1 = vec2(z, -1.0 / z);
    highp vec2 m2 = m1 * m1;

    // compute analytic variance (2nd moment), taking into account the change in depth accross the texel
    highp float dzdx = dFdx(depth);
    highp float dzdy = dFdy(depth);
    highp float linearVariance = 0.25 * (dzdx * dzdx + dzdy * dzdy);
    highp vec2 analyticVariance = c * c * m2 * linearVariance;
    m2 = min(m2 + analyticVariance, MAX_MOMENT);

    return vec4(m1.x, m2.x, m1.y, m2.y);
}

#endif
