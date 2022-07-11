//------------------------------------------------------------------------------
// Instance access
//------------------------------------------------------------------------------

#if defined(MATERIAL_HAS_INSTANCES)
/** @public-api */
int getInstanceIndex() {
    return instance_index;
}
#endif

//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if defined(HAS_ATTRIBUTE_COLOR)
/** @public-api */
vec4 getColor() {
    return vertex_color;
}
#endif

#if defined(HAS_ATTRIBUTE_UV0)
/** @public-api */
highp vec2 getUV0() {
    return vertex_uv01.xy;
}
#endif

#if defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
highp vec2 getUV1() {
    return vertex_uv01.zw;
}
#endif

#if defined(BLEND_MODE_MASKED)
/** @public-api */
float getMaskThreshold() {
    return materialParams._maskThreshold;
}
#endif

/** @public-api */
highp mat3 getWorldTangentFrame() {
    return shading_tangentToWorld;
}

/** @public-api */
highp vec3 getWorldPosition() {
    return shading_position;
}

/** @public-api */
vec3 getWorldViewVector() {
    return shading_view;
}

/** @public-api */
vec3 getWorldNormalVector() {
    return shading_normal;
}

/** @public-api */
vec3 getWorldGeometricNormalVector() {
    return shading_geometricNormal;
}

/** @public-api */
vec3 getWorldReflectedVector() {
    return shading_reflected;
}

/** @public-api */
float getNdotV() {
    return shading_NoV;
}

/**
 * Returns the normalized [0, 1] viewport coordinates with the origin at the viewport's bottom-left.
 * Z coordinate is in the [0, 1] range as well.
 *
 * @public-api
 */
highp vec3 getNormalizedViewportCoord() {
    // make sure to handle our reversed-z
    return vec3(shading_normalizedViewportCoord, 1.0 - gl_FragCoord.z);
}

// This new version doesn't invert Z.
// TODO: Should we make it public?
highp vec3 getNormalizedViewportCoord2() {
    return vec3(shading_normalizedViewportCoord, gl_FragCoord.z);
}

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DYNAMIC_LIGHTING)
highp vec4 getSpotLightSpacePosition(uint index, highp float zLight) {
    highp mat4 lightFromWorldMatrix = shadowUniforms.shadows[index].lightFromWorldMatrix;
    highp vec3 dir = shadowUniforms.shadows[index].direction;

    // for spotlights, the bias depends on z
    float bias = shadowUniforms.shadows[index].normalBias * zLight;

    return computeLightSpacePosition(getWorldPosition(), getWorldNormalVector(),
            dir, bias, lightFromWorldMatrix);
}
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
bool isDoubleSided() {
    return materialParams._doubleSided;
}
#endif

/**
 * Returns the cascade index for this fragment (between 0 and CONFIG_MAX_SHADOW_CASCADES - 1).
 */
uint getShadowCascade() {
    vec3 viewPos = mulMat4x4Float3(getViewFromWorldMatrix(), getWorldPosition()).xyz;
    bvec4 greaterZ = greaterThan(frameUniforms.cascadeSplits, vec4(viewPos.z));
    uint cascadeCount = frameUniforms.cascades & 0xFu;
    return clamp(uint(dot(vec4(greaterZ), vec4(1.0))), 0u, cascadeCount - 1u);
}

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)

highp vec4 getCascadeLightSpacePosition(uint cascade) {
    // For the first cascade, return the interpolated light space position.
    // This branch will be coherent (mostly) for neighboring fragments, and it's worth avoiding
    // the matrix multiply inside computeLightSpacePosition.
    if (cascade == 0u) {
        // Note: this branch may cause issues with derivatives
        return vertex_lightSpacePosition;
    }

    return computeLightSpacePosition(getWorldPosition(), getWorldNormalVector(),
        frameUniforms.lightDirection, frameUniforms.shadowBias,
        frameUniforms.lightFromWorldMatrix[cascade]);
}

#endif

PerRenderableData getObjectUniforms() {
#if defined(MATERIAL_HAS_INSTANCES)
    // the material manages instancing, all instances share the same uniform block.
    return objectUniforms.data[0];
#else
     // automatic instancing was used, each instance has its own uniform block.
    return objectUniforms.data[instance_index];
#endif
}
