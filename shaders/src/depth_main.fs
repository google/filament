//------------------------------------------------------------------------------
// Depth
//------------------------------------------------------------------------------

void main() {
#if defined(BLEND_MODE_MASKED)
    MaterialInputs inputs;
    initMaterial(inputs);
    material(inputs);

    float alpha = inputs.baseColor.a;
    if (alpha < getMaskThreshold()) {
        discard;
    }
#endif

#if defined(HAS_VSM)
    // For VSM, we use the linear light space Z coordinate as the depth metric, which works for both
    // directional and spot lights.
    // We negate it, because we're using a right-handed coordinate system (-Z points forward).
    highp float depth = -mulMat4x4Float3(frameUniforms.viewFromWorldMatrix, vertex_worldPosition).z;

    // Scale by cameraFar to help prevent a floating point overflow below when squaring the depth.
    depth /= abs(frameUniforms.cameraFar);

    highp float dx = dFdx(depth);
    highp float dy = dFdy(depth);

    // Output the first and second depth moments.
    // The first moment is mean depth.
    // The second moment is mean depth squared.
    // These values are retrieved when sampling the shadow map to compute variance.
    highp float bias = 0.25 * (dx * dx + dy * dy);
    fragColor = vec4(depth, depth * depth + bias, 0.0, 0.0);
#endif
}
