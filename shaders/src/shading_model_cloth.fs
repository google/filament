/**
 * Evaluates lit materials with the cloth shading model. Similar to the standard
 * model, the cloth shading model is based on a Cook-Torrance microfacet model.
 * Its distribution and visibility terms are however very different to take into
 * account the softer apperance of many types of cloth. Some highly reflecting
 * fabrics like satin or leather should use the standard model instead.
 *
 * This shading model optionally models subsurface scattering events. The
 * computation of these events is not physically based but can add necessary
 * details to a material.
 */
vec3 surfaceShading(const PixelParams pixel, const Light light, float occlusion) {
    float NoL = saturate(dot(shading_normal, light.l));

#if !defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    if (NoL <= 0.0) return vec3(0.0);
#endif

    vec3 h = normalize(shading_view + light.l);
    float NoH = saturate(dot(shading_normal, h));
    float LoH = saturate(dot(light.l, h));

    // specular BRDF
    float D = gl_FragCoord.x * frameUniforms.resolution.z < 0.5 ?
        D_Charlie(pixel.linearRoughness, NoH) : D_Ashikhmin(pixel.linearRoughness, NoH);
    float V = visibilityCloth(shading_NoV, NoL);
    vec3  F = fresnel(pixel.f0, LoH);
    // Ignore pixel.energyCompensation since we use a different BRDF here
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    float diffuse = diffuse(pixel.linearRoughness, shading_NoV, NoL, LoH);
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    // Energy conservative wrap diffuse to simulate subsurface scattering
    diffuse *= Fd_Wrap(dot(shading_normal, light.l), 0.5);
#endif

    // We multiply the diffuse term by the Fresnel term as discussed in
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    vec3 Fd = (diffuse * (1.0 - F_Schlick(0.05, 1.0, LoH))) * pixel.diffuseColor;

#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    // Cheap subsurface scatter
    Fd *= saturate(pixel.subsurfaceColor + NoL);
    // We need to apply NoL separately to the specular lobe since we already took
    // it into account in the diffuse lobe
    vec3 color = Fd + Fr * NoL;
    color *= light.colorIntensity.rgb * (light.colorIntensity.w * light.attenuation * occlusion);
#else
    vec3 color = Fd + Fr;
    color *= light.colorIntensity.rgb * (light.colorIntensity.w * light.attenuation * NoL * occlusion);
#endif

    return color;
}
