//------------------------------------------------------------------------------
// Uniforms access
//------------------------------------------------------------------------------

/** sort-of public */
float getObjectUserData() {
    return object_uniforms_userData;
}

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
highp vec3 getUserWorldPosition() {
    return mulMat4x4Float3(getUserWorldFromWorldMatrix(), getWorldPosition()).xyz;
}

/** @public-api */
vec3 getWorldViewVector() {
    return shading_view;
}

bool isPerspectiveProjection() {
    return frameUniforms.clipFromViewMatrix[2].w != 0.0;
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

highp vec3 getNormalizedPhysicalViewportCoord() {
    // make sure to handle our reversed-z
    return vec3(shading_normalizedViewportCoord, gl_FragCoord.z);
}

/**
 * Returns the normalized [0, 1] logical viewport coordinates with the origin at the viewport's
 * bottom-left. Z coordinate is in the [1, 0] range (reversed).
 *
 * @public-api
 */
highp vec3 getNormalizedViewportCoord() {
    // make sure to handle our reversed-z
    highp vec2 scale = frameUniforms.logicalViewportScale;
    highp vec2 offset = frameUniforms.logicalViewportOffset;
    highp vec2 logicalUv = shading_normalizedViewportCoord * scale + offset;
    return vec3(logicalUv, gl_FragCoord.z);
}

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DYNAMIC_LIGHTING)
highp vec4 getSpotLightSpacePosition(int index, highp vec3 dir, highp float zLight) {
    highp mat4 lightFromWorldMatrix = shadowUniforms.shadows[index].lightFromWorldMatrix;

    // for spotlights, the bias depends on z
    float bias = shadowUniforms.shadows[index].normalBias * zLight;

    return computeLightSpacePosition(getWorldPosition(), getWorldGeometricNormalVector(),
            dir, bias, lightFromWorldMatrix);
}
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
bool isDoubleSided() {
    return materialParams._doubleSided;
}
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)

/**
 * Returns the cascade index for this fragment (between 0 and CONFIG_MAX_SHADOW_CASCADES - 1).
 */
int getShadowCascade() {
    highp float z = mulMat4x4Float3(getViewFromWorldMatrix(), getWorldPosition()).z;
    ivec4 greaterZ = ivec4(greaterThan(frameUniforms.cascadeSplits, vec4(z)));
    int cascadeCount = frameUniforms.cascades & 0xF;
    return clamp(greaterZ.x + greaterZ.y + greaterZ.z + greaterZ.w, 0, cascadeCount - 1);
}

highp vec4 getCascadeLightSpacePosition(int cascade) {
    // For the first cascade, return the interpolated light space position.
    // This branch will be coherent (mostly) for neighboring fragments, and it's worth avoiding
    // the matrix multiply inside computeLightSpacePosition.
    if (cascade == 0) {
        // Note: this branch may cause issues with derivatives
        return vertex_lightSpacePosition;
    }

    return computeLightSpacePosition(getWorldPosition(), getWorldGeometricNormalVector(),
        frameUniforms.lightDirection,
        shadowUniforms.shadows[cascade].normalBias,
        shadowUniforms.shadows[cascade].lightFromWorldMatrix);
}

#endif

