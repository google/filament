#if defined(VARIANT_HAS_VSM)
layout(location = 0) out highp vec4 fragColor;
#elif defined(VARIANT_HAS_PICKING)
#   if __VERSION__ == 100
highp vec4 outPicking;
#   else
#       if MATERIAL_FEATURE_LEVEL == 0
layout(location = 0) out highp vec4 outPicking;
#       else
layout(location = 0) out highp vec2 outPicking;
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

highp vec2 computeDepthMomentsVSM(const highp float depth);

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
    // Interleaved gradient noise, see dithering.fs
    float noise = interleavedGradientNoise(gl_FragCoord.xy);
    if (noise >= alpha) {
        discard;
    }
#endif
#endif

#if defined(VARIANT_HAS_VSM)
    // interpolated depth is stored in vertex_worldPosition.w (see main.vs)
    // we always compute the "negative" side of ELVSM because the cost is small, and this allows
    // EVSM/ELVSM choice to be done on the CPU side more easily.
    highp float depth = vertex_worldPosition.w;
    depth = exp(frameUniforms.vsmExponent * depth);
    fragColor.xy = computeDepthMomentsVSM(depth);
    fragColor.zw = computeDepthMomentsVSM(-1.0 / depth); // requires at least RGBA16F
#elif defined(VARIANT_HAS_PICKING)
#if FILAMENT_EFFECTIVE_VERSION == 100
    outPicking.a = mod(float(object_uniforms_objectId / 65536), 256.0) / 255.0;
    outPicking.b = mod(float(object_uniforms_objectId /   256), 256.0) / 255.0;
    outPicking.g = mod(float(object_uniforms_objectId)        , 256.0) / 255.0;
    outPicking.r = vertex_position.z / vertex_position.w;
#else
    outPicking.x = intBitsToFloat(object_uniforms_objectId);
    outPicking.y = vertex_position.z / vertex_position.w;
#endif
#if __VERSION__ == 100
    gl_FragData[0] = outPicking;
#endif
#else
    // that's it
#endif
}

highp vec2 computeDepthMomentsVSM(const highp float depth) {
    // computes the moments
    // See GPU Gems 3
    // https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps
    highp vec2 moments;

    // the first moment is just the depth (average)
    moments.x = depth;

    // compute the 2nd moment over the pixel extents.
    moments.y = depth * depth;

    // the local linear approximation is not correct with a warped depth
    //highp float dx = dFdx(depth);
    //highp float dy = dFdy(depth);
    //moments.y += 0.25 * (dx * dx + dy * dy);

    return moments;
}
