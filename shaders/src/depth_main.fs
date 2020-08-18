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
    // Since we're rendering from the perspective of the light, frameUniforms.cameraPosition is the
    // light position, in world space.
    // We need a linear depth representation, so we can't simply use gl_FragDepth here, which won't
    // be linear for spot shadows or when using LiSPM.
    highp float depth = length(frameUniforms.cameraPosition.xyz - vertex_worldPosition);

    highp float dx = dFdx(depth);
    highp float dy = dFdy(depth);

    // Output the first and second depth moments.
    // The first moment is mean depth.
    // The second moment is depth squared.
    // These values are retrieved when sampling the shadow map to compute variance.
    highp float bias = 0.25 * (dx * dx + dy * dy);
    fragColor = vec4(depth, depth * depth + bias, 0.0, 0.0);
#endif
}
