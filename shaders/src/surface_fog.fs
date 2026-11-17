//------------------------------------------------------------------------------
// Fog
// see: Real-time Atmospheric Effects in Games (Carsten Wenzel)
//------------------------------------------------------------------------------

#if MATERIAL_FEATURE_LEVEL > 0

vec4 fog(highp vec3 view, const mediump samplerCube fogColorTexture) {
    // note: d can be +inf with the skybox
    highp float d = length(view);

    // early exit for object "in front" of the fog
    if (d < frameUniforms.fogStart) {
        return vec4(0);
    }

    // fogCutOffDistance is set to +inf to disable the cutoff distance
    if (d > frameUniforms.fogCutOffDistance) {
        return vec4(0);
    }

    // .z = density * exp(-fallof*(y-height))
    highp float density = frameUniforms.fogDensity.z;

    // height falloff [1/m]
    highp float falloff = frameUniforms.fogHeightFalloff;

    // Compute the fog's optical path (unitless) at a distance of 1m at a given height.
    highp float fogOpticalPathAtOneMeter = density;
    highp float fh = falloff * view.y;
    if (abs(fh) > 0.00125) {
        // The function below is continuous at fh=0, so to avoid a divide-by-zero, we just clamp fh
        fogOpticalPathAtOneMeter = density * (1.0 - exp(-fh)) / fh;
    }

    // Compute the integral of the fog density at a given height from fogStart to the fragment
    highp float fogOpticalPath = fogOpticalPathAtOneMeter * (d - frameUniforms.fogStart);

    // Compute the transmittance [0,1] using the Beer-Lambert Law
    float fogTransmittance = exp(-fogOpticalPath);

    // Compute the opacity from the transmittance
    float fogOpacity = min(1.0 - fogTransmittance, frameUniforms.fogMaxOpacity);

    // compute fog color
    vec3 fogColor = frameUniforms.fogColor;

#if MATERIAL_FEATURE_LEVEL > 0
    if (frameUniforms.fogColorFromIbl > 0.0) {
        float normalizedDepth = d * frameUniforms.fogOneOverFarMinusNear - frameUniforms.fogNearOverFarMinusNear;
        lowp vec2 minMaxMip = unpackHalf2x16(frameUniforms.fogMinMaxMip);
        lowp float lod = mix(minMaxMip.y, minMaxMip.x, saturate(normalizedDepth));

        // when sampling the IBL we need to take into account the IBL transform. We know it's a
        // rigid transform, so we can left-multiply by view instead of transposing or inverting.
        highp vec3 iblView = view * mat3(frameUniforms.userWorldFromWorldMatrix);
        fogColor *= textureLod(fogColorTexture, iblView, lod).rgb;
    }
#endif

    fogColor *= frameUniforms.iblLuminance * fogOpacity;

    if (frameUniforms.fogInscatteringSize > 0.0) {
        // compute a new line-integral for a different start distance
        highp float sunOpticalPath =
                fogOpticalPathAtOneMeter * max(d - frameUniforms.fogInscatteringStart, 0.0);

        // Compute the transmittance using the Beer-Lambert Law
        float sunTransmittance = exp(-sunOpticalPath);

        // Add sun colored fog when looking towards the sun
        vec3 sunColor = frameUniforms.lightColorIntensity.rgb * frameUniforms.lightColorIntensity.w;

        float sunAmount = d > 0.0 ?
                max(dot(view, frameUniforms.lightDirection) / d, 0.0) : 0.0;
        float sunInscattering = pow(sunAmount, frameUniforms.fogInscatteringSize);

        fogColor += sunColor * (sunInscattering * (1.0 - sunTransmittance));
    }

    return vec4(fogColor, fogOpacity);
}

#endif  // MATERIAL_FEATURE_LEVEL > 0

// A linear approximation of the fog function
vec4 fogLinear(highp vec3 view
#if MATERIAL_FEATURE_LEVEL > 0
    , const mediump samplerCube fogColorTexture
#endif
)
{
    // note: d can be +inf with the skybox
    highp float d = length(view);

    // early exit for object "in front" of the fog
    if (d < frameUniforms.fogStart) {
        return vec4(0);
    }

    // fogCutOffDistance is set to +inf to disable the cutoff distance
    if (d > frameUniforms.fogCutOffDistance) {
        return vec4(0);
    }

    // compute fog color
    highp float A = frameUniforms.fogLinearParams.x;
    highp float B = frameUniforms.fogLinearParams.y;
    float fogOpacity = saturate(A * d + B);

    vec3 fogColor = frameUniforms.fogColor;

#if MATERIAL_FEATURE_LEVEL > 0
    if (frameUniforms.fogColorFromIbl > 0.0) {
        lowp vec2 minMaxMip = unpackHalf2x16(frameUniforms.fogMinMaxMip);
        // when sampling the IBL we need to take into account the IBL transform. We know it's a
        // rigid transform, so we can left-multiply by view instead of transposing or inverting.
        highp vec3 iblView = view * mat3(frameUniforms.userWorldFromWorldMatrix);
        fogColor *= textureLod(fogColorTexture, iblView, minMaxMip.x).rgb;
    }
#endif

    fogColor *= frameUniforms.iblLuminance * fogOpacity;

    return vec4(fogColor, fogOpacity);
}
