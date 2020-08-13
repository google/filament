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
    highp float depth = length(frameUniforms.cameraPosition.xyz - vertex_worldPosition);

    highp float dx = dFdx(depth);
    highp float dy = dFdy(depth);

    fragColor = vec4(depth, depth * depth + 0.25 * (dx * dx + dy * dy), 0.0, 0.0);
#endif
}
