vec3 customSurfaceShading(const MaterialInputs materialInputs,
        const PixelParams pixel, const Light light, float visibility) {

    LightData lightData;
    lightData.colorIntensity = light.colorIntensity;
    lightData.l = light.l;
    lightData.NdotL = light.NoL;
    lightData.worldPosition = light.worldPosition;
    lightData.attenuation = light.attenuation;
    lightData.visibility = visibility;

    ShadingData shadingData;
    shadingData.diffuseColor = pixel.diffuseColor;
    shadingData.perceptualRoughness = pixel.perceptualRoughness;
    shadingData.f0 = pixel.f0;
    shadingData.roughness = pixel.roughness;

    return surfaceShading(materialInputs, shadingData, lightData);
}
