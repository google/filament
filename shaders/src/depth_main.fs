#if defined(VARIANT_HAS_VSM)
layout(location = 0) out vec4 fragColor;
#elif defined(VARIANT_HAS_PICKING)
layout(location = 0) out highp uint2 outPicking;
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

#if defined(BLEND_MODE_MASKED) || ((defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)) && defined(MATERIAL_HAS_TRANSPARENT_SHADOW))
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
    outPicking.x = getObjectUniforms().objectId;
    outPicking.y = floatBitsToUint(vertex_position.z / vertex_position.w);
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
