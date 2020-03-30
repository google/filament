//------------------------------------------------------------------------------
// Fog
//------------------------------------------------------------------------------

vec4 fog(vec4 color, vec3 view) {
    if (frameUniforms.fogDensity > 0.0) {
        float A = frameUniforms.fogDensity;
        float B = frameUniforms.fogHeightFalloff;

        float d = length(view);

        float h = view.y;
        // The function below is continuous at h=0, so to avoid a divide-by-zero, we use the
        // constant approximation 'B'. A better approximation would be B * (1 - 0.5 * B * h)
        float fogIntegralFunctionOfDistance = A * ((abs(h) < 0.001) ? B : ((1.0 - exp(-B * h)) / h));
        float fogIntegral = fogIntegralFunctionOfDistance * max(d - frameUniforms.fogStart, 0.0);

        // remap fogIntegral to an opacity between 0 and 1. saturate(fogIntegral) is too harsh.
        float fogOpacity = max(1.0 - exp2(-fogIntegral), 0.0);
        //float fogOpacity = clamp(fogIntegral, 0.0, 1.0);

        // don't go above requested max opacity
        fogOpacity = min(fogOpacity, frameUniforms.fogMaxOpacity);

        // compute fog color
        vec3 fogColor = frameUniforms.fogColor;

        if (frameUniforms.fogColorFromIbl > 0.0) {
            // get fog color from envmap
            float lod = frameUniforms.iblRoughnessOneLevel;
            fogColor *= textureLod(light_iblSpecular, view, lod).rgb * frameUniforms.iblLuminance;
        }

        fogColor *= fogOpacity;
        if (frameUniforms.fogInscatteringSize >= 0.0) {
            // compute a new line-integral for a different start distance
            float inscatteringIntegral = fogIntegralFunctionOfDistance *
                    max(d - frameUniforms.fogInscatteringStart, 0.0);
            float inscatteringOpacity = max(1.0 - exp2(-inscatteringIntegral), 0.0);
            //float inscatteringOpacity = clamp(inscatteringIntegral, 0.0, 1.0);

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
