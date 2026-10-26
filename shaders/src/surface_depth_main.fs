#if defined(VARIANT_HAS_MNT)
layout(location = 0) out highp vec4 fragColor;
#elif defined(VARIANT_HAS_PICKING)
#   if MATERIAL_FEATURE_LEVEL == 0
layout(location = 0) out highp vec4 outPicking;
#   else
layout(location = 0) out highp uvec2 outPicking;
#   endif
#elif defined(VARIANT_HAS_MOT)
layout(location = 0) out highp vec2 outMotionVector;
#else
// not color output
#endif

//------------------------------------------------------------------------------
// Depth
//
// note: VARIANT_HAS_MNT, VARIANT_HAS_PICKING and VARIANT_HAS_MOT are mutually exclusive
//------------------------------------------------------------------------------

highp vec4 computeDepthMomentsVSM(const highp float depth);

void main() {
    filament_lodBias = frameUniforms.lodBias;
#if defined(FILAMENT_HAS_FEATURE_INSTANCING)
    logical_instance_index = instance_index;
#endif
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

#if defined(VARIANT_HAS_MNT)
    // interpolated depth is stored in vertex_worldPosition.w (see surface_main.vs)
    // we always compute the "negative" side of ELVSM because the cost is small, and this allows
    // EVSM/ELVSM choice to be done on the CPU side more easily.
    highp float depth = vertex_worldPosition.w;
    fragColor = computeDepthMomentsVSM(depth);
#elif defined(VARIANT_HAS_PICKING)
#if MATERIAL_FEATURE_LEVEL == 0
    outPicking.a = mod(float(object_uniforms_objectId / 65536), 256.0) / 255.0;
    outPicking.b = mod(float(object_uniforms_objectId /   256), 256.0) / 255.0;
    outPicking.g = mod(float(object_uniforms_objectId)        , 256.0) / 255.0;
    outPicking.r = vertex_position.z / vertex_position.w;
#else
    outPicking.x = uint(object_uniforms_objectId);
    outPicking.y = floatBitsToUint(vertex_position.z / vertex_position.w);
#endif

#elif defined(VARIANT_HAS_MOT)
    // Object motion in the previous frame's render-target UV space: both positions are projected
    // with the previous (unjittered) camera. A position behind that camera (w <= 0) has no usable
    // projection, which is reported as an invalid motion vector (|v| >= 1). The clamp keeps the
    // RG16F output finite.
    const highp float kMinW = 1.0e-6;
    highp vec4 c = vertex_motionCurrClip;
    highp vec4 p = vertex_motionPrevClip;
    if (c == p) {
        outMotionVector = vec2(0.0);
    } else if (min(c.w, p.w) > kMinW) {
        highp vec2 uvCurr = uvToRenderTargetUV(c.xy * (0.5 / c.w) + 0.5);
        highp vec2 uvPrev = uvToRenderTargetUV(p.xy * (0.5 / p.w) + 0.5);
        outMotionVector = clamp(uvCurr - uvPrev, vec2(-2.0), vec2(2.0));
    } else {
        outMotionVector = vec2(2.0);
    }
#else
    // that's it
#endif
}

#if MATERIAL_FEATURE_LEVEL > 0

highp vec4 computeDepthMomentsVSM(const highp float depth) {
    // depth mush be interpolated with `centroid`, otherwise when MSAA is used, it could be
    // interpolated outside ot the triangle (i.e. the center of the pixel could be outside),
    // and because we're using a large exp() factor, that depth value could dominate the
    // surrounding pixels when we generate the mipmap chain. It doesn't work to clamp
    // depth to [-1, 1] because +/-1 could still be too far from the real value of the
    // MSAA sample. The downside is that `centroid` will damage the derivatives below,
    // however, it seems to be acceptable.

    highp float MAX_MOMENT = frameUniforms.vsmMaxMoment;
    float c = frameUniforms.vsmExponent;

    // wrap depth for EVSM
    highp float z = exp(c * depth);

    // compute EVSM moments
    highp vec2 m1 = vec2(z, -1.0 / z);
    highp vec2 m2 = m1 * m1;

    // Compute analytic variance (2nd moment), taking into account the change in depth accross the texel.
    // This should help with shadow acne and peter panning.
    highp float dzdx = dFdx(depth);
    highp float dzdy = dFdy(depth);
    highp float linearVariance = 0.25 * (dzdx * dzdx + dzdy * dzdy);
    highp vec2 analyticVariance = c * c * m2 * linearVariance;
    m2 = min(m2 + analyticVariance, MAX_MOMENT);

    return vec4(m1.x, m2.x, m1.y, m2.y);
}

#endif
