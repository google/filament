#if defined(HAS_VSM)
layout(location = 0) out vec4 fragColor;
#endif

//------------------------------------------------------------------------------
// Depth
//------------------------------------------------------------------------------

void main() {
    filament_lodBias = frameUniforms.lodBias;

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
    // Interleaved gradient noise, see dithering.fs
    float noise = fract(52.982919 * fract(dot(vec2(0.06711, 0.00584), gl_FragCoord.xy)));
    if (noise >= alpha) {
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
