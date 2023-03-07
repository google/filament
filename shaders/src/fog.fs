//------------------------------------------------------------------------------
// Fog
// see: Real-time Atmospheric Effects in Games (Carsten Wenzel)
//------------------------------------------------------------------------------

vec4 fog(vec4 color, highp vec3 view) {
    // .x = density
    // .y = -fallof*(y-height)
    // .z = density * exp(-fallof*(y-height))
    highp vec3 density = frameUniforms.fogDensity;
    float falloff = frameUniforms.fogHeightFalloff;

    // Compute the integral of the fog density at a distance of 1m at a given height.
    highp float fogDensityIntegralAtOneMeter = density.z;
    highp float h = falloff * view.y;
    if (abs(h) > 0.01) {
        // The function below is continuous at h=0, so to avoid a divide-by-zero, we just clamp h
        fogDensityIntegralAtOneMeter = (fogDensityIntegralAtOneMeter - density.x * exp(density.y - h)) / h;
    }

    // Compute the integral of the fog density at a given height from fogStart to the fragment
    highp float d = length(view);
    highp float fogDensityIntegral =
            fogDensityIntegralAtOneMeter * max(d - frameUniforms.fogStart, 0.0);

    // Compute the transmittance using the Beer-Lambert Law
    float fogTransmittance = exp(-fogDensityIntegral);

    // Compute the opacity from the transmittance
    float fogOpacity = min(1.0 - fogTransmittance, frameUniforms.fogMaxOpacity);

    // compute fog color
    vec3 fogColor = frameUniforms.fogColor;

    if (frameUniforms.fogColorFromIbl > 0.0) {
        // get fog color from envmap
        float lod = frameUniforms.iblRoughnessOneLevel;
        fogColor *= textureLod(light_iblSpecular, view, lod).rgb * frameUniforms.iblLuminance;
    }

    fogColor *= fogOpacity;

    if (frameUniforms.fogInscatteringSize > 0.0) {
        // compute a new line-integral for a different start distance
        highp float inscatteringDensityIntegral =
                fogDensityIntegralAtOneMeter * max(d - frameUniforms.fogInscatteringStart, 0.0);

        // Compute the transmittance using the Beer-Lambert Law
        float inscatteringDensity = exp(-inscatteringDensityIntegral);

        // Compute the opacity from the transmittance
        float inscatteringOpacity = 1.0 - inscatteringDensity;

        // Add sun colored fog when looking towards the sun
        vec3 sunColor = frameUniforms.lightColorIntensity.rgb * frameUniforms.lightColorIntensity.w;

        float sunAmount = max(dot(view, frameUniforms.lightDirection) / d, 0.0); // between 0 and 1
        float sunInscattering = pow(sunAmount, frameUniforms.fogInscatteringSize);

        fogColor += sunColor * (sunInscattering * inscatteringOpacity);
    }

#if   defined(BLEND_MODE_OPAQUE)
    // nothing to do here
#elif defined(BLEND_MODE_TRANSPARENT)
    fogColor *= color.a;
#elif defined(BLEND_MODE_ADD)
    fogColor = vec3(0.0);
#elif defined(BLEND_MODE_MASKED)
    // nothing to do here
#elif defined(BLEND_MODE_MULTIPLY)
    // FIXME: unclear what to do here
#elif defined(BLEND_MODE_SCREEN)
    // FIXME: unclear what to do here
#endif

    color.rgb = color.rgb * (1.0 - fogOpacity) + fogColor;

    return color;
}
