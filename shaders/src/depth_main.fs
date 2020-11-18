#if defined(HAS_VSM)
layout(location = 0) out vec4 fragColor;
#endif

//------------------------------------------------------------------------------
// Depth
//------------------------------------------------------------------------------

#if defined(HAS_TRANSPARENT_SHADOW)
float bayer2x2(highp vec2 p) {
    return mod(2.0 * p.y + p.x + 1.0, 4.0);
}

float bayer8x8(highp vec2 p) {
    vec2 p1 = mod(p, 2.0);
    vec2 p2 = floor(0.5  * mod(p, 4.0));
    vec2 p4 = floor(0.25 * mod(p, 8.0));
    return 4.0 * (4.0 * bayer2x2(p1) + bayer2x2(p2)) + bayer2x2(p4);
}
#endif

void main() {
#if defined(BLEND_MODE_MASKED) || (defined(BLEND_MODE_TRANSPARENT) && defined(HAS_TRANSPARENT_SHADOW))
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
    if ((bayer8x8(floor(mod(gl_FragCoord.xy, 8.0)))) / 64.0 >= alpha) {
        discard;
    }
#endif
#endif

#if defined(HAS_VSM)
    // For VSM, we use the linear light space Z coordinate as the depth metric, which works for both
    // directional and spot lights.
    // The value is guaranteed to be between [0, -zfar] by construction of viewFromWorldMatrix,
    // (see ShadowMap.cpp).
    highp float z = (frameUniforms.viewFromWorldMatrix * vec4(vertex_worldPosition, 1.0)).z;

    // rescale the depth between [0, 1]
    highp float depth = -z / abs(frameUniforms.cameraFar);

    // We use positive only EVSM which helps a lot with light bleeding.
    depth = depth * 2.0 - 1.0;
    depth = exp(frameUniforms.vsmExponent * depth);

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
#endif
}
