#if defined(HAS_VSM)
layout(location = 0) out vec4 fragColor;
#elif defined(HAS_PICKING)
layout(location = 0) out highp uint2 outPicking;
#else
// not color output
#endif

//------------------------------------------------------------------------------
// Depth
//
// note: HAS_VSM and HAS_PICKING are mutually exclusive
//------------------------------------------------------------------------------

void main() {
    filament_lodBias = frameUniforms.lodBias;

#if defined(BLEND_MODE_MASKED) || ((defined(BLEND_MODE_TRANSPARENT) || defined(BLEND_MODE_FADE)) && defined(HAS_TRANSPARENT_SHADOW))
    MaterialInputs inputs;
    initMaterial(inputs);
    material(inputs);

    float alpha = inputs.baseColor.a;
#if defined(BLEND_MODE_MASKED)
    if (alpha < getMaskThreshold()) {
        discard;
    }
#endif

#if defined(HAS_TRANSPARENT_SHADOW)
    // Interleaved gradient noise, see dithering.fs
    float noise = fract(52.982919 * fract(dot(vec2(0.06711, 0.00584), gl_FragCoord.xy)));
    if (noise >= alpha) {
        discard;
    }
#endif
#endif

#if defined(HAS_VSM)
    // interpolated depth is stored in vertex_worldPosition.w (see main.vs)
    highp float depth = exp(vertex_worldPosition.w);

    // computes the moments
    // See GPU Gems 3
    // https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps
    highp vec2 moments;

    // the first moment is just the depth (average)
    moments.x = depth;

    // compute the 2nd moment over the pixel extents.
    highp float dx = dFdx(depth);
    highp float dy = dFdy(depth);
    moments.y = depth * depth + 0.25 * (dx * dx + dy * dy);

    fragColor = vec4(moments, 0.0, 0.0);
#elif defined(HAS_PICKING)
    outPicking.x = objectUniforms.objectId;
    outPicking.y = floatBitsToUint(vertex_position.z / vertex_position.w);
#else
    // that's it
#endif
}
