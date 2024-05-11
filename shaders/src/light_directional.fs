//------------------------------------------------------------------------------
// Directional light evaluation
//------------------------------------------------------------------------------

#if FILAMENT_QUALITY >= FILAMENT_QUALITY_HIGH
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
    light.channels = frameUniforms.lightChannels & 0xFF;
    return light;
}

void evaluateDirectionalLight(const MaterialInputs material,
        const PixelParams pixel, inout vec3 color) {

    Light light = getDirectionalLight();

    int channels = object_uniforms_flagsChannels & 0xFF;
    if ((light.channels & channels) == 0) {
        return;
    }

#if defined(MATERIAL_CAN_SKIP_LIGHTING)
    if (light.NoL <= 0.0) {
        return;
    }
#endif

    float visibility = 1.0;
#if defined(VARIANT_HAS_SHADOWING)
    if (light.NoL > 0.0) {
        float ssContactShadowOcclusion = 0.0;

        int cascade = getShadowCascade();
        bool cascadeHasVisibleShadows = bool(frameUniforms.cascades & ((1 << cascade) << 8));
        bool hasDirectionalShadows = bool(frameUniforms.directionalShadows & 1);
        if (hasDirectionalShadows && cascadeHasVisibleShadows) {
            highp vec4 shadowPosition = getShadowPosition(cascade);
            visibility = shadow(true, light_shadowMap, cascade, shadowPosition, 0.0);
            // shadow far attenuation
            highp vec3 v = getWorldPosition() - getWorldCameraPosition();
            // (viewFromWorld * v).z == dot(transpose(viewFromWorld), v)
            highp float z = dot(transpose(getViewFromWorldMatrix())[2].xyz, v);
            highp vec2 p = frameUniforms.shadowFarAttenuationParams;
            visibility = 1.0 - ((1.0 - visibility) * saturate(p.x - z * z * p.y));
        }
        if ((frameUniforms.directionalShadows & 0x2) != 0 && visibility > 0.0) {
            if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0) {
                ssContactShadowOcclusion = screenSpaceContactShadow(light.l);
            }
        }

        visibility *= 1.0 - ssContactShadowOcclusion;

        #if defined(MATERIAL_HAS_AMBIENT_OCCLUSION)
        visibility *= computeMicroShadowing(light.NoL, material.ambientOcclusion);
        #endif
#if defined(MATERIAL_CAN_SKIP_LIGHTING)
        if (visibility <= 0.0) {
            return;
        }
#endif
    }
#endif

#if defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
    color.rgb += customSurfaceShading(material, pixel, light, visibility);
#else
    color.rgb += surfaceShading(pixel, light, visibility);
#endif
}
