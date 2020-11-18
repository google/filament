#if defined(HAS_VSM)
layout(location = 0) out vec4 fragColor;
#endif

//------------------------------------------------------------------------------
// Depth
//------------------------------------------------------------------------------

#if defined(HAS_TRANSPARENT_SHADOW)
const float bayer8x8[64] = float[64](
    0.0000, 0.5000, 0.1250, 0.6250, 0.03125, 0.53125, 0.15625, 0.65625,
    0.7500, 0.2500, 0.8750, 0.3750, 0.78125, 0.28125, 0.90625, 0.40625,
    0.1875, 0.6875, 0.0625, 0.5625, 0.21875, 0.71875, 0.09375, 0.59375,
    0.9375, 0.4375, 0.8125, 0.3125, 0.96875, 0.46875, 0.84375, 0.34375,
    0.0469, 0.5469, 0.1719, 0.6719, 0.01563, 0.51563, 0.14063, 0.64063,
    0.7969, 0.2969, 0.9219, 0.4219, 0.76563, 0.26563, 0.89063, 0.39063,
    0.2344, 0.7344, 0.1094, 0.6094, 0.20313, 0.70313, 0.07813, 0.57813,
    0.9844, 0.4844, 0.8594, 0.3594, 0.95313, 0.45313, 0.82813, 0.32813
);
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
    vec2 coords = mod(gl_FragCoord.xy, 8.0);
    if (bayer8x8[int(coords.y * 8.0 + coords.x)] >= alpha) {
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
