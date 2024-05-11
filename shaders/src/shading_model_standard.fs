#if defined(MATERIAL_HAS_SHEEN_COLOR)
vec3 sheenLobe(const PixelParams pixel, float NoV, float NoL, float NoH) {
    float D = distributionCloth(pixel.sheenRoughness, NoH);
    float V = visibilityCloth(NoV, NoL);

    return (D * V) * pixel.sheenColor;
}
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
float clearCoatLobe(const PixelParams pixel, const vec3 h, float NoH, float LoH, out float Fcc) {
#if defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    // If the material has a normal map, we want to use the geometric normal
    // instead to avoid applying the normal map details to the clear coat layer
    float clearCoatNoH = saturate(dot(shading_clearCoatNormal, h));
#else
    float clearCoatNoH = NoH;
#endif

    // clear coat specular lobe
    float D = distributionClearCoat(pixel.clearCoatRoughness, clearCoatNoH, h);
    float V = visibilityClearCoat(LoH);
    float F = F_Schlick(0.04, 1.0, LoH) * pixel.clearCoat; // fix IOR to 1.5

    Fcc = F;
    return D * V * F;
}
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
vec3 anisotropicLobe(const PixelParams pixel, const Light light, const vec3 h,
        float NoV, float NoL, float NoH, float LoH) {

    vec3 l = light.l;
    vec3 t = pixel.anisotropicT;
    vec3 b = pixel.anisotropicB;
    vec3 v = shading_view;

    float ToV = dot(t, v);
    float BoV = dot(b, v);
    float ToL = dot(t, l);
    float BoL = dot(b, l);
    float ToH = dot(t, h);
    float BoH = dot(b, h);

    // Anisotropic parameters: at and ab are the roughness along the tangent and bitangent
    // to simplify materials, we derive them from a single roughness parameter
    // Kulla 2017, "Revisiting Physically Based Shading at Imageworks"
    float at = max(pixel.roughness * (1.0 + pixel.anisotropy), MIN_ROUGHNESS);
    float ab = max(pixel.roughness * (1.0 - pixel.anisotropy), MIN_ROUGHNESS);

    // specular anisotropic BRDF
    float D = distributionAnisotropic(at, ab, ToH, BoH, NoH);
    float V = visibilityAnisotropic(pixel.roughness, at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
    vec3  F = fresnel(pixel.f0, LoH);

    return (D * V) * F;
}
#endif

vec3 isotropicLobe(const PixelParams pixel, const Light light, const vec3 h,
        float NoV, float NoL, float NoH, float LoH) {

    float D = distribution(pixel.roughness, NoH, h);
    float V = visibility(pixel.roughness, NoV, NoL);
    vec3  F = fresnel(pixel.f0, LoH);

    return (D * V) * F;
}

vec3 specularLobe(const PixelParams pixel, const Light light, const vec3 h,
        float NoV, float NoL, float NoH, float LoH) {
#if defined(MATERIAL_HAS_ANISOTROPY)
    return anisotropicLobe(pixel, light, h, NoV, NoL, NoH, LoH);
#else
    return isotropicLobe(pixel, light, h, NoV, NoL, NoH, LoH);
#endif
}

vec3 diffuseLobe(const PixelParams pixel, float NoV, float NoL, float LoH) {
    return pixel.diffuseColor * diffuse(pixel.roughness, NoV, NoL, LoH);
}

/**
 * Evaluates lit materials with the standard shading model. This model comprises
 * of 2 BRDFs: an optional clear coat BRDF, and a regular surface BRDF.
 *
 * Surface BRDF
 * The surface BRDF uses a diffuse lobe and a specular lobe to render both
 * dielectrics and conductors. The specular lobe is based on the Cook-Torrance
 * micro-facet model (see brdf.fs for more details). In addition, the specular
 * can be either isotropic or anisotropic.
 *
 * Clear coat BRDF
 * The clear coat BRDF simulates a transparent, absorbing dielectric layer on
 * top of the surface. Its IOR is set to 1.5 (polyutherane) to simplify
 * our computations. This BRDF only contains a specular lobe and while based
 * on the Cook-Torrance microfacet model, it uses cheaper terms than the surface
 * BRDF's specular lobe (see brdf.fs).
 */
vec3 surfaceShading(const PixelParams pixel, const Light light, float occlusion) {
    vec3 h = normalize(shading_view + light.l);

    float NoV = shading_NoV;
    float NoL = saturate(light.NoL);
    float NoH = saturate(dot(shading_normal, h));
    float LoH = saturate(dot(light.l, h));

    vec3 Fr = specularLobe(pixel, light, h, NoV, NoL, NoH, LoH);
    vec3 Fd = diffuseLobe(pixel, NoV, NoL, LoH);
#if defined(MATERIAL_HAS_REFRACTION)
    Fd *= (1.0 - pixel.transmission);
#endif

    // TODO: attenuate the diffuse lobe to avoid energy gain

    // The energy compensation term is used to counteract the darkening effect
    // at high roughness
    vec3 color = Fd + Fr * pixel.energyCompensation;

#if defined(MATERIAL_HAS_SHEEN_COLOR)
    color *= pixel.sheenScaling;
    color += sheenLobe(pixel, NoV, NoL, NoH);
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float Fcc;
    float clearCoat = clearCoatLobe(pixel, h, NoH, LoH, Fcc);
    float attenuation = 1.0 - Fcc;

#if defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    color *= attenuation * NoL;

    // If the material has a normal map, we want to use the geometric normal
    // instead to avoid applying the normal map details to the clear coat layer
    float clearCoatNoL = saturate(dot(shading_clearCoatNormal, light.l));
    color += clearCoat * clearCoatNoL;

    // Early exit to avoid the extra multiplication by NoL
    return (color * light.colorIntensity.rgb) *
            (light.colorIntensity.w * light.attenuation * occlusion);
#else
    color *= attenuation;
    color += clearCoat;
#endif
#endif

    return (color * light.colorIntensity.rgb) *
            (light.colorIntensity.w * light.attenuation * NoL * occlusion);
}
