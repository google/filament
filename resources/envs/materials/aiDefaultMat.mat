material {
    name : AssimpDefaultMat,
    shadingModel : lit,
    parameters : [
        {
            type : float3,
            name : baseColor
        },
        {
            type : float,
            name : metallic
        },
        {
            type : float,
            name : roughness
        },
        {
            type : float,
            name : reflectance
        }
    ],
}

fragment {
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        material.baseColor.rgb = materialParams.baseColor;
        material.metallic = materialParams.metallic;
        material.roughness = materialParams.roughness;
        material.reflectance = materialParams.reflectance;
    }
}
