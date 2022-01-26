//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions for Visualization material implementations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(BLEND_MODE_OPAQUE) || defined(BLEND_MODE_MASKED))
#define BLENDING_DISABLED
#else
#define BLENDING_ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Input adjustment functions
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FragmentData {
    highp vec3 pos; // world space
    highp vec3 normal; // tangent space
};
const FragmentData FRAGMENT_DATA_INITIAL = FragmentData(vec3(0.0), vec3(0.0));

void ApplyEditorScalers(inout MaterialInputs material) {
#if !defined(IN_SHAPR_SHADER)
    // Temporarily disabling these - we will only need these again when we start authoring mesh environments
    //material.specularScale = 1.0 + materialParams.scalingControl.x;
    //material.diffuseScale = 1.0 + materialParams.scalingControl.z;
#endif
}

FragmentData GetPositionAndNormal() {
    FragmentData result = FRAGMENT_DATA_INITIAL;
#if defined(SHAPR_OBJECT_SPACE_TRILINEAR)
    result.pos = variable_objectSpacePosition.xyz;
#else
    result.pos = getWorldPosition() + getWorldOffset();
#endif
#if 1
#if defined(SHAPR_OBJECT_SPACE_TRILINEAR)
    result.normal = variable_objectSpaceNormal.xyz;
#else
    result.normal = getWorldGeometricNormalVector();
#endif
#else
    // Since I have to re-add this so many times to debug issues, I'm leaving this commented out for the time being
    // Still, be careful: OGL did not expose the fine gradients until 4.5 (!!!) core, so you cannot know if these are
    // coarse (=constant for the 2x2 fragment quad) or fine (forward/backward difference'd) derivatives
    vec3 DxPos = dFdx(result.pos);
    vec3 DyPos = dFdy(result.pos);
    result.normal = normalize(cross(DxPos, DyPos));
#endif
    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blending related functions that implement triplanar texture and normal blending
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A simple linear blend with normalization
vec3 ComputeWeights(vec3 normal) {
    // This one has a region where there is no blend, creating more defined interpolations
    const float blendBias = 0.2;
    vec3 blend = abs(normal.xyz);
    blend = max(blend - blendBias, vec3(0.0));
    blend = blend * blend;
    blend /= (blend.x + blend.y + blend.z);
    return blend;
}

vec4 TriplanarTexture(sampler2D tex, float scaler, highp vec3 pos, lowp vec3 normal) {
    // Depending on the resolution of the texture, we may want to multiply the texture coordinates
    vec3 queryPos = scaler * pos;
    vec3 weights = ComputeWeights(normal);
    return weights.x * texture(tex, queryPos.yz) + weights.y * texture(tex, queryPos.xz) +
           weights.z * texture(tex, queryPos.xy);
}

vec3 UnpackNormal(vec2 packedNormal) {
    float x = packedNormal.x * 2.0 - 1.0;
    float y = packedNormal.y * 2.0 - 1.0;
    return vec3(x, y, sqrt(clamp(1.0 - x * x - y * y, 0.0, 1.0)));
}
vec3 UnpackNormal(vec3 packedNormal) {
    return packedNormal * 2.0 - 1.0;
}
// This is a whiteout blended tripalanar normal mapping, where each plane's tangent frame is
// approximated by the appropriate sequence and flips of world space axes. For more details
// Refer to https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
vec3 TriplanarNormalMap(sampler2D normalMap, float scaler, highp vec3 pos, lowp vec3 normal) {
    // Whiteout blend
    // Triplanar uvs
    // We intentionally diverged from the original article and use 'pos.yz' below to align with triplanarTexture()
    vec2 uvX = scaler * pos.yz; // x facing plane
    vec2 uvY = scaler * pos.xz; // y facing plane
    vec2 uvZ = scaler * pos.xy; // z facing plane

    // Tangent space normal maps
    // 2-channel XY TS normal texture: this saves 33% on storage
    lowp vec3 tnormalX = UnpackNormal(texture(normalMap, uvX).xy);
    lowp vec3 tnormalY = UnpackNormal(texture(normalMap, uvY).xy);
    lowp vec3 tnormalZ = UnpackNormal(texture(normalMap, uvZ).xy);

    // Swizzle world normals into tangent space and apply Whiteout blend
    tnormalX = vec3(tnormalX.xy + normal.zy, abs(tnormalX.z) * normal.x);
    tnormalY = vec3(tnormalY.xy + normal.xz, abs(tnormalY.z) * normal.y);
    tnormalZ = vec3(tnormalZ.xy + normal.xy, abs(tnormalZ.z) * normal.z);
    // Compute blend weights
    vec3 blend = ComputeWeights(normal);
    // Swizzle tangent normals to match world orientation and triblend
    return normalize(tnormalX.zyx * blend.x + tnormalY.xzy * blend.y + tnormalZ.xyz * blend.z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Custom code to evaluate the materials
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ApplyNormalMap(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_NORMAL)
    if (materialParams.useNormalTexture == 1) {
        // We combine the normals in world space, hence the transformation in the end from world to tangent, assuming an
        // orthonormal tangent frame (which may not hold actually but looks fine enough for now).
        vec3 normalWS = TriplanarNormalMap(materialParams_normalTexture,
                                           materialParams.textureScaler.y,
                                           fragmentData.pos,
                                           fragmentData.normal);
        material.normal = normalWS * getWorldTangentFrame();
    }
    // By this time the normal should be ready, it is safe to apply the normal scale
    material.normal.xy *= materialParams.normalIntensity;
#endif
}

void ApplyClearCoatNormalMap(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
    if (materialParams.useClearCoatNormalTexture == 1) {
        vec3 clearCoatNormalWS = TriplanarNormalMap(materialParams_clearCoatNormalTexture,
                                                    materialParams.textureScaler.z,
                                                    fragmentData.pos,
                                                    fragmentData.normal);
        material.clearCoatNormal = clearCoatNormalWS * getWorldTangentFrame();
        material.clearCoatNormal.xy *= materialParams.clearCoatNormalIntensity;
    }
#endif
}

void ApplyBaseColor(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_BASE_COLOR) 
    if (materialParams.useBaseColorTexture == 1) {
#if defined(BLENDING_ENABLED) || defined(HAS_REFRACTION)
        material.baseColor.rgba = TriplanarTexture(materialParams_baseColorTexture,
                                                   materialParams.textureScaler.x,
                                                   fragmentData.pos,
                                                   fragmentData.normal)
                                      .rgba;
#else
        material.baseColor.rgb = TriplanarTexture(materialParams_baseColorTexture,
                                                  materialParams.textureScaler.x,
                                                  fragmentData.pos,
                                                  fragmentData.normal)
                                     .rgb;
#endif
    } else {
#if defined(BLENDING_ENABLED) || defined(HAS_REFRACTION)
        material.baseColor.rgba = materialParams.baseColor.rgba;
#else
        material.baseColor.rgb = materialParams.baseColor.rgb;
#endif
    }

    // Naive multiplicative tinting seems to be fine enough for now
    material.baseColor.rgb *= materialParams.tintColor.rgb;
#if defined(BLENDING_ENABLED)
    material.baseColor.rgb *= material.baseColor.a;
#endif

#if defined(BLENDING_ENABLED)
    material.baseColor.a = 0.0;
#else
    material.baseColor.a = 1.0;
#endif
#endif
}

void ApplyOcclusion(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_AMBIENT_OCCLUSION) && defined(BLENDING_DISABLED)
    if (materialParams.useOcclusionTexture == 1) {
        material.ambientOcclusion = TriplanarTexture(materialParams_occlusionTexture,
                                                     materialParams.textureScaler.y,
                                                     fragmentData.pos,
                                                     fragmentData.normal)
                                        .r;
    } else {
        material.ambientOcclusion = materialParams.occlusion;
    }
    material.ambientOcclusion = 1.0 - materialParams.occlusionIntensity * (1.0 - material.ambientOcclusion);
#endif
}

void ApplyRoughness(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_ROUGHNESS)
    if (materialParams.useRoughnessTexture == 1) {
        material.roughness = TriplanarTexture(materialParams_roughnessTexture,
                                              materialParams.textureScaler.y,
                                              fragmentData.pos,
                                              fragmentData.normal)
                                 .r;
    } else {
        material.roughness = materialParams.roughness;
    }
    material.roughness *= materialParams.roughnessScale;
#endif
}

void ApplyReflectance(inout MaterialInputs material, inout FragmentData fragmentData) {
    // Unfortunately, MATERIAL_HAS_REFLECTANCE is defined even for cloth which does not have reflectance. Luckily, cloth and
    // specular-glossiness models are the only two that do not have this property (and the latter is a legacy mode anyway).
#if defined(MATERIAL_HAS_REFLECTANCE) && !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    // Careful: reflectance affects non-metals, so this alone may still make things overly shiny - we added
    // specularIntensity to handle specular response on all lighting paths.
#if defined(HAS_REFRACTION)
    material.reflectance = materialParams.reflectance;
#else
    material.reflectance = clamp(1.0 - material.roughness, 0.0, 1.0);
#endif
#endif
}

void ApplyMetallic(inout MaterialInputs material, inout FragmentData fragmentData) {
    // Same as applyReflectance: cloth and specular glossiness explicitly do not have these properties.
#if defined(MATERIAL_HAS_METALLIC) && !defined(SHADING_MODEL_CLOTH) && !defined(SHADING_MODEL_SPECULAR_GLOSSINESS)
    if (materialParams.useMetallicTexture == 1) {
        material.metallic = TriplanarTexture(materialParams_metallicTexture,
                                             materialParams.textureScaler.y,
                                             fragmentData.pos,
                                             fragmentData.normal)
                                .r;
    } else {
        material.metallic = materialParams.metallic;
    }
#endif
}

void ApplyClearCoatRoughness(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_CLEAR_COAT_ROUGHNESS)
    if (materialParams.useClearCoatRoughnessTexture == 1) {
        material.clearCoatRoughness = TriplanarTexture(materialParams_clearCoatRoughnessTexture,
                                                       materialParams.textureScaler.z,
                                                       fragmentData.pos,
                                                       fragmentData.normal)
                                          .r;
    } else {
        material.clearCoatRoughness = materialParams.clearCoatRoughness;
    }
#endif
}

void ApplyAbsorption(inout MaterialInputs material, inout FragmentData fragmentData) {
    // This is a transmission-only property and those materials actually disable blending
#if defined(MATERIAL_HAS_ABSORPTION) && defined(HAS_REFRACTION)
    material.absorption = ( materialParams.doDeriveAbsorption == 1 ) ? 1.0 - material.baseColor.rgb : materialParams.absorption;
#endif
}

void ApplyIOR(inout MaterialInputs material, inout FragmentData fragmentData) {
    // This is a transmission-only property and those materials actually disable blending
#if defined(MATERIAL_HAS_IOR) && defined(HAS_REFRACTION)
    material.ior = 1.0 + materialParams.iorScale * ( materialParams.ior - 1.0 );
#endif
}

void ApplyTransmission(inout MaterialInputs material, inout FragmentData fragmentData) {
    // This is a transmission-only property and those materials actually disable blending
#if defined(MATERIAL_HAS_TRANSMISSION) && defined(HAS_REFRACTION)
    if ( materialParams.useTransmissionTexture == 1 ) {
        material.transmission = TriplanarTexture(materialParams_transmissionTexture, materialParams.textureScaler.w, fragmentData.pos, fragmentData.normal).r;
    } else {
        material.transmission = materialParams.transmission;
    }
#endif
}

void ApplyThickness(inout MaterialInputs material, inout FragmentData fragmentData) {
    // This is a transmission-only property and those materials actually disable blending
    // This applies both micro and regular thickness, although we only do the latter for now (the former would be used in transparent thin materials).
#if defined(BLENDING_ENABLED) && defined(HAS_REFRACTION)
    float thicknessValue = 0.0;
    if (materialParams.useThicknessTexture == 1) {
        thicknessValue = TriplanarTexture(materialParams_thicknessTexture,
                                          materialParams.textureScaler.w,
                                          fragmentData.pos,
                                          fragmentData.normal)
                             .r;
    } else {
        thicknessValue = materialParams.thickness;
    }
    thicknessValue *= materialParams.maxThickness;

#if defined(MATERIAL_HAS_MICRO_THICKNESS) && defined(REFRACTION_TYPE) && REFRACTION_TYPE == REFRACTION_TYPE_THIN
    material.microThickness = thicknessValue; // default 0.0
#elif defined(HAS_REFRACTION)
    material.thickness = thicknessValue; // default 0.5
#endif
#endif
}

void ApplySheenRoughness(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_SHEEN_ROUGHNESS) && !defined(SHADING_MODEL_SUBSURFACE) && !defined(SHADING_MODEL_CLOTH) &&    \
    defined(BLENDING_DISABLED)
#if defined(MATERIAL_HAS_USE_SHEEN_ROUGHNESS_TEXTURE)
    if (materialParams.useSheenRoughnessTexture == 1) {
        material.sheenRoughness =
            TriplanarTexture(materialParams_sheenRoughnessTexture, 1.0f, fragmentData.pos, fragmentData.normal).r;
    } else {
        material.sheenRoughness = materialParams.sheenRoughness;
    }
#else
    material.sheenRoughness = materialParams.sheenRoughness;
#endif
#endif
}

void ApplyShaprScalars(inout MaterialInputs material, inout FragmentData fragmentData) {
    // All of our materials have specularIntensity and useWard, so no need to define-guard these
    material.specularIntensity = materialParams.specularIntensity;
    material.useWard = (materialParams.useWard == 1) ? true : false;
}

void ApplyNonTextured(inout MaterialInputs material, inout FragmentData fragmentData) {
#if defined(MATERIAL_HAS_CLEAR_COAT)
    material.clearCoat = materialParams.clearCoat;
#endif
#if defined(MATERIAL_HAS_ANISOTROPY) && defined(BLENDING_DISABLED)
    material.anisotropy = materialParams.anisotropy;
#endif
#if defined(MATERIAL_HAS_ANISOTROPY_DIRECTION) && defined(BLENDING_DISABLED)
    material.anisotropyDirection = materialParams.anisotropyDirection;
#endif
#if defined(MATERIAL_HAS_SHEEN_COLOR) && !defined(SHADING_MODEL_SUBSURFACE) && defined(BLENDING_DISABLED)
    // Subsurface is not using sheen color but the others are
    material.sheenColor =
        (materialParams.doDeriveSheenColor == 1) ? sqrt(material.baseColor.rgb) : materialParams.sheenColor;
#endif
#if defined(MATERIAL_HAS_SUBSURFACE_COLOR) && defined(SHADING_MODEL_SUBSURFACE)
    material.subsurfaceColor = materialParams.subsurfaceColor;
#endif
#if defined(MATERIAL_HAS_SUBSURFACE_POWER) && defined(SHADING_MODEL_SUBSURFACE)
    material.subsurfacePower = materialParams.subsurfacePower;
#endif
#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    material.postLightingColor.rgb = float3(materialParams.ambient);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Convenience functions to simplify the various material files. These basically batch together all calls to the
// functions above, relying on the proper define-s to avoid trying to manipulate attributes that are not present.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ApplyAllPrePrepare(inout MaterialInputs material, inout FragmentData fragmentData) {
    // Filament material docs state normals should be set _before_ calling prepareMaterial,
    // and that also means clear coat normals
    ApplyNormalMap(material, fragmentData);
    ApplyClearCoatNormalMap(material, fragmentData);
}

void ApplyAllPostPrepare(inout MaterialInputs material, inout FragmentData fragmentData) {
    ApplyOcclusion(material, fragmentData);
    ApplyBaseColor(material, fragmentData);
    ApplyRoughness(material, fragmentData);
    ApplyReflectance(material, fragmentData);
    ApplyMetallic(material, fragmentData);

    ApplyClearCoatRoughness(material, fragmentData);
    ApplySheenRoughness(material, fragmentData);

    ApplyAbsorption(material, fragmentData);
    ApplyIOR(material, fragmentData);
    ApplyTransmission(material, fragmentData);
    ApplyThickness(material, fragmentData);

    ApplyNonTextured(material, fragmentData);
    ApplyShaprScalars(material, fragmentData);
}