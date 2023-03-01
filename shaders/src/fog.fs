//------------------------------------------------------------------------------
// Fog
// see: Real-time Atmospheric Effects in Games (Carsten Wenzel)
//------------------------------------------------------------------------------

vec4 fog(vec4 color, highp vec3 view) {
    if (frameUniforms.fogDensity > 0.0) {
        // density * exp(-falloff * height)
        float densityAtViewerHeight = frameUniforms.fogDensity;
        float falloff = frameUniforms.fogHeightFalloff;

        highp float fogIntegralFunctionOfDistance = densityAtViewerHeight;
        highp float h = falloff * view.y;
        if (abs(h) > 0.01) {
            // The function below is continuous at h=0, so to avoid a divide-by-zero, we just clamp h
            fogIntegralFunctionOfDistance *= (1.0 - exp(-h)) / h;
        }
        highp float d = length(view);
        highp float fogIntegral = fogIntegralFunctionOfDistance * max(d - frameUniforms.fogStart, 0.0);
        float fogDensity = exp(-fogIntegral);

        // don't go above requested max opacity
        float fogOpacity = min(1.0 - fogDensity, frameUniforms.fogMaxOpacity);

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
            highp float inscatteringIntegral = fogIntegralFunctionOfDistance *
                    max(d - frameUniforms.fogInscatteringStart, 0.0);
            float inscatteringDensity = exp(-inscatteringIntegral);
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
    }
    return color;
}
