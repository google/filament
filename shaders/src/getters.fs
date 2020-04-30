#if defined(HAS_ATTRIBUTE_COLOR)
/** @public-api */
vec4 getColor() {
    return vertex_color;
}
#endif

#if defined(HAS_ATTRIBUTE_UV0)
/** @public-api */
vec2 getUV0() {
    return vertex_uv01.xy;
}
#endif

#if defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
vec2 getUV1() {
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
 * Transforms a texture UV to make it suitable for a render target attachment.
 *
 * In Vulkan and Metal, texture coords are Y-down but in OpenGL they are Y-up. This wrapper function
 * accounts for these differences. When sampling from non-render targets (i.e. uploaded textures)
 * these differences do not matter because OpenGL has a second piece of backwardness, which is that
 * the first row of texels in glTexImage2D is interpreted as the bottom row.
 *
 * To protect users from these differences, we recommend that materials in the SURFACE domain
 * leverage this wrapper function when sampling from offscreen render targets.
 *
 * @public-api
 */
highp vec2 uvToRenderTargetUV(highp vec2 uv) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    return uv;
}

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
highp vec3 getLightSpacePosition() {
    return vertex_lightSpacePosition.xyz * (1.0 / vertex_lightSpacePosition.w);
}
#endif

/**
 * Returns the normalized [0, 1] viewport coordinates with the origin at the viewport's bottom-left.
 *
 * @public-api
 */
highp vec3 getNormalizedViewportCoord() {
    return vec3(shading_normalizedViewportCoord, gl_FragCoord.z);
}

#if defined(HAS_SHADOWING) && defined(HAS_DYNAMIC_LIGHTING)
highp vec3 getSpotLightSpacePosition(uint index) {
    highp vec4 position = vertex_spotLightSpacePosition[index];
    return position.xyz * (1.0 / position.w);
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
    uint cascadeCount = frameUniforms.cascades & 0xF;
    return clamp(uint(dot(vec4(greaterZ), vec4(1.0))), 0u, cascadeCount - 1u);
}

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)

highp vec3 getCascadeLightSpacePosition(uint cascade) {
    // For the first cascade, return the interpolated light space position.
    // This branch will be coherent (mostly) for neighboring fragments, and it's worth avoiding
    // the matrix multiply inside computeLightSpacePosition.
    if (cascade == 0) {
        return getLightSpacePosition();
    }

    highp vec4 pos = computeLightSpacePosition(getWorldPosition(), getWorldNormalVector(),
        frameUniforms.lightDirection, frameUniforms.shadowBias.y,
        frameUniforms.lightFromWorldMatrix[cascade]);
    return pos.xyz * (1.0 / pos.w);
}

#endif
