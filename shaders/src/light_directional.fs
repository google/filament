//------------------------------------------------------------------------------
// Directional light evaluation
//------------------------------------------------------------------------------

#if !defined(TARGET_MOBILE)
#define SUN_AS_AREA_LIGHT
#endif

vec3 sampleSunAreaLight(const vec3 lightDirection) {
#if defined(SUN_AS_AREA_LIGHT)
    if (frameUniforms.sun.w >= 0.0) {
        // simulate sun as disc area light
        float LoR = dot(lightDirection, shading_reflected);
        float d = frameUniforms.sun.x;
        highp vec3 s = shading_reflected - LoR * lightDirection;
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
    light.NoL = saturate(dot(shading_normal, light.l));
    return light;
}

void evaluateDirectionalLight(const MaterialInputs material,
        const PixelParams pixel, inout vec3 color) {

    Light light = getDirectionalLight();

    float visibility = 1.0;
#if defined(HAS_SHADOWING)
    if (light.NoL > 0.0) {
        float ssContactShadowOcclusion = 0.0;

        if ((frameUniforms.directionalShadows & 1u) != 0u) {
            visibility = shadow(light_shadowMap, 0u, getLightSpacePosition());
        }

        if (visibility > 0.0 && frameUniforms.ssContactShadowDistance != 0.0) {
            if (objectUniforms.screenSpaceContactShadows != 0) {
                ssContactShadowOcclusion = screenSpaceContactShadow(light.l);
            }
        }

        visibility = visibility * (1.0 - ssContactShadowOcclusion);

        #if defined(MATERIAL_HAS_AMBIENT_OCCLUSION)
        visibility *= computeMicroShadowing(light.NoL, material.ambientOcclusion);
        #endif
    } else {
#if defined(MATERIAL_CAN_SKIP_LIGHTING)
        return;
#endif
    }
#elif defined(MATERIAL_CAN_SKIP_LIGHTING)
    if (light.NoL <= 0.0) return;
#endif

    color.rgb += surfaceShading(pixel, light, visibility);
}
