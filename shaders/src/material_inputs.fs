// Decide if we can skip lighting when dot(n, l) <= 0.0
#if defined(SHADING_MODEL_CLOTH)
#if !defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    #define MATERIAL_CAN_SKIP_LIGHTING
#endif
#elif defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
    // Cannot skip lighting
#else
    #define MATERIAL_CAN_SKIP_LIGHTING
#endif

struct MaterialInputs {
    vec4  baseColor;
#if !defined(SHADING_MODEL_UNLIT)
#if !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float roughness;
#endif
#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    float metallic;
    float reflectance;
#endif
    float ambientOcclusion;
#endif
    vec4  emissive;

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
    vec3 sheenColor;
    float sheenRoughness;
#endif

    float clearCoat;
    float clearCoatRoughness;

    float anisotropy;
    vec3  anisotropyDirection;

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    float thickness;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    float subsurfacePower;
    vec3  subsurfaceColor;
#endif

#if defined(SHADING_MODEL_CLOTH)
    vec3  sheenColor;
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3  subsurfaceColor;
#endif
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    vec3  specularColor;
    float glossiness;
#endif

#if defined(MATERIAL_HAS_NORMAL)
    vec3  normal;
#endif
#if defined(MATERIAL_HAS_BENT_NORMAL)
    vec3  bentNormal;
#endif
#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    vec3  clearCoatNormal;
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    vec4  postLightingColor;
    float postLightingMixFactor;
#endif

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_REFRACTION)
#if defined(MATERIAL_HAS_ABSORPTION)
    vec3 absorption;
#endif
#if defined(MATERIAL_HAS_TRANSMISSION)
    float transmission;
#endif
#if defined(MATERIAL_HAS_IOR)
    float ior;
#endif
#if defined(MATERIAL_HAS_MICRO_THICKNESS) && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    float microThickness;
#endif
#elif !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
#if defined(MATERIAL_HAS_IOR)
    float ior;
#endif
#endif
#endif
};

void initMaterial(out MaterialInputs material) {
    material.baseColor = vec4(1.0);
#if !defined(SHADING_MODEL_UNLIT)
#if !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.roughness = 1.0;
#endif
#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.metallic = 0.0;
    material.reflectance = 0.5;
#endif
    material.ambientOcclusion = 1.0;
#endif
    material.emissive = vec4(vec3(0.0), 1.0);

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_SHEEN_COLOR)
    material.sheenColor = vec3(0.0);
    material.sheenRoughness = 0.0;
#endif
#endif

#if defined(MATERIAL_HAS_CLEAR_COAT)
    material.clearCoat = 1.0;
    material.clearCoatRoughness = 0.0;
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    material.anisotropy = 0.0;
    material.anisotropyDirection = vec3(1.0, 0.0, 0.0);
#endif

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    material.thickness = 0.5;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    material.subsurfacePower = 12.234;
    material.subsurfaceColor = vec3(1.0);
#endif

#if defined(SHADING_MODEL_CLOTH)
    material.sheenColor = sqrt(material.baseColor.rgb);
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    material.subsurfaceColor = vec3(0.0);
#endif
#endif

#if defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    material.glossiness = 0.0;
    material.specularColor = vec3(0.0);
#endif

#if defined(MATERIAL_HAS_NORMAL)
    material.normal = vec3(0.0, 0.0, 1.0);
#endif
#if defined(MATERIAL_HAS_BENT_NORMAL)
    material.bentNormal = vec3(0.0, 0.0, 1.0);
#endif
#if defined(MATERIAL_HAS_CLEAR_COAT) && defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    material.clearCoatNormal = vec3(0.0, 0.0, 1.0);
#endif

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    material.postLightingColor = vec4(0.0);
    material.postLightingMixFactor = 1.0;
#endif

#if !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_UNLIT)
#if defined(MATERIAL_HAS_REFRACTION)
#if defined(MATERIAL_HAS_ABSORPTION)
    material.absorption = vec3(0.0);
#endif
#if defined(MATERIAL_HAS_TRANSMISSION)
    material.transmission = 1.0;
#endif
#if defined(MATERIAL_HAS_IOR)
    material.ior = 1.5;
#endif
#if defined(MATERIAL_HAS_MICRO_THICKNESS) && (REFRACTION_TYPE == REFRACTION_TYPE_THIN)
    material.microThickness = 0.0;
#endif
#elif !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
#if defined(MATERIAL_HAS_IOR)
    material.ior = 1.5;
#endif
#endif
#endif
}

#if defined(MATERIAL_HAS_CUSTOM_SURFACE_SHADING)
/** @public-api */
struct LightData {
    vec4  colorIntensity;
    vec3  l;
    float NdotL;
    vec3  worldPosition;
    float attenuation;
    float visibility;
};

/** @public-api */
struct ShadingData {
    vec3  diffuseColor;
    float perceptualRoughness;
    vec3  f0;
    float roughness;
};
#endif
