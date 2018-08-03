//------------------------------------------------------------------------------
// Directional light evaluation
//------------------------------------------------------------------------------

vec3 sampleSunAreaLight(const vec3 lightDirection) {
#if !defined(TARGET_MOBILE)
    if (frameUniforms.sun.w >= 0.0) {
        // simulate sun as disc area light
        float LoR = dot(lightDirection, shading_reflected);
        float d = frameUniforms.sun.x;
        HIGHP vec3 s = shading_reflected - LoR * lightDirection;
        return LoR < d ?
                normalize(lightDirection * d + normalize(s) * frameUniforms.sun.y) : shading_reflected;
    }
#endif
    return lightDirection;
}

Light getDirectionalLight() {
    Light light;
    // note: lightColorIntensity.w is always premultiplied by the exposure
    light.colorIntensity = frameUniforms.lightColorIntensity;
    light.l = sampleSunAreaLight(frameUniforms.lightDirection);
    light.attenuation = 1.0;
    return light;
}

void evaluateDirectionalLight(const PixelParams pixel, inout vec3 color) {
    Light light = getDirectionalLight();
    float visibility = 1.0;
#ifdef HAS_SHADOWING
    // TODO: don't compute when NoL < 0.0
    visibility = shadow(light_shadowMap, getLightSpacePosition());
#endif
    // TODO: skip when visibility == 0.0 (shading model dependent)
    color.rgb += surfaceShading(pixel, light, visibility);
}
