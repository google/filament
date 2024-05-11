/**
 * Evalutes lit materials with the subsurface shading model. This model is a
 * combination of a BRDF (the same used in shading_model_standard.fs, refer to that
 * file for more information) and of an approximated BTDF to simulate subsurface
 * scattering. The BTDF itself is not physically based and does not represent a
 * correct interpretation of transmission events.
 */
vec3 surfaceShading(const PixelParams pixel, const Light light, float occlusion) {
    vec3 h = normalize(shading_view + light.l);

    float NoL = light.NoL;
    float NoH = saturate(dot(shading_normal, h));
    float LoH = saturate(dot(light.l, h));

    vec3 Fr = vec3(0.0);
    if (NoL > 0.0) {
        // specular BRDF
        float D = distribution(pixel.roughness, NoH, h);
        float V = visibility(pixel.roughness, shading_NoV, NoL);
        vec3  F = fresnel(pixel.f0, LoH);
        Fr = (D * V) * F * pixel.energyCompensation;
    }

    // diffuse BRDF
    vec3 Fd = pixel.diffuseColor * diffuse(pixel.roughness, shading_NoV, NoL, LoH);

    // NoL does not apply to transmitted light
    vec3 color = (Fd + Fr) * (NoL * occlusion);

    // subsurface scattering
    // Use a spherical gaussian approximation of pow() for forwardScattering
    // We could include distortion by adding shading_normal * distortion to light.l
    float scatterVoH = saturate(dot(shading_view, -light.l));
    float forwardScatter = exp2(scatterVoH * pixel.subsurfacePower - pixel.subsurfacePower);
    float backScatter = saturate(NoL * pixel.thickness + (1.0 - pixel.thickness)) * 0.5;
    float subsurface = mix(backScatter, 1.0, forwardScatter) * (1.0 - pixel.thickness);
    color += pixel.subsurfaceColor * (subsurface * Fd_Lambert());

    // TODO: apply occlusion to the transmitted light
    return (color * light.colorIntensity.rgb) * (light.colorIntensity.w * light.attenuation);
}
